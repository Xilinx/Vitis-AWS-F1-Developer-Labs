## IDCT FPGA Kernel: Optimizations and Performance Analysis

This lab builds on top of previous labs which gave an overview of the Vitis development environment and explained the various performance analysis capabilities provided by the tool using different reports. In this lab you will utilize these analysis capabilities to drive and measure performance improvements enabled by code optimizations. This lab illustrates the dataflow optimization and loop pipelining variations effects on overall performance. 

Please note that although the entire lab is performed on an F1 instance, only the steps that involve use hardware/FPGA card need to run on F1 instance. All the interactive development, profiling and optimization steps would normally be performed on-premise or on a cost-effective AWS EC2 instance such as C4. However, to avoid switching from C4 to F1 instance during this lab, all the steps are performed on the F1 instance.

If you have closed the terminal window at the end of the previous lab, open a new one, run setup script and go back to the project folder:

1.  Source the Vitis environment  

    ```bash
    source $AWS_FPGA_REPO_DIR/vitis_setup.sh
    ```
    
1. Go to project folder
    
    ```bash 
    export LAB_WORK_DIR=/home/centos/src/project_data/
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct
    ```
   
### Optimizing the IDCT kernel using Dataflow
We will carry out a simple experiment that will illustrate the effects and power of dataflow optimization. The FPGA binary built and used for experiments is built to have 4 different kernels. There are minor differences among them and they are created to illustrate different performance optimizations. In this experiment we will focus on two kernels namely **krnl_idct** and **krnl_idct_noflow** and compare them.
 
#### Comparing FPGA Resource Usage
1. Open kernel source files and compare

    ```bash
    vim src/krnl_idct.cpp
    vim src/krnl_idct_noflow.cpp
    ``` 
   
   these files contain description for both of these kernels, they are exactly identical kernel with different names and one major difference. Dataflow optimization is enabled for **krnl_idct** in **krnl_idct.cpp** whereas it is not used for other kernel **krnl_idct_noflow** in **knrl_idct_noflow.cpp**. You can see this by going to a label called "DATAFLOW_PRAGMA" which is placed as marker near line 358 in both the files. An HLS Pragma is applied here, it is enabled for "krnl_idct" and disabled for other by commenting out. The application of this pragma makes functions in the region execute concurrently and create a function pipeline which overlaps compute in different functions as compared to full sequential execution. The functions in this region are connected through FIFOs also called hls::streams, it is one of the recommended style for the functions used in dataflow region, given that the data is produced or consumed in order at every function boundary.
      
1. Now we will look at the synthesis report for latency and II to compare expected performance, proceed as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct
    vim src/host.cpp
    ```
   
    go to label **CREATE_KERNEL** near line 226 and make sure the kernel name string is "krnl_idct" and not anything else. Now run hardware emulation as follows:
    
    ```bash
    make run TARGET=hw_emu
    ```
   
   The hardware emulation will build all variations of kernels and use **krnl_idct** for emulation. Now open Vitis_hls reports(or use link summary) as follows and see different metrics:
   
    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
    vim ./build_hw_emu/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
    ```
   
   From performance estimate section under Latency, note down :
   - Latency 
   - Interval
   - Pipeline 
   
   Under Utilization section note different resources utilized:
   - BRAMs
   - FFs
   - DSPs
   - LUTs
   
   For the second kernel note the same metrics by opening the following report:
   
   ```bash
   vim ./build_hw_emu/reports/krnl_idct_noflow.hw_emu/hls_reports/krnl_idct_noflow_csynth.rpt 
   ```
   
   By comparison you should notice that the performance of the kernel with dataflow optimization is almost 3 times better in terms of Latency and II and the resource utilization is pretty much still the same.
   
#### Comparing Execution Times and Application Timeline   

##### Kernel with Dataflow Optimization   
 
1. The next thing we can see is the compute unit execution time by looking at the profile summary reports. We have already ran hardware emulation for kernel "kernel_idct" which uses dataflow optimization, now open the profile summary report using vitis_analyzer as follows:

    ```
    vitis_analyzer ./build_hw_emu/xclbin.run_summary
   
    ```
   * Once vitis_analyzer is open select **Profile Summary** from left side and then select **Kernel and Compute Units** section. It gives execution estimates for kernel and compute unit. Compute Unit Utilization section reports min/max/avg estimated execution times note it down.
   * Next open the application time line by selecting "Application Timeline" from left hand panel and it opens up in right side main window.
   * Zoom in close to first transaction happening on device time line on read/write interfaces, snapshot for this is shown in the figure below and focus on the part marked by yellow box:
   
    ![](../../images/module_01/lab_04_idct/hwEmuComputMemTxOverLap.PNG) 
    
    We can observe from this timeline that:
    * There is overlapping activity at the read and write interfaces for compute unit essentially meaning things are happening concurrently(read_block/execute/write_block functions running concurrently).
    
    * The amount of overlap seems marginal because we have intentionally chosen very small data size for emulation, the situation will be much better when we go to actual hardware or system run where we can use large data size.

##### Kernel without Dataflow Optimization   
       
1. Now open host.cpp and make changes so that hardware emulation will use second kernel namely "krnl_idct_noflow" and then run the emulation again:

    ```bash
    vim src/host.cpp
    ```  

    Go to label "CREATE_KERNEL" near line 226 and change the kernel name string to "krnl_idct_noflow" and run hardware emulation:

    ```bash
    make run TARGET=hw_emu
    ```
   
   Now again open the profile summary and note down compute unit execution time and compare it with kernel with dataflow optimization it will be 2-3x worse. Idealy it should be 3x since the II was 3 times more but due to memory bandwidth effects it can come down.
    
1.  Now open application time line trace and check the compute unit activity:

     ```bash
    vitis_analyzer ./build_hw_emu/xclbin.run_summary
    ```  
    
     it will show something similar to the figure below, look at the section highlighted by yellow box and compare it to kernel execution with dataflow optimization, you will easily find out that all activity happened sequentially here there is no overlap at all, essentially the time line can be interpreted as:

    * sequential read: kernel first executes read_block function marked by data read activity on **coeffs** and **inBlocks** interfaces
    
    * process: kernel processes input data blank period before write
    
    * write activity: lastly write_block execution marked by data write activity on **outBlocks** interface
    
    ![](../../images/module_01/lab_04_idct/hwEmuComputMemTxNoOverLap.PNG)
    
### Kernel Loop Pipelining Using Various Initiation Intervals(IIs)  

In this section we will experiment with Initiation Interval for loop pipelining. To make this experiment easier we have included different kernels that are almost identical except with different names and IIs. To understand the kernel code structure open: 

```bash
vim src/krnl_idct.cpp
```
  
Go to label "FUNCTION_PIPELINE" near line 370 here you will see four different function calls within the dataflow optimization region:

- read_blocks
- read_blocks
- execute
- write_blocks 

read and write blocks simply reads data and write data from memory interfaces and stream it to execute function which calls IDCT function to perform the core compute. The read and write functions can be pipelined with desired II with overall performance dictated by "execute" function II. It is a functional pipeline where all of these functions will be constructed as chain of independent hardware modules. The overall performance will be defined by any block that has the lowest performance which essentially means largest II. Since execute block carries out almost all compute so II variation on this block will show significant overall performance and resource utilization variations. 

Now we will do actual system runs no emulations using a pre-built FPGA binary file. The user can also build a binary using the provided makefile.

To see how pipeline pragmas with different II are applied to kernels, open different kernel source files and compare II constraints placed near label "PIPELINE_PRAGMA:" in each file around line 297, you will see II as follows:

- krnl_idct      : II=2
- krnl_idct_med  : II=4
- krnl_idct_slow : II=8 

```bash
    vim src/krnl_idct.cpp
    vim src/krnl_idct_med.cpp
    vim src/krnl_idct_slow.cpp     
```

**NOTE** : _II = 1 cannot be achieved because the device DDR memory width is 512 bits(64 Bytes) and IDCT compute if done is single cycle requires (64*size(short)*8=1024) 1024 bits (128 bytes) per cycle. In terms of how this fact manifest in our model can be seen by going to "execute" function to find that it will require 2 reads or writes on FIFO I/O interfaces in single cycle which is not possible._

#### Initiation Interval vs. FPGA Resource Usage

We won't built actual FGPA binary here so to get the look and feel of II effects on resources we can use results from hardware emulation runs since it also performs kernel synthesis. Since we have ran hw_emulation in previous experiment you can go to build folder and have a look at Vitis_hls reports to find out Initiation Interval and Latency for these kernels as well as resource utilization. The resource utilization will have a trend showing decrease in utilization with increase in II.

1. Open and compare synthesis report to note down IIs/Latencies and resource utilization ( you can also use link summary as discussed in previous lab):
 
    ```bash
    vim ./build_hw_emu/reports/krnl_idct_med.hw_emu/hls_reports/krnl_idct_med_csynth.rpt
    vim ./build_hw_emu/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
    vim ./build_hw_emu/reports/krnl_idct_slow.hw_emu/hls_reports/krnl_idct_slow_csynth.rpt
    ```

#### Initiation Interval vs. Acceleration

##### Performance with II=2   
1. Open and modify host code to run "krnl_idct" as follows:

    ```bash
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226 and make sure the kernel name string is "krnl_idct". and build host application as follows:
   
   ```bash
   make compile_host TARGET=hw
   ```    
1. Now we will run host application on Amazon F1 instance, please perform the Xilinx runtime setup and launch the application as follows:

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
##### Performance with II=4   

1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
   cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226 and make sure the kernel name string is "krnl_idct_med". and build host application again as follows:
   
   ```bash
   make compile_host TARGET=hw
   ```    
1. Now we will run host application as follows:

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

 ##### Performance with II=8
 
 1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
    vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line 226 and make sure the kernel name string is "krnl_idct_slow". and build host application again as follows:
   
    ```bash
    make compile_host TARGET=hw
    ```    
   
1. Now we will run host application as follows:

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
   
**NOTE**: In last three experiments, with II going from 2 to 4 and then 8, the performance should be going down by 2x every time but it was not the case, reason for this is that performance is not only defined by kernel or compute performance but it also depends on memory bandwidth available to CU and host at different times and sometime it dictates performance. But one thing should be clear from the last experiment that II variations have significant effect on performance and hardware resource consumption.   


### Summary  

In this lab, you learned:
* How to use dataflow optimization to improve performance
* How II variation during loop pipelining effect performance and resource utilization
---------------------------------------

<p align="center"><b>
Finish this module: <a href="lab_05_sw_pipelining_wrapUp.md">Software Optimizations and Wrap Up</a>
</b></p>  
