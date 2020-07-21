#pragma once
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <thread>

#include "xcl2.hpp"
#include "types.hpp"
#include "idct.hpp"

/* ***************************************************************************

runFPGA:

Wrapper function used to wrap host side setup and initialization and callfor 
FPGA accelerator

    done				: Signal the completion of runFPGA when used inside thread
    totalNumOfBatche    : No. of multi-block Batches processed per call
    batchSize           : No. of blocks per batch or processed per kernel call
    maxScheduledBatches : Total, no. of overlapping kernel(enqueues) calls allowed
    batchSizeBytes      : The data contained in one batch in bytes
    qDataSizeBytes      : The size of IDCT coefficients in bytes
    context             : Device context with initialized device
    cmdQ                : Command Queues 
    krnl_idct           : IDCT kernel created from device xclbin
    inputData           : Input data vector, contains multiple contiguous batches
    coeffs              : IDCT coefficients vector
    fpgaOutput          : FGPA kernel output data vector, contigeous multiple batches

*************************************************************************** */

void runFPGA(	std::atomic<bool> &done, // done flag signal completion
				int totalNumOfBatches,
				int batchSize,
				int maxScheduledBatches, 
				size_t batchSizeBytes,
				size_t qDataSizeBytes, 
				cl::Context &context,
				cl::CommandQueue cmdQ[],
				cl::Kernel &krnl_idct,
				std::vector<int16_t, aligned_allocator<int16_t>> &inputData,
				std::vector<uint16_t, aligned_allocator<uint16_t>> &coeffs,
				std::vector<int16_t, aligned_allocator<int16_t>> &fpgaOutput
		    )
{

EVENTS_AND_WAIT_LISTS:
    std::vector<cl::Event> writeToDeviceEvent(maxScheduledBatches);
	std::vector<cl::Event> kernelExecEvent(maxScheduledBatches);
	std::vector<cl::Event> readFromDeviceEvent(maxScheduledBatches);
    std::vector<cl::Event> *kernelWaitList   = new std::vector<cl::Event>[maxScheduledBatches];
    std::vector<cl::Event> *readBackWaitList = new std::vector<cl::Event>[maxScheduledBatches];

    cl::Buffer *inputDataBuffer      = new cl::Buffer[maxScheduledBatches];
	cl::Buffer *qInputDataBuffer     = new cl::Buffer[maxScheduledBatches];
	cl::Buffer *fpgaOutputDataBuffer = new cl::Buffer[maxScheduledBatches];
    
	cl_int err;
    
    // Set kernel arguments that need to be set once.
    int m_dev_ignore_dc = 1;
    OCL_CHECK(err, err = krnl_idct.setArg(3, m_dev_ignore_dc));
	OCL_CHECK(err, err = krnl_idct.setArg(4, batchSize));
    bool schedulingBufferFull = false;
    unsigned int earlyKernelEnqPtr;

BATCH_PROCESSING_LOOP:
	for (int batch_no = 0; batch_no <totalNumOfBatches ; ++batch_no)
	{

        earlyKernelEnqPtr = batch_no % maxScheduledBatches;
        
        if(batch_no > (maxScheduledBatches-1)) 
        {
            (writeToDeviceEvent[earlyKernelEnqPtr]).wait();
            (kernelExecEvent[earlyKernelEnqPtr]).wait();
            (readFromDeviceEvent[earlyKernelEnqPtr]).wait();
            (kernelWaitList[earlyKernelEnqPtr]).clear();
            (readBackWaitList[earlyKernelEnqPtr]).clear();
        }
        
        OCL_CHECK(err,
				inputDataBuffer[earlyKernelEnqPtr] = 
		                      cl::Buffer(context,
		                                 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		                                 batchSizeBytes,
		                                 &inputData[batch_no * (batchSizeBytes/sizeof(int16_t))],
		                                 &err));
		OCL_CHECK(err,
				qInputDataBuffer[earlyKernelEnqPtr] = 
		                      cl::Buffer(context,
		                                 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
		                                 qDataSizeBytes,
		                                 &coeffs[0],
		                                 &err));
		OCL_CHECK(err,
				fpgaOutputDataBuffer[earlyKernelEnqPtr] = 
		                      cl::Buffer(context,
		                    		     CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
		                    		     batchSizeBytes,
		                    		     &fpgaOutput[batch_no * (batchSizeBytes/sizeof(int16_t))],
		                                 &err));

		// Set the kernel arguments
		OCL_CHECK(err, err = krnl_idct.setArg(0, inputDataBuffer[earlyKernelEnqPtr]));
		OCL_CHECK(err, err = krnl_idct.setArg(1, qInputDataBuffer[earlyKernelEnqPtr]));
		OCL_CHECK(err, err = krnl_idct.setArg(2, fpgaOutputDataBuffer[earlyKernelEnqPtr]));
        OCL_CHECK(err,
				   err = cmdQ[0].enqueueMigrateMemObjects(
														{inputDataBuffer[earlyKernelEnqPtr],qInputDataBuffer[earlyKernelEnqPtr]},
														0,
														NULL,
														&writeToDeviceEvent[earlyKernelEnqPtr] 
													   )
	              );
        kernelWaitList[earlyKernelEnqPtr].push_back(writeToDeviceEvent[earlyKernelEnqPtr]);
        OCL_CHECK(err,
	    		  err = cmdQ[1].enqueueTask(krnl_idct,
                                         &kernelWaitList[earlyKernelEnqPtr], 
	    				  	  	  	  	 &kernelExecEvent[earlyKernelEnqPtr]
                                         ) 
                  );
	    readBackWaitList[earlyKernelEnqPtr].push_back(kernelExecEvent[earlyKernelEnqPtr]);
	    OCL_CHECK(err,
				  err = cmdQ[2].enqueueMigrateMemObjects(
						  	  	  	  	  	  	  	  {fpgaOutputDataBuffer[earlyKernelEnqPtr]},
												      CL_MIGRATE_MEM_OBJECT_HOST,
												      &readBackWaitList[earlyKernelEnqPtr], 
	                                                  &readFromDeviceEvent[earlyKernelEnqPtr])
                  );
   	}
    // Wait for all tasks to finish
    OCL_CHECK(err, err = cmdQ[0].finish());
    OCL_CHECK(err, err = cmdQ[1].finish());
    OCL_CHECK(err, err = cmdQ[2].finish());
    for(int v = 0; v<maxScheduledBatches; v++)
    {
     kernelWaitList[v].clear();
     readBackWaitList[v].clear();   
    }
	
	done = true;
	delete [] kernelWaitList;
	delete [] readBackWaitList;
	delete [] inputDataBuffer;
	delete [] qInputDataBuffer;
	delete [] fpgaOutputDataBuffer;
}
