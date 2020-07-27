# Becoming Familiar with the IDCT Application

  - [Setting Up the Vitis Environment](#setting-up-the-vitis-environment)
  - [Overview of the Application Source Code](#overview-of-the-application-source-code)
  - [Theoretical Application Performance Estimation](#theoretical-application-performance-estimation)
    - [Application Categories and Memory Subsystem Performance](#application-categories-and-memory-subsystem-performance)
    - [Kernel Performance Estimation](#kernel-performance-estimation)
    - [Application Performance Estimation](#application-performance-estimation)
    - [Calibrating Kernel Initiation Interval (II)](#calibrating-kernel-initiation-interval-ii)
  - [Running the Application](#running-the-application)
    - [Running Software Emulation](#running-software-emulation)
    - [Running the Hardware Emulation](#running-the-hardware-emulation)
  - [Summary](#summary)

This lab is designed to teach the fundamentals of the Vitis&trade; development environment and programming model. Its contents are tailored to familiarize you with basic OpenCL APIs, software and hardware emulation flows, application performance estimation, profiling performance, and identifying how to optimize host code and kernel hardware implementation.

The kernels or the functions used for FPGA acceleration in this lab are slightly different hardware implementations of the Inverse Discrete Cosine Transform (IDCT) algorithm, a function widely used for transform coding in applications like audio/image codecs such as JPEG and High Efficiency Video Coding(HEVC).

## Setting Up the Vitis Environment

1. Open a new terminal by right-clicking anywhere in the desktop area and by selecting **Open Terminal**.

1. Set up the Vitis environment.  

    ```bash
    source $AWS_FPGA_REPO_DIR/vitis_setup.sh
    ```
    
	>**Note**: The `Vitis_setup.sh` script might generate warning messages, which can be safely ignored.*

1. Go to the design folder and investigate the design files.

    ```bash
    # Go to the lab directory
    export LAB_WORK_DIR=/home/centos/src/project_data/
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    ls
    ```
	  The `src` folder contains the kernel source files and host code. The `Makefile` is provided for design compilation and execution. Open the make file and take a look at it.
    ```
    vi Makefile
    ```

    In the 'platform selection' section, the default target platform is set as `xilinx_aws-vu9p-f1_shell-v04261818_201920_1` which is the AWS F1 platform.

    The next couple of lines define the design files location and file names. Following that is the host compiler settings, kernel compiler, and linker settings.  You do not need to modify any of the options here but you may want to play with them after finishing this tutorial.

    Exit **Makefile** view window and let's take a look at the design files.

## Overview of the Application Source Code

The project comprises of multiple files under the `src` directory. The following is the list with brief description:

* `host.cpp` contains main function, performs data initialization, allocation and forks CPU and FPGA threads.

* `krnl_idct_wrapper.hpp` contains the **_runFPGA_** thread that coordinates kernel execution on FPGA accelerator.

* `idct.cpp` contains the software implementation of IDCT (reference model) that will run as a separate thread on CPU or host side.

* `krnl_idct.cpp` contains the code for the IDCT kernel (custom accelerator) for running it on FPGA.

* `krnl_idct_med.cpp` contains a version of the IDCT kernel for demonstration purposes only **_with II=4_**.

* `krnl_idct_noflow.cpp` contains a version of the IDCT kernel for demonstration purposes only without dataflow optimization.

* `krnl_idct_slow.cpp` contains a version of IDCT kernel for demonstration purposes only **_with II=8_**.

* `types.hpp` defines IDCT I/O and DDR interface types.

* `utilities.hpp` contains utility function used for printing execution information to console.

1. Open the `krnl_idct.cpp` file to see the code structure used for the IDCT kernel.
	* The **idct** function is the core algorithm implemented as a custom hardware accelerator.
	* This computationally heavy function can be highly parallelized on an FPGA using a parallel datapath providing significant acceleration compared to a CPU-based implementation.
	* The **krnl_idct** function is the top-level for the custom hardware accelerator. Memory interface properties for the accelerator are specified in this function allowing us to utilize the maximum potential bandwidth of the device DDR memory.
	* The **krnl_idct_dataflow** function is called by the top-level function and encapsulates the main functions of the accelerator.
	* The **read_blocks** function reads data from device DDR memory written by the host application and streams(writes to an output FIFO storage) this data to the **execute** function.
	* The **execute** function receives the streaming data and, for each 8x8 block received, calls the **idct** function to perform the actual computation and streams the results to and output FIFO.
	* The **write_blocks** function receives the streaming results from the **execute** function through a FIFO and writes them back to device DDR memory which are read by the host application.
	
2. The kernels in the source files `krnl_idct_slow.cpp`, `krnl_idct_med.cpp`, and `krnl_idct_noflow.cpp` describe very similar kernels. These are almost identical but with variations on micro-architecture for hardware implementation, which can be achieved by adding and removing some **Vitis HLS pragmas**. These kernels are designed to *show how different micro-architectural variations can be used to trade-off FPGA hardware resources with performance*. All experiments in this lab are related to the **"krnl_idct"** kernel only.

3. Open the `host.cpp` file and observe the following sequence of steps carried out by host application:  
	 1. **Command Line Arguments:** The **main** first parses the command line arguments. These command line arguments can be used to control the total number of IDCT blocks to be processed and how many IDCT blocks the kernel processes in one go (call made from host side with appropriate data size). The latter is also called batch size.

    1. **Test Vectors:** Next, test vectors are allocated and initialized

    1. **Device Programming:**  In the next phase, the Xilinx device search is performed and the first found device is programmed with the user-provided Xilinx binary, which is also known as AFIs for AWS. An associated OpenCL Context and Command Queue are also created.

    1. **Kernel Handle:** After the device is programmed with the FPGA image, a kernel handle is created which can be used to process data.

    2. **CPU and FPGA Threads:** Next, the host launches two separate threads **runCPU** and **runFPGA** for the CPU run and the FPGA accelerated run, respectively, at the same time.

    3. **Execution Time Measurement:** Once the CPU and the FPGA threads are forked, the main thread calls the `measureExecTimes` function which waits on these threads to finish. It also samples the time when these threads finish to calculate the execution times.

    4. **Output Validation and FPGA Peformance:** Once the execution is complete, the FPGA results are validated by comparing against the reference output generated by the CPU run and the performance comparison is printed out. The performance numbers give metrics such as execution times and throughput(data processing rates).

 1. Now go to `host.cpp` line 47. Three important parameters are defined here: **BATCH_SIZE**, **NUM_OF_BATCHES**, and **MAX_SCHEDULED_BATCHES**:
 
    * **BATCH_SIZE**: Defines the number of IDCTs carried out by a single call to the kernel. It can also be specified at runtime as a host application command line argument.
    * **NUM_OF_BATCHES**: Defines the number of batches to be processed in total by the application.
    * **MAX_SCHEDULED_BATCHES**: If multiple batches of input data are to be processed then there is a potential for IDCT compute and data transfer to overlap for different batches. This parameter specifies how many batches can be scheduled to be processed in an overlapping fashion. Essentially, it defines the number of duplicate resources, such as **cl::Buffer** objects.

4. Go to label **FPGA_DEVICE_OPENCL_SETUP** near line 186 of the `host.cpp` file:
    
    This section sets up OpenCL environment for **runFPGA** thread. It is very typical setup for Vitis acceleration applications very familiar to developers with prior OpenCL experience. Following section provides the briefs of used APIs. 
   * **xcl::get_xil_devices**: Xilinx provided API, returns a list of Xilinx devices connected to the host
   * **xcl::read_binary_file**: Xilinx provided API, reads a compiled binary file for FPGA
   * **cl::Program::Binaries**: Creates a binary file object from raw binary file which can be used to create a OpenCL program  associated with a device, essentially programming FPGA device there in.
   * **cl::Program**: Creates a cl::Program objects and also programs FPGA device. The programmed device may have multiple kernels compiled inside single programs so created object also provides a reference that can be used to create  handles to different kernels.
   * **cl::Kernel**: Creates a kernel object given the cl::Program handle and kernel name as string. In the next lab, you will change this name string often. In this case, you have multiple kernels but here one kernel, namely "krnl_idct", is used.


  >**NOTE**: Many function calls and object construction call use a macro **OCL_CHECK** which is used to parse the return status of type **cl_int**. It errors out after OpenCL call, if the call does not complete as expected.

5. Open the `krnl_idct_wrapper.hpp` file.
   
   This file defines a thread wrapper function for the use of host side. It has the host side code that runs on the CPU and interacts with the FPGA accelerator using OpenCL C/C++ APIs. The host code structure and the use of OpenCL C/C++ APIs is very typical and similar to the one used in any other heterogeneous acceleration framework, such as the ones based on GPUs. 
   
   The **runFPGA** wrapper function takes as input:
      * data size
      * batching info
      * OpenCL objects like command queue, context, and kernel
      * input data to be transformed passed as a vector
      * input vector for IDCT coefficients
      * an output vector.

    See how **runFPGA** performs IDCT compute using FPGA kernel, which is very similar to any other device kernel such as GPU used for compute with OpenCL APIs. The basic logic behind this pieces of code is as follows:

    First, it creates vector of events to define the dependencies between different compute and data migration tasks such as:
        
      * Kernel compute should be triggered only when host to device data transfer completes

      * Output data transfer from device back to host should trigger only when kernel compute completes

      * The next batch of input data (triggering processing of next batch of data) transfer from host to device should wait for current transfer
       
    Next **cl::Buffers** objects for **input**, **output**, and **coefficients** are created. These are used to transfer data from host to device (FPGA) and back. The main loop runs over a number of batches to be processed and inside this loop, **cl::Buffer** objects are initialized with proper attributes and host pointers. Since the input data is contiguous (stored as single block of multiple input batches), the host pointer is provided by calculating the offset based on current batch number and batch size over the input data. Host side application uses a number of **cl::Buffer** objects each for input and output data; this number can be selected by passing a command line argument to the host application. These sets of buffers are used in a circular buffer style to create a pool of simultaneously enqueued tasks which allows compute and data transfers to overlap between different kernel calls.

    * **setArg** is used to set different kernel arguments.

    * **enqueueMigrateMemObjects** is used to enqueue data transfer request between host and FPGA device.

    * **enqueueTask** is used to enqueue kernel on command queue for execution. It takes as input list of events that should be completed and also produces an event to signal end of execution.
    
 2. Open the file `idct.cpp`.

	The **runCPU** function is defined here. It takes a vector of inputs and for each 8x8 block, it calls **idctSoft**, a reference implementation of the IDCT to be ran on the CPU. The **idctSoft** function is the reference software implementation of the IDCT algorithm, used in this example to check the results coming back from the FPGA.

All of the OpenCL API functions used here are documented by the [Khronos Group](https://www.khronos.org), the maintainers of OpenCL, the open standard for parallel programming of heterogeneous systems.

## Theoretical Application Performance Estimation

You can estimate application performance and also the acceleration versus software performance if the software profiling results are available. The [recommended methodology guide](https://www.xilinx.com/html_docs/xilinx2020_1/vitis_doc/methodologyacceleratingapplications.html#wgb1568690490380
) describes this process in detail. For this lab, you will estimate hardware performance using pen and paper style calculations. Once the hardware results become available, you can correlate estimated results with measured ones and also reason about theses number in case of differences.

### Application Categories and Memory Subsystem Performance

The system architecture and memory subsystems play a crucial role in defining the performance of overall application. In general depending on the characteristics applications can be categorized in two broad categories:

 *  **Compute limited applications** : In the case of compute limited applications, the performance is limited by compute complexity. The challenge is to architect accelerators/compute units and compose a system that may consist of multiple compute units that can meet the required throughput or saturate memory bandwidth.
   
 *  **Memory bandwidth or I/O limited applications**: These type of applications do carry computational complexity and need dedicated accelerators but it is relatively easier to come up with accelerators/compute units that can easily saturate memory bandwidth and reach maximum achievable performance for the given memory bandwidth.
 
 In general, the lowest memory bandwidth component in the memory sub-system defines the upper bound to max achievable performance, but in case of memory bandwidth limited applications it plays a more critical role. In case of AWS F1 FPGA instance, the I/O data movements between host and FPGA kernel happen as follows:
 
 1. Input data moves from Host through PCIe&reg; to device DDR memories
 2. Kernel/CUs reads input data from device DDR memories for processing
 3. Kernel writes output data back to DDR memories after processing
 4. Host reads output data from device DDR memories
 
 2D IDCT carries a significant potential for acceleration compared to CPU performance as is evident from the final results but in general it is a memory bandwidth limited kernel so its performance is capped by any component in the memory subsystem that has minimum bandwidth. You can use memory subsystem data transfer rate specs from datasheets but to have better estimates, it is recommended to use a Xilinx run time utility such as xbutil to perform bandwidth test and use its measurement for estimation. From the measurements for AWS F1 instance, it is found that this bottleneck bandwidth is around 8.4 GB/s. Another important number that you can use for estimating the latency of kernel is the device DDR read and write bandwidths which can be taken as 12GHz(250MHz * 64 Bytes *0.75). Here the assumption is that DDR interfaces are operating at 75% of maximum bandwidth depending on kernel read/write patterns. 

### Kernel Performance Estimation

 By looking at the kernel source code in `krnl_idct.cpp`, around line 370 you can easily see that the dataflow pipeline consists of four function calls, read_blocks, execute, and two calls to write_blocks. The write_block is called twice once for loading co-efficients and then for laoding input data. The size of co-efficients is very small as compared to main input data chunk that is processed so it can be ignored. By looking at the definitions of these functions it is clear that all functions can be pipelined with II=1 except "execute" which performs 2D IDCT(8x8). The reason being it needs 1024(8 * 8 * 2 * 8) bits to process per cycle which is not possible since it reads and writes from device DDR memories with 512-bit device interfaces. Essentially read_blocs and write_blocks which connect to memories can provide IDCT with only 512-bit wide sample every cycle. The kernel latency and throughput can be estimated as follows:

  ```
    Data Processed per Call     = 8*8*2 (size of short type in bytes) = 128 Bytes
    Kernel Frequency fmax       = 250 MHz
    Initiation Interval (II)    = 2
    Throughput (Max)            = Fmax * (Data processed per Call) / II = 16 GB/s
    Throughput (Capped)         = Min (IDCT Throughput(max Measured Device DDR Bandwidth)= 12 GB/s
  ``` 
Here, fmax of 250 MHz is used and also since the maximum bandwidth available to kernel from device DDR memories is estimated as 12 GB/s, so throughput is capped at 12 GB/s.

  ```
   Kernel Latency = Input Data Block Size / Throughput =  (16 MB) / (12 GB/s) = 1.33 ms
  ``` 
 For the kernel latency estimate, capped throughput and input data size of 16 MBs is used. Here data 16 MBs is assumed to be the size of total input data that is used to perform IDCT on multiple blocks. The rational for using such a large size is that IDCT is used in applications like image and video processing where the bulk of data is available to process in one go. Also, larger data size movements through memory subsystem using DMA and kernel burst read and write from device DDR memories can significantly improve performance.

### Application Performance Estimation

Once the kernel performance estimates are available it is very easy to estimate the system performance given the PCI bandwidth estimates are available. They define data movements rates to and from host global memory.
   ```
   Overall Application Performance = min(IDCT Throughput (capped), PCI Bandwidth)
   Overall Application Performance = min(12 GB/s, 8.4 GB/s) = 8.4 GB/s  
  ``` 
So, in the case of IDCT overall system performance estimate is 8.4 GB/s limited by PCIe performance. Once you have built the whole application, you can come back and compare the performance with this number and also compare the kernel latency using profile summary reports in the Vitis Analyzer.

### Calibrating Kernel Initiation Interval (II)

From the overall application performance which is 8.4 GB/s and Kernel Max Throughput of 16 GB/s with II=2 , you can conclude that slowing down the kernel by 2x i.e. by increasing II=4 with max kernel throughput of 8 GB/s will not degrade overall performance by much. But having II=4 may result in saving resources. You will experiment with II in the next lab and also observe its effect on performance and resources.

## Running the Application

Vitis applications can run in multiple modes. These modes include software emulation, hardware emulation, and actual system run on an FPGA acceleration card. The Vitis software platform provides two emulation flows which allow testing the application before deploying it on the F1 instance or actual FPGA accelerator. These flows are referred to as software emulation and hardware emulation modes.

- **Software emulation** is used to identify syntax issues and verify the behavior of application. In this mode, the C/C++ model of kernel is used for execution instead of any hardware or hardware model.
- **Hardware emulation** is used to get performance estimates for the accelerated application with more realistic results than software emulation flow but it can only provide very accurate estimates about the hardware accelerated function/kernel performance and FPGA resource usage, the memory interfaces and data transfers use approximate models only. In this mode, the RTL model is used for kernel.
  
### Running Software Emulation

1. Run the following commands in the terminal window.

    ```bash
    source $AWS_FPGA_REPO_DIR/vitis_setup.sh
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    make run TARGET=sw_emu
    ```
    This will run through software emulation and print out messages as shown below to indicate that the software emulation process has finished successfully. Output log signals the execution mode such as sw_emu/hw_emu or system hardware. The emulation mode is defined by the environment variable **XCL_EMULATION_MODE** set by mthe akefile which is read by host executable to identify run mode, host application tries to identify run mode, to change input data size for quick execution in case of emulation mode.

    ```bash
    ------ Identified Run mode : sw_emu
    .
    .
    .
	=====================================================================
    ------ Launched CPU and FPGA Thread and Monitoring Execution.
    =====================================================================
    [FPGA Time(     1s ) : ]  [CPU Time(    1s ) : ]
    Execution Finished
    =====================================================================
    ------ All Task finished !
    ------ Done with CPU and FPGA based IDCTs
    ------ Runs complete validating results
    ------ TEST PASSED ------
    ```

    The generated files are put into `build` folder under `design` directory. You can use `ls` command to investigate the generated files.

### Running the Hardware Emulation

1. After the software emulation finishes successfully, you can move forward and run the design in hardware emulation mode. The corresponding command is:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    make run TARGET=hw_emu
    ```

	- In hardware emulation, the host code is compiled to run on the x86 processor and the kernel code is compiled into a hardware model (known as RTL or Register Transfer Level) which runs in an RTL simulator.
	- The build and run cycle takes longer because the kernel code is compiled into a detailed hardware model which is slower to simulate.
	- The more detailed hardware simulation allow more accurate reporting of kernel and system performance.
	- This flow is also useful for testing the functionality of the logic that will go in the FPGA.
	- The hardware emulation is complete when the following messages are displayed:

    ```bash
	=====================================================================
	------ Launched CPU and FPGA Thread and Monitoring Execution. 
	=====================================================================

	[FPGA Time(    21s ) : ]  [CPU Time(    1s ) : Done. ]  

	Execution Finished
	=====================================================================
	------ All Task finished !
	------ Done with CPU and FPGA based IDCTs
	------ Runs complete validating results
	------ TEST PASSED ------
	INFO::[ Vitis-EM 22 ] [Time elapsed: 1 minute(s) 19 seconds, Emulation time: 0.132265 ms]
	Data transfer between kernel(s) and global memory(s)
	krnl_idct_1:m_axi_gmem0-DDR[0]                 RD = 32.000 KB              WR = 0.000 KB        
	krnl_idct_noflow_1:m_axi_gmem1-DDR[1]          RD = 0.000 KB               WR = 0.000 KB      
	krnl_idct_noflow_1:m_axi_gmem2-DDR[2]          RD = 0.000 KB               WR = 0.000 KB      
	krnl_idct_slow_1:m_axi_gmem0-DDR[0]            RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_slow_1:m_axi_gmem1-DDR[1]            RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_slow_1:m_axi_gmem2-DDR[2]            RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_1:m_axi_gmem1-DDR[1]                 RD = 1.000 KB               WR = 0.000 KB        
	krnl_idct_1:m_axi_gmem2-DDR[2]                 RD = 0.000 KB               WR = 32.000 KB       
	krnl_idct_med_1:m_axi_gmem0-DDR[0]             RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_med_1:m_axi_gmem1-DDR[1]             RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_med_1:m_axi_gmem2-DDR[2]             RD = 0.000 KB               WR = 0.000 KB        
	krnl_idct_noflow_1:m_axi_gmem0-DDR[0]          RD = 0.000 KB               WR = 0.000 KB 
    ```
   
>**NOTE:** The Vitis emulation info message "INFO::[ Vitis-EM 22 ]" lists interfaces used by all the kernels and data transferred over them, since in this experiment only kernel "krnl_idct" is used by host so only data transfers for this kernel have non-zero values. Also during the emulation host application identified the emulation mode and used a small data size to ensure short emulation time. To do the full length emulation, please use makefile as follows:

 ```bash
make run TARGET=sw_emu EMU_FULL=1 BATCH_SIZE=1024 NUM_OF_BATCHES=16 MAX_SCHEDULED_BATCHES=8
 ```   
Here, you can choose the batch size, number of batches, maximum scheduled batches, and full length emulation mode by setting **EMU_FULL=1**. If it is set to '0', the emulation uses small data set for emulation. For the actual FPGA run, full length data is used. In general, the setting of these parameter is such that **NUM_OF_BATCHES** = N * **MAX_SCHEDULED_BATCHES**, where N is an integer number.
   
## Summary

In this lab, you learned the following:

- Working with the IDCT application source code structure
- OpenCL C/C++ APIs used to communicate between Host and FPGA
- Running an IDCT application in different emulation Modes
- Differences between software and emulation flows

---------------------------------------

<p align="center"><b>
Start the next lab: <a href="lab_03_vitis_analyzer.md">Application Performance Analysis</a>
</b></p> 