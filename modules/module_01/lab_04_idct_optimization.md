# IDCT FPGA Kernel: Optimizations and Performance Analysis


  - [Optimizing the IDCT Kernel using Dataflow](#optimizing-the-idct-kernel-using-dataflow)
    - [Comparing FPGA Resource Usage](#comparing-fpga-resource-usage)
    - [Comparing Execution Times and Application Timeline](#comparing-execution-times-and-application-timeline)
      - [Kernel with Dataflow Optimization](#kernel-with-dataflow-optimization)
      - [Kernel without Dataflow Optimization](#kernel-without-dataflow-optimization)
  - [Kernel Loop Pipelining Using Various Initiation Intervals (IIs)](#kernel-loop-pipelining-using-various-initiation-intervals-iis)
    - [Initiation Interval versus FPGA Resource Usage](#initiation-interval-versus-fpga-resource-usage)
    - [Initiation Interval versus Acceleration](#initiation-interval-versus-acceleration)
      - [Performance with II=8](#performance-with-ii8)
      - [Performance with II=4](#performance-with-ii4)
      - [Performance with II=2](#performance-with-ii2)
  - [Summary](#summary)

This lab builds on top of previous labs which gave an overview of the Vitis development environment and explained the various performance analysis capabilities provided by the tool using different reports. In this lab, you will utilize these analysis capabilities to drive and measure performance improvements enabled by code optimizations. This lab illustrates the dataflow optimization and loop pipelining variations effects on overall performance.

>**NOTE**: Although the entire lab is performed on an F1 instance, only the steps that involve use hardware/FPGA card need to run on F1 instance. All the interactive development, profiling and optimization steps would normally be performed on-premise or on a cost-effective AWS EC2 instance such as C4. However, to avoid switching from C4 to F1 instance during this lab, all the steps are performed on the F1 instance.

If you have closed the terminal window at the end of the previous lab, open a new one, run setup script and go back to the project folder:

1.  Source the Vitis environment  

    ```bash
    source $AWS_FPGA_REPO_DIR/vitis_setup.sh
    ```
    
2. Go to the project folder
    
    ```bash 
    export LAB_WORK_DIR=/home/centos/src/project_data/
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct
    ```
   
## Optimizing the IDCT Kernel using Dataflow

Carry out a simple experiment that will illustrate the effects and power of dataflow optimization. The FPGA binary built and used for experiments is built to have four different kernels. There are minor differences among them and they are created to illustrate different performance optimizations. In this experiment, you will focus on two kernels namely **krnl_idct** and **krnl_idct_noflow** and compare them.
 
### Comparing FPGA Resource Usage

1. Open kernel source files and compare

    ```bash
    vim src/krnl_idct.cpp
    vim src/krnl_idct_noflow.cpp
    ``` 
   
   These files contain description for both of these kernels. They are exactly identical kernel with different names and one major difference: Dataflow optimization is enabled for **krnl_idct** in **krnl_idct.cpp** whereas it is not used for other kernel **krnl_idct_noflow** in **knrl_idct_noflow.cpp**. You can see this by going to a label called "DATAFLOW_PRAGMA" which is placed as marker near line 358 in both the files. An HLS Pragma is applied here, it is enabled for "krnl_idct" and disabled for other by commenting out. The application of this pragma makes functions in the region execute concurrently and create a function pipeline which overlaps compute in different functions as compared to full sequential execution. The functions in this region are connected through FIFOs also called hls::streams, it is one of the recommended style for the functions used in dataflow region, given that the data is produced or consumed in order at every function boundary.
      
1. Look at the synthesis report for latency and II to compare expected performance, proceed as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct
    vim src/host.cpp
    ```
   
    Go to label **CREATE_KERNEL** near line 226 and make sure the kernel name string is "krnl_idct" and not anything else. Run hardware emulation as follows:
    
    ```bash
    make run TARGET=hw_emu
    ```
   
   The hardware emulation will build all variations of kernels and use **krnl_idct** for emulation. Now open Vitis HLS reports(or use link summary) as follows and see different metrics:
   
    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
    vim ./build_hw_emu/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
    ```
   
   From performance estimate section under Latency, note down :
   - Latency
   - Interval
   - Pipeline
   
   Under Utilization section note different resources utilized:
   - Block RAMs
   - FFs
   - DSPs
   - LUTs
   
   For the second kernel note the same metrics by opening the following report:
   
   ```bash
   vim ./build_hw_emu/reports/krnl_idct_noflow.hw_emu/hls_reports/krnl_idct_noflow_csynth.rpt
   ```
   
   By comparison you should notice that the performance of the kernel with dataflow optimization is almost 3 times better in terms of Latency and II and the resource utilization is pretty much still the same.
   
### Comparing Execution Times and Application Timeline

#### Kernel with Dataflow Optimization
 
You can see the compute unit execution time by looking at the profile summary reports. You have already run hardware emulation for kernel "kernel_idct" which uses dataflow optimization. Now, open the profile summary report using vitis_analyzer as follows:

```
vitis_analyzer ./build_hw_emu/xclbin.run_summary

```
   * Once the vitis_analyzer is open, select **Profile Summary** from the left pane, then select **Kernel and Compute Units** section. It gives execution estimates for kernel and compute unit. Compute Unit Utilization section reports min/max/avg estimated execution times note it down.
   * Next, open the application time line by selecting "Application Timeline" from left hand panel and it opens up in right side main window.
   * Zoom in close to first transaction happening on device time line on read/write interfaces, snapshot for this is shown in the figure below and focus on the part marked by yellow box:
   
![](images/module_01/lab_04_idct/hwEmuComputMemTxOverLap.PNG)

You can observe from this timeline that:

- There is overlapping activity at the read and write interfaces for compute unit essentially meaning things are happening concurrently(read_block/execute/write_block functions running concurrently).
    
- The amount of overlap seems marginal because a very small data size has been intentionally chosen for emulation. Overlapping is further reduced when you go to the actual hardware or system run where you can use a larger data size.

#### Kernel without Dataflow Optimization
       
1. Open `host.cpp` and make changes so that hardware emulation will use the second kernel namely "krnl_idct_noflow" and then run the emulation again:

    ```bash
    vim src/host.cpp
    ```  

    Go to label "CREATE_KERNEL" near line 226 and change the kernel name string to "krnl_idct_noflow" and run hardware emulation:

    ```bash
    make run TARGET=hw_emu
    ```
   
   Open the profile summary again and note the compute unit execution time and compare it with kernel with dataflow optimization. It will be 2-3x worse. Ideally, it should be 3x since the II was three times more but due to memory bandwidth effects, it can come down.
    
2.  Open the application time line trace and check the compute unit activity:

     ```bash
    vitis_analyzer ./build_hw_emu/xclbin.run_summary
    ```  
    
    It will show something similar to the figure below. Look at the section highlighted by the yellow box and compare it to kernel execution with dataflow optimization, you will easily find out that all activity happened sequentially here there is no overlap at all, essentially the time line can be interpreted as:

    * sequential read: kernel first executes read_block function marked by data read activity on **coeffs** and **inBlocks** interfaces
    
    * process: kernel processes input data blank period before write
    
    * write activity: lastly write_block execution marked by data write activity on **outBlocks** interface
    
    ![](images/module_01/lab_04_idct/hwEmuComputMemTxNoOverLap.PNG)
    
## Kernel Loop Pipelining Using Various Initiation Intervals (IIs)  

In this section, you will experiment with the Initiation Interval for loop pipelining. To make this experiment easier, different kernels that are almost identical except with different names and IIs have been included. To understand the kernel code structure open:

```bash
vim src/krnl_idct.cpp
```
  
Go to label "FUNCTION_PIPELINE" near line 37. You will see four different function calls within the dataflow optimization region:

- read_blocks
- read_blocks
- execute
- write_blocks

The read and write blocks simply reads data and writes data from memory interfaces and streams it to execute function which calls IDCT function to perform the core compute. The read and write functions can be pipelined with desired II with overall performance dictated by "execute" function II. It is a functional pipeline where all of these functions will be constructed as chain of independent hardware modules. The overall performance will be defined by any block that has the lowest performance which essentially means largest II. Since execute block carries out almost all compute so II variation on this block will show significant overall performance and resource utilization variations.

Now, you will do actual system runs no emulations using a pre-built FPGA binary file. You can also build a binary using the provided makefile.

To see how pipeline pragmas with different II are applied to kernels, open different kernel source files and compare II constraints placed near label "PIPELINE_PRAGMA:" in each file around line 297, you will see II as follows:

- krnl_idct      : II=2
- krnl_idct_med  : II=4
- krnl_idct_slow : II=8

```bash
    vim src/krnl_idct.cpp
    vim src/krnl_idct_med.cpp
    vim src/krnl_idct_slow.cpp     
```

>**NOTE**: II = 1 cannot be achieved because the device DDR memory width is 512 bits(64 Bytes) and IDCT compute if done is single cycle requires (64*size(short)*8=1024) 1024 bits (128 bytes) per cycle. In terms of how this fact manifest in our model can be seen by going to "execute" function to find that it will require 2 reads or writes on FIFO I/O interfaces in single cycle which is not possible.

### Initiation Interval versus FPGA Resource Usage

You will not build an actual FGPA binary here. To get the look and feel of II effects on resources, you can use results from hardware emulation runs since it also performs kernel synthesis. Since you have ran hw_emulation in a previous experiment, you can go ahead and build a folder and look at the Vitis HLS reports to find out Initiation Interval and Latency for these kernels as well as resource utilization. The resource utilization will have a trend showing decrease in utilization with increase in II.

To do so, open and compare synthesis report to note down IIs/Latencies and resource utilization ( you can also use link summary as discussed in previous lab):
 
    ```bash
    vim ./build_hw_emu/reports/krnl_idct_med.hw_emu/hls_reports/krnl_idct_med_csynth.rpt
    vim ./build_hw_emu/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
    vim ./build_hw_emu/reports/krnl_idct_slow.hw_emu/hls_reports/krnl_idct_slow_csynth.rpt
    ```

### Initiation Interval versus Acceleration

It is estimated in previous labs that kernel with II=4 may be able to run at maximum throughput but to show the effects of II on overall performance, you have to start with II=8 and then run with II=4 and finally with II=2. The results reported here and on the machine you are using may vary depending on the memory subsystem performance specially PCIe bandwidth available at run time to the application.

#### Performance with II=8
 
1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
    vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226. Ensure that the kernel name string is "krnl_idct_slow" and build host application again as follows:
   
    ```bash
    make compile_host TARGET=hw
    ```    
   
2. Run host application as follows:

	```bash
	./build/host.exe ./xclbin/krnl_idct.hw.awsxclbin $((1024*128)) 32 1
	```
 
	You will see an output like this, which shows FGPA acceleration by a factor of 7x:
	
    ```
    Execution Finished
    =====================================================================
    ------ All Task finished !
    ------ Done with CPU and FPGA based IDCTs
    ------ Runs complete validating results
    CPU Time:        1.89137 s ( 1894.34ms )
    CPU Throughput:  270.704 MB/s
    FPGA Time:       0.266215 s (166.936 ms )
    FPGA Throughput: 1923.26 MB/s
    ------ TEST PASSED ------
    =====================================================================
    FPGA accelerations ( CPU Exec. Time / FPGA Exec. Time): 7.10467
    =====================================================================
    ```   
#### Performance with II=4

1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
   cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226 and make sure the kernel name string is "krnl_idct_med". and build host application again as follows:
   
   ```bash
   make compile_host TARGET=hw
   ```    
1. Run the host application as follows:

	```bash
	./build/host.exe ./xclbin/krnl_idct.hw.awsxclbin $((1024*128)) 32 1
	```
 
	You will see an output like this, which shows FGPA acceleration by a factor of 9x:
	
    ```
    Execution Finished
    =====================================================================
    ------ All Task finished !
    ------ Done with CPU and FPGA based IDCTs
    ------ Runs complete validating results
    CPU Time:        1.9116 s ( 1894.34ms )
    CPU Throughput:  267.838 MB/s
    FPGA Time:       0.199102 s (166.936 ms )
    FPGA Throughput: 2571.55 MB/s
    ------ TEST PASSED ------
    =====================================================================
    FPGA accelerations ( CPU Exec. Time / FPGA Exec. Time): 9.60113
    =====================================================================
    ``` 
#### Performance with II=2

1. Open and modify host code to run "krnl_idct" as follows:

    ```bash
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226 and make sure the kernel name string is "krnl_idct". and build host application as follows:
   
   ```bash
   make compile_host TARGET=hw
   ```    
2. Run the host application on the AWS F1 instance. Perform the Xilinx runtime setup and launch the application as follows:

    ```bash
    source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh
    ./build/host.exe ./xclbin/krnl_idct.hw.awsxclbin $((1024*128)) 32 1
    ```
	
    You will see an output like this, which shows FGPA acceleration by a factor of 11x:
   
    ```
    Execution Finished
    =====================================================================
    ------ All Task finished !
    ------ Done with CPU and FPGA based IDCTs
    ------ Runs complete validating results
    CPU Time:        1.89434 s ( 1894.34ms )
    CPU Throughput:  270.279 MB/s
    FPGA Time:       0.166936 s (166.936 ms )
    FPGA Throughput: 3067.04 MB/s
    ------ TEST PASSED ------
    =====================================================================
    FPGA accelerations ( CPU Exec. Time / FPGA Exec. Time): 11.3477
    =====================================================================
    ```   
>**NOTE**: In last three experiments, with II going from eight to four and then two, the overall performance should be going up by 2x every time but it was not the case. The reason for this is that performance is not only defined by kernel or compute performance but it also depends on the memory bandwidth available to CU and host at different times and sometime it dictates performance. However, it is clear from the last experiment that II variations have significant effect on performance and hardware resource consumption.

## Summary  

In this lab, you learned the following:

- Using dataflow optimization to improve performance
- Effect of the II variation during loop pipelining on performance and resource utilization

---------------------------------------

<p align="center"><b>
Finish this module: <a href="lab_05_sw_pipelining_wrapUp.md">Software Optimizations and Wrap Up</a>
</b></p>  
