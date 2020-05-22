/**********
 Copyright (c) 2020, Xilinx, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********/


#include <vector>
#include <assert.h>
#include <stdlib.h> 
#include <chrono>
#include <thread>
#include <atomic>

#include "xcl2.hpp"
#include "types.hpp"
#include "idct.hpp"
#include "krnl_idct_wrapper.hpp"
#include "utilities.hpp"

/* Define number of idct batches to be scheduled in one go
enables potential overlapping of data movement and kernel
execution if (MAX_SCHEDULED_BATCHES > 1) */
#define MAX_SCHEDULED_BATCHES 8 


// IDCT Size and related parameters
#define IDCT_SIZE 64 // Elements


// Batch of blocks processed by single kernel call
#define BATCH_SIZE 1024*16


// Total no. of batches to be processed, equal no. of
// kernel calls.    
#define NUM_OF_BATCHES (256)
#define NUM_QS 3



int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 6)
    {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        std::cout << "------ OR ------"<<std::endl;
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <Batch Size>" << std::endl;
        std::cout << "------ OR ------"<<std::endl;
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <Batch Size> <Total No. of Batches>" << std::endl;
        std::cout << "------ OR ------"<<std::endl;
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <Batch Size> <Total No. of Batches> <Max. Scheduled Batches>" << std::endl;
        std::cout << "------ OR ------"<<std::endl;
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <Batch Size> <Total No. of Batches> <Max. Scheduled Batches>"
                               <<"<Emulate all Batches(1/0)>" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string binaryFile = argv[1];
    long long unsigned int idctBlockSize = IDCT_SIZE;
    long long unsigned int numBlocks = BATCH_SIZE * NUM_OF_BATCHES;
    long long unsigned int batchSize = BATCH_SIZE;
    long long unsigned int totalNumOfBatches = NUM_OF_BATCHES;
    long long unsigned int maxScheduledBatches = MAX_SCHEDULED_BATCHES;
    
    if ( argc >= 3 )
    {
        batchSize = atoi(argv[2]);
        numBlocks = batchSize * totalNumOfBatches;
    }

    if ( argc >=4 )
    {
        totalNumOfBatches = atoi (argv[3]);
        numBlocks = batchSize * totalNumOfBatches;
    }

    if ( argc >=5 )
    {
        maxScheduledBatches = atoi (argv[4]);
    }
    
    int enableFullLenEmulation = 0;
    
    if ( argc >=6 )
    {
        enableFullLenEmulation = atoi (argv[5]);
    }

    std::string emuMode("System HW");
    char *emuModePtr = getenv("XCL_EMULATION_MODE");
    
    if(emuModePtr!=nullptr)
        emuMode = std::string(emuModePtr);
    
    bool isEmulationModeEnabled =    (emuMode.compare("sw_emu")==0)
                                   | (emuMode.compare("hw_emu")==0);
    
    std::cout<<"\n------ Identified run mode : "<<emuMode<<std::endl;
    if ( isEmulationModeEnabled  && (!enableFullLenEmulation) )
    {
        std::cout<<"------ Reducing Batch size and Total no. of Batches for faster emulation ..."<<std::endl;
        std::cout<<"------ For running full length enumation use the following style to launch."<<std::endl;
        std::cout << "\t" << argv[0] << " <XCLBIN File> <Batch Size> <Total No. of Batches> <Max. Scheduled Batches>"
                               <<" 1" << std::endl;
        batchSize = 32; 
        totalNumOfBatches = maxScheduledBatches;
        numBlocks = batchSize * totalNumOfBatches;
    }

    assert ( numBlocks > 0 );
    assert ( numBlocks % batchSize == 0 );
    // The max number of batches scheduled at any given time should be a factor of numBlocks
    assert( numBlocks % maxScheduledBatches == 0 );
    // Max batches to be scheduled should be less than total number of batches
    assert( totalNumOfBatches >= maxScheduledBatches);
    //check idct and kernel parameters
    if(idctBlockSize !=  64)
    {
       printHeader(); 
       std::cout<<"The only IDCT size supported currently is 64\n";
       printFooter();
       exit(1);
    }
    
    size_t qDataSizeBytes = idctBlockSize * sizeof(uint16_t);
    size_t batchSizeBytes =  batchSize * idctBlockSize * sizeof(int16_t);
    
    printHeader();
    std::cout<<"Total Input data to be processed = "
             << (numBlocks*idctBlockSize*sizeof(int16_t))/(1024)
             <<" KBs ( "<<(numBlocks*idctBlockSize*sizeof(int16_t))/(1024*1024)
             <<" MBs) ("
             <<(numBlocks*idctBlockSize*sizeof(int16_t))/(1024*1024*1024)
             <<" GBs)"
             <<std::endl;
    std::cout<<"IDCT block size = " <<idctBlockSize<<std::endl;
    std::cout<<"Batch Size ( IDCT blocks processed per kernel call) = "<<batchSize<<std::endl;
    std::cout<<"Total Number of batches to be processed( Total Kernel Calls) = "<<totalNumOfBatches<<std::endl;
    std::cout<<"Max. scheduled kernel calls(Overlap Compute and data transfers) = " << maxScheduledBatches<<std::endl;
    std::cout<<"Total Number blocks to be processed = " << numBlocks << std::endl;
    printFooter();
    
    // Allocate Data for input/output
    std::cout<<"------ Starting to allocate memory for CPU/FPGA data storage\n";
    std::vector<int16_t, aligned_allocator<int16_t>> inputData(idctBlockSize * numBlocks);
    std::vector<uint16_t, aligned_allocator<uint16_t>>  qInData(idctBlockSize);
    std::vector<int16_t, aligned_allocator<int16_t>> cpuOutput(idctBlockSize * numBlocks);
    std::vector<int16_t, aligned_allocator<int16_t>> fpgaOutput(idctBlockSize * numBlocks);
    std::cout<<"------ Successfully allocated memory for input/ouput data storage.\n";

    std::cout<<"------ Initializing input data arrays and DCT co-efficients...\n";
    for(size_t i = 0; i < (size_t)numBlocks; i++)
    {
        for(size_t j = 0; j < (size_t)idctBlockSize; j++)
        {
            assert(i*idctBlockSize+j < inputData.size());
            inputData[i*idctBlockSize + j] = j;
        }
    }
    
    for(size_t j = 0; j < (size_t)idctBlockSize; j++)
    {
        qInData[j] = j;
    }
    std::cout<<"------ Done Initilizing input data arrays.\n";    
    printHeader();
    
FPGA_DEVICE_OPENCL_SETUP:
    cl_int err;
    cl::Context context;
    cl::CommandQueue cmdQ[NUM_QS];
    cl::Kernel krnl_idct;
    auto devices = xcl::get_xil_devices();
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins {{ fileBuf.data(), fileBuf.size() } };
    int valid_device = 0;
    
    for (unsigned int i = 0; i < devices.size(); i++)
    {
        auto device = devices[i];
        OCL_CHECK(err, context = cl::Context(
                                             { device },
                                             NULL,
                                             NULL,
                                             NULL,
                                             &err));
        for(int qn=0; qn<NUM_QS; qn++)
        {
            OCL_CHECK(err, cmdQ[qn] = cl::CommandQueue(
                                                   context,
                                                   { device },
                                                   CL_QUEUE_PROFILING_ENABLE,
                                                   &err));

        }
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context,
                            { device },
                            bins,
                            NULL,
                            &err
                           );
        if (err != CL_SUCCESS)
        {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        }
        else
        {
CREATE_KERNEL:
           std::string kernelName = "krnl_idct";
           std::cout << "Device[" << i << "]: programming successful!\n";           
           std::cout << "------Created Handle for Kernel: " << kernelName <<std::endl;            
           OCL_CHECK(err, krnl_idct = cl::Kernel(program, kernelName.c_str(), &err));
           valid_device++;
           break;
        }
    }
    if (valid_device == 0)
    {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
    printFooter()

    
    std::atomic<bool> fpgaDone(false);
    std::atomic<bool> cpuDone (false);
    auto fpgaStartTime = std::chrono::high_resolution_clock::now();
    decltype(fpgaStartTime) fpgaEndTime; 

    /*
        fpgaThread:
                    Create a thread for launching FPGA based IDCT
    */
    auto fpgaThread = std::thread(    runFPGA,
                                    std::ref(fpgaDone),
                                    totalNumOfBatches,
                                    batchSize,
                                    maxScheduledBatches,
                                    batchSizeBytes,
                                    qDataSizeBytes,
                                    std::ref(context),
                                    cmdQ,
                                    std::ref(krnl_idct),
                                    std::ref(inputData),
                                    std::ref(qInData),
                                    std::ref(fpgaOutput)

        );

    auto cpuStartTime = std::chrono::high_resolution_clock::now();
    decltype(cpuStartTime) cputEndTime;
    int ignoreDC = 1;

    /*
        cpuThread:
            Create a thread for launching CPU based IDCT
    */
    auto cpuThread = std::thread (
                                    runCPU,
                                    std::ref(cpuDone),
                                    numBlocks,
                                    std::ref(inputData),
                                    std::ref(qInData),
                                    std::ref(cpuOutput),
                                    ignoreDC
                                  );
    
    std::cout<<"------ Launched CPU and FPGA Thread and Monitoring Execution. "<<std::endl;
    
    measureExecTimes(isEmulationModeEnabled,cpuDone,fpgaDone,cputEndTime,fpgaEndTime);
    // Join both the thread and release resources
    fpgaThread.join();
    cpuThread.join();
    std::cout<<"------ All Task finished !"<<std::endl;
    std::cout<<"------ Done with CPU and FPGA based IDCTs"<<std::endl;
    /*
        **************************************************************

        verifyResults:
                    Comparison cpu vs. fpga results

        **************************************************************
    */

    std::cout << "------ Runs complete validating results" << std::endl;
    int krnl_match = 0;
    for (size_t i = 0; i < (size_t)idctBlockSize * numBlocks; i++)
    {
        if (fpgaOutput[i] != cpuOutput[i])
        {
            std::cout<<"------ Error: Result mismatch"<<std::endl;
            std::cout<<"i = "<< (int)i<<" CPU reults = "<<cpuOutput[i]<<" FPGA Results = "<< fpgaOutput[i]<<std::endl;
            krnl_match = 1;
            break;
        }
    }

    /*
        **************************************************************

        Performance Comparison:
                                Throughput measurements : CPU vs FPGA

        **************************************************************
    */

    std::chrono::duration<double> cpuExecutionTime = cputEndTime - cpuStartTime;
    std::chrono::duration<double> fpgaExecTime = fpgaEndTime - fpgaStartTime;
    double cpuTPUT = (double)(numBlocks * idctBlockSize * sizeof(int16_t))/ cpuExecutionTime.count() / (1024.0*1024.0);
    double fpgaTPUT = (double) (numBlocks * idctBlockSize * sizeof(uint16_t)) / fpgaExecTime.count() / (1024.0*1024.0);

    if(!isEmulationModeEnabled)
    {
        std::cout << "CPU Time:        " << cpuExecutionTime.count() << " s" 
                  <<" ( "<<cpuExecutionTime.count()*1000
                  << "ms )"
                  <<std::endl;
    
        std::cout << "CPU Throughput:  " 
                  << cpuTPUT << " MB/s" << std::endl;
        std::cout << "FPGA Time:       " << fpgaExecTime.count() << " s" 
                  <<" ("<< fpgaExecTime.count()*1000 
                  << " ms )" 
                  << std::endl;
        std::cout << "FPGA Throughput: " 
                  << fpgaTPUT << " MB/s" << std::endl;
    } 
    std::cout << "------ TEST " << (krnl_match ? "FAILED" : "PASSED")<<" ------" << std::endl;
    if(!isEmulationModeEnabled)
    {
        printHeader();
        std::cout<<"FPGA accelerations ( CPU Exec. Time / FPGA Exec. Time): "<< fpgaTPUT/cpuTPUT <<std::endl;
        printFooter();
    }
    return (krnl_match ? EXIT_FAILURE : EXIT_SUCCESS);
}
