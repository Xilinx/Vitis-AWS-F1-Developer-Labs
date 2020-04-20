## Becoming Familiar With IDCT Application

This lab is designed to teach the fundamentals of the Vitis development environment and programming model. This includes: familiarizing with OpenCL, understanding software and hardware emulation flows, profiling performance and identifying how to optimize host and kernel code.

The kernel or the function ( it can be a set of function also) used for acceleration in this lab is an Inverse Discrete Cosine Transform (IDCT), a function widely used for transform coding in applications like audio/image codecs such as HEVC.


### Setting Up Vitis Environment

1.  Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**.

1.  Set up the Vitis environment.  

    ```bash
    cd $AWS_FPGA_REPO_DIR
    source vitis_setup.sh
    ```
	*Note: the Vitis_setup.sh script might generate warning messages, but these can be safely ignored.*

1.  Go to design folder and investigate the files.
    ```bash
    # Go to the lab directory
    export LAB_WORK_DIR=/home/centos/src/project_data/
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    ls
    ```
	  The `src` folder contains the kernel source file and host code. The `Makefile` is provided for design compilation and execution. Let's open up the make file and take a look at its contents.
    ```
    vi Makefile
    ```

    In 'platform selection' section, the default target platform is set as `xilinx_aws-vu9p-f1_shell-v04261818_201920_1` which is the AWS F1 platform.

    The next couple of lines define the design files location and filenames. Following that is the host compiler settings and kernel compiler and linker settings. Notice that in the last line of kernel linker setting, DDR banks are assigned to each port. You don't need to modify any of the options here but you may want to play with them after finishing this tutorial.

    Exit Makefile view window and let's take a look at the design files.

### Overview of the source code used in this example

1.  The project is comprised of multiple files under src directory:
	* **idct.cpp** contains the software implementation of IDCT that will run as a separate thread on CPU or host side.
	* **krnl_idct.cpp** contains the code for IDCT kernel (custom accelerator) running on the FPGA.
	
	
	
	

1.  Open the **krnl_idct.cpp** file to see the code structure used for IDCT kernel.
	* The **idct** function is the core algorithm implemented in the custom hardware accelerator.
	* This computationally heavy function can be highly parallelized on the FPGA using a parallel datapath providing significant acceleration over a CPU-based implementation.
	* The **krnl_idct** function is the top-level for the custom hardware accelerator. Memory interface properties for the accelerator are specified in this function allowing us to utilize the maximum potential bandwidth of on-chip DRAM
	* The **krnl_idct_dataflow** function is called by the top-level function and encapsulates the main functions of the accelerator.
	* The **read_blocks** function reads from global memory values sent by the host application and streams them to the **execute** function.
	* The **execute** function receives the streaming data and, for each 8x8 block received, calls the **idct** function to perform the actual computation and streams the results back out.
	* The **write_blocks** function receives the streaming results from the **execute** function and writes them back to global memory for the host application.

1. Open the **host.cpp** file.  
	* The **main** function of the C++ program first parses the command line arguments. These command line arguments can be used to control total number of IDCT blocks to be processed and how many IDCT blocks kernel processes in one go (call) also called batch size. After this test vectors are allocated and initialized, a Xilinx device search is performed and found device is programmed with user provided xclbin (FPGA Image) and an associated OpenCL Context and Command Queue are created. Once the device is programmed with FPGA Image a kernel is created which allows to process data. After that host launches two separate threads **runCPU** and **runFPGA** for CPU run and FPGA accelerated run respectively. Once these two threads are forked the main thread calls **measureExecTimes** function which waits on these threads to finish and also samples the time when threads finish to measure execution times. Once threads finish execution results are validated and performance comparison is printed out.
	
	
1. Open the **krnl_idct_wrapper.hpp** file.
    * This file defines a wrapper function for host side to use, it has all the host side code that interacts with the FPGA accelerator using OpenCL C++ APIs just like any other acceleration like device GPUs will do. 
    * The **runFPGA** wrapper function takes as input information about data size and batching, OpenCL objects like Command Queue, Context and Kernel followed by input for data to be transformed arranged in a contiguous fashion in a vector, a vector for IDCT coefficients and an output vector.
 1. Open the file **idct.cpp**        	
	* The **runCPU** function is defined here it takes a vector of inputs and, for each 8x8 block, calls **idctSoft**, a reference implementation of the IDCT.
	* The **idctSoft** function is the reference software implementation of the IDCT algorithm, used in this example to check the results coming back from the FPGA.
1. Go to line 188 of the **host.cpp** file:
This section of the code sets up OpenCL environment for **runFPGA**. Here Xilinx provided APIs are used to look for the Xilinx devices connected to Host and also to read binary file compiled binary for FPGA platform. his section of code is where the OpenCL environment is setup in the host application. This section is typical of most Vitis application and will look very familiar to developers with prior OpenCL experience. This body of code can often be reused as-is from project to project. Following section provides the briefs of used APIs. Many function calls and object construction call use a macro called **OCL_CHECHK** this macro is used to parse the return status cl_int and error out of any OpenCL call that doesnt complete as expected.
   * **xcl::get_xil_devices**: Xilinx provided API, returns a list of Xilinx devices connected to the host
   * **xcl::read_binary_file**: Xilinx provided API, reads a compiled binary file for FPGA
   * **cl::Program::Binaries**: Creates a binary file object from raw binary file which can be used to create a OpenCL program  associated with a device, essentially programming FPGA device here.
   * **cl::Program**: Creates a cl::Program objects and also programs FPGA device. The programmed device may have multiple kernels inside single programs so created object also provides a handle that can be used to create handle to individual kernels.
   * **cl::Kernel**: Creates a kernel object given the cl::Program handle and kernel name, in this case we have only one kernel namely "krnl_idct".
   Once the OpenCL device context is setup, device is programmed and a kernel handle is available, a call to **runFPGA** is made in an separate **std::thread**
1. Open "krnl_idct_wrapper.hpp" To see how runFPGA performs IDCT compute using FPGA kernel which is very similar to any other device such as GPU used for compute using OpenCL APIs. The basic logic behind this pieces of code is as follows: It creates vector of events that will define the dependencies between different task such host to device data transfer completion that should trigger computation and so. The **cl::Buffers** objects for **input**,**ouput** and **coefficients** are created that will be used to transfer data from host to device(FPGA) and back. The main loop runs over number of batches to be processed and inside this loop **cl::Buffer** objects are initialized with proper attributes and host pointers, since the input data is a contiguous so host pointer is provided by calculating the offset based on current batch number and batch size over the input data.
    * **setArg**: is used to set different kernel arguments.
    * **enqueueMigrateMemObjects**: is used to enqueue data transfer request between host and FPGA device.
    * **enqueueTask**: is used to enqueue kernel on command queue for execution. equeueTask takes as input list of events that should be completed and also produces an event to signal completion. 
 
 All of the above API functions are documented by the [Khronos Group](https://www.khronos.org), the maintainers of OpenCL, the open standard for parallel programming of heterogeneous systems.

### Running the Emulation Flows

  Vitis provides two emulation flows which allow testing the application before deploying it on the F1 instance. The flows are referred to as software emulation and hardware emulation, respectively.
  * Software emulation is used to identify syntax issues and verify the behavior of application.
  * Hardware emulation is used to get performance estimates for the accelerated application.

1. Run below commands in the terminal window.
    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    make run TARGET=sw_emu
    ```
    This will run through software emulation and print out messages as shown in below to indicate the process finishes successfully. Output log will signal the execution mode such as sw_emu/hw_emu or system hw. The emulation mode is set by environment variable **XCL_EMULATION_MODE** which read by host executable to identify run mode.

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

1. After software emulation finishes successfully, you can move forward to run the design in hardware emulation. The corresponding command is:
    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
    make run TARGET=hw_emu
    ```

	* In hardware emulation, the host code is compiled to run on the x86 processor and the kernel code is compiled into a hardware model (known as RTL or Register Transfer Level) which is run in RTL simulator.
	* The build and run cycle takes longer because the kernel code is compiled into a detailed hardware model which is slower to simulate.
	* The more detailed hardware simulation allow more accurate reporting of kernel and system performance.
	* This flow is also useful for testing the functionality of the logic that will go in the FPGA.
	* The hardware emulation is complete when the following messages are displayed:

    ```bash
	Execution Finished
	=====================================================================
	------ All Task finished !
	------ Done with CPU and FPGA based IDCTs
	------ Runs complete validating results
	------ TEST PASSED ------
	INFO::[ Vitis-EM 22 ] [Time elapsed: 0 minute(s) 21 seconds, Emulation time: 0.0886795 ms]
	Data transfer between kernel(s) and global memory(s)
	krnl_idct_1:m_axi_gmem0-DDR[0]          RD = 32.000 KB              WR = 0.000 KB        
	krnl_idct_1:m_axi_gmem1-DDR[1]          RD = 1.000 KB               WR = 0.000 KB        
	krnl_idct_1:m_axi_gmem2-DDR[2]          RD = 0.000 KB               WR = 32.000 KB       

	INFO: [HW-EM 06-0] Waiting for the simulator process to exit
	INFO: [HW-EM 06-1] All the simulator processes exited successfully
    ```

### Analyzing the Reports  

This section covers how to locate and read the various reports generated by the emulation runs. The goal of the section is to understand the analysis reports of Vitis before utilizing them in the next section.  

#### Profile Summary report

After the emulation run completes, an profile_summary_hw_emu.csv file is generated in the `build` folder. Before viewing it in Vitis GUI, it must be converted into an appropriate format.

Open the generated profile summary report generated
```
cd ./build/
vitis_analyzer profile_summary_hw_emu.csv
```

  ![](../../images/module_01/lab_02_idct/HWProfile.PNG)

  This report provides data related to how the application runs. Notice that the report has four tabs at the bottom: **Top Operations**, **Kernels & Compute Units**, **Data Transfers**, and **OpenCL APIs**.

  Click through and inspect each of the tabs:

  * **Top Operations**: Shows all the major top operations of memory transfer between the host and kernel to global memory, and kernel execution. This allows you to identify throughput bottlenecks when transferring data. Efficient transfer of data to the kernel/host allows for faster execution times.

  * **Kernels & Compute Units**: Shows the number of times the kernel was executed. Includes the total, minimum, average, and maximum run times. If the design has multiple compute units, it will show each compute unit’s utilization. When accelerating an algorithm, the faster the kernel executes, the higher the throughput which can be achieved. It is best to optimize the kernel to be as fast as it can be with the data it requires.

  * **Data Transfers**: This tab has no bearing in software emulation as no actual data transfers are emulated across the host to the platform. In hardware emulation, this shows the throughput and bandwidth of the read/writes to the global memory that the host and kernel share.

  * **OpenCL APIs**: Shows all the OpenCL API command executions, how many time each was executed, and how long they take to execute.

3. Click on the **Kernels & Compute Units** tab of the **Profile Summary** report, locate and note the following numbers:

  - Kernel Total Time (ms):

This number will serve as reference point to compare against after optimization.    

#### HLS reports

The Vitis v++ compiler also generates **HLS Reports** for each kernel. **HLS Reports** explain the results of compiling the kernel into hardware. It contains many details (including clocking, resources or device utilization) about the performance and logic usage of the custom-generated hardware logic. These details provide many insights to guide the kernel optimization process.    

1. Locate the HLS reports:
```
cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/
find . -name "*_csynth.rpt"
```

2. Open the **./build/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt** file, scroll to the **Performance Estimates** section, locate the **Latency (clock cycles)**  summary table and note the following performance numbers:

  - Latency (min/max):
  - Interval (min/max):

![](../../images/module_01/lab_02_idct/LatencyKrnlIdctDataflow.PNG)


  * Note that the 3 sub-functions read, execute and write have roughly the same latency and that their sum total is equivalent to the total Interval reported in the **Summary** table.
  * This indicates that the three sub-functions are executing sequentially, hinting to an optimization opportunity.


#### Application Timeline report

In addition to the profile_summary_hw_emu.csv file, the emulation run also generates an timeline_trace_hw_emu.csv file in yhe `build` folder. Before viewing it in Vitis GUI, it must be converted into an appropriate format.

Open the generated profile summary report generated
```
vitis_analyzer timeline_trace_hw_emu.csv 
```

![](../../images/module_01/lab_02_idct/SWTimeline.PNG)

The **Application Timeline** collects and displays host and device events on a common timeline to help you understand and visualize the overall health and performance of your systems. These events include OpenCL API calls from the host code: when they happen and how long each of them takes.


### Summary  

In this lab, you learned:
* Important OpenCL API calls to communicate between the host and the FPGA
* The differences between the software and hardware emulation flows and how to run them
* How to read the various reports generated by Vitis


In the next lab you utilize these analysis capabilities to drive and measure code optimizations.

---------------------------------------

<p align="center"><b>
Start the next lab: <a href="lab_03_idct_optimization.md">Optimizing F1 applications</a>
</b></p>  
