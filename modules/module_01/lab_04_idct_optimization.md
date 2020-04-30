## IDCT Kernel Hardware Optimizations and Performance Analysis

This lab builds on top of previous labs which gave an overview of the Vitis development environment and explained the various performance analysis capabilities provided by the tool. In this lab you will utilize these analysis capabilities to drive and measure code optimizations. This lab illustrates the DATAFLOW optimization and loop pipelining variations and their effect on overall performance. 

Please note that although the entire lab is performed on an F1 instance, only the steps that involve hardware runs need to be to be run on F1 instance. All the interactive development, profiling and optimization steps would normally be performed on-premise or on a cost-effective AWS EC2 instance such as C4. However, to avoid switching from C4 to F1 instances during this lab, all the steps are performed on the F1 instance.

If you have closed the terminal window at the end of the previous lab, open a new one, run setup script and go back to the project folder:
1.  Source the Vitis environment  

    ```bash
    cd $AWS_FPGA_REPO_DIR/
    source vitis_setup.sh
    ```
1. Go to project folder
    
    ```bash 
    export LAB_WORK_DIR=/home/centos/src/project_data/
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct
    ```
   
### Optimizing the IDCT kernel using Dataflow
We will carry out a simple experiment that illustrates the effect and power of dataflow optimization. The FPGA binary that will be built for experiment is built to have 4 different kernel, there are minor difference between them and they are created to illustrate different performance optimizations. In this experiment we will focus on two kernels namely **krnl_idct** and **krnl_idct_noflow** and compare them. 

1. Open kernel source files and compare

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/idct/src 
   open krnl_idct.cpp
   open krnl_idct_noflow.cpp
    ``` 
   
   these files contain description for both of these kernels, they are exactly identical kernel with different names and one major difference. Dataflow optimization is enabled for **krnl_idct** in **krnl_idct.cpp** whereas it is not used for other kernel **krnl_idct_noflow** in **knrl_idct_noflow.cpp**. You can see this by going to a label called "DATAFLOW_PRAGMA" which is placed as marker near line 354 in both the files. The application of this pragma makes functions in the region execute concurrently and create a function pipeline which overlap calculations in different functions as compared to full sequential execution. The functions in this region are connected through FIFOs generally called hls::streams, it is one of the recommended style for the functions used in Dataflow region, given that the data is produced or consumed in order at every function boundary.
      
1. Now we will look at the synthesis report for latency and II to compare expected performance, proceed as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/src
   vim host.cpp
    ```
   
    go to label **CREATE_KERNEL** near line no.229 and make sure the kernel name string is "krnl_idct" and not anything else. Now run hardware emulation as follows:
    
    ```bash
   make run TARGET=hw_emu
    ```
   
   The hardware emulation will build all the kernels and use **krnl_idct** for emulation. Now open Vitis_hls reports as follows and see different metrics as follows:
   
    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
   vim ./build/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
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
   vim ./build/reports/krnl_idct_noflow.hw_emu/hls_reports/krnl_idct_noflow_csynth.rpt 
   ```
   
   By comparison you should notice that the performance of the kernel with Dataflow optimization is almost 3 times better in terms of Latency and II and the resource utilization is pretty much still the same.
   
1. The next thing we can see is the compute unit execution time by looking at the profile summary reports. We have already ran hardware emulation for kernel "kernel_idct" which uses Dataflow optimization, now open the profile summary report using vitis_analyzer as follows:

    ```bash
   vitis_analyzer build/profile_summary_hw_emu.csv 
    ```  
   
    Once vitis_analyzer is open select Profile Summary from left side and then select "Kernel and Compute Units" section. It gives execution estimates for kernel and compute unit. Compute Unit Utilization section reports min/max/avg estimated execution times note it down. Now open the application time line as follows:
   
   ```bash
    vitis_analyzer build/timeline_trace_hw_emu.csv
   ```  
   
   and zoom close to first transaction happening on device time line on read/write interfaces, snapshot is shown in the figure below and focus on the part marked by yellow box:
   
   ![](../../images/module_01/lab_04_idct/hwEmuComputMemTxOverLap.PNG) 
   
    What we can observe from this timeline is that there is overlapping activity at the read and write interfaces for compute unit essentially meaning things are happening concurrently. The amount of overlap seems marginal because we have intentionally chosen very small data size for emulation, the situation will be much better when we go to actual hardware or system run when we can use large data size. In next section we will compare this waveform with non-dataflow kernel.
       
    Now open host.cpp and make changes so that hardware emulation will use second kernel namely "krnl_idct_noflow" and then run the emulation:
   
   ```bash
   vim src/host.cpp
   ```  
  
    Go to label "CREATE_KERNEL" near line no.228 and change the kernel name string to "krnl_idct_noflow" and run hardware emulation:
   
    ```bash
    make run TARGET=hw_emu
    ```
   
   Now again open the profile summary and note down compute unit execution time and compare it with kernel with no Dataflow optimization it will be 2-3x worse. Idealy it should be 3x since the II was 3 times more but due to memory bandwidth effects it can come down.
    
1.  Now open application time line trace and check the compute unit activity it will show something similar to the figure below, look at the section highlighted by yellow box and compare it to kernel execution with Dataflow optimization, you will easily find out that all activity happened sequentially here there is no overlap at all, essentially the time line can be interpreted as, sequential read, process and write activity:

   ```bash
    vitis_analyzer build/timeline_trace_hw_emu.csv
   ```  
   
   ![](../../images/module_01/lab_04_idct/hwEmuComputMemTxNoOverLap.PNG)

### Kernel Loop Pipelining using various Initiation Intervals(IIs)  
We will experiment with Initiation Interval for loop pipelining. To this experiment easier we have included different kernels that are identical except with different names and IIs. To understand the kernel code structure open: 
```bash
vim src/krnl_idct.cpp
```  
Go to label "FUNCTION_PIPELINE" near line no.364 here you will see four different function calls within the Dataflow optimization region:

- read_blocks
- read_blocks
- execute
- write_blocks 

read and write blocks simply reads data and write data from memory interfaces and streams it to execute which calls IDCT function to perform the core compute. The read and write functions can be pipelined with desired II with overall performance defined by "execute" function II. It is a functional pipeline where all of these function will be constructed as independent hardware modules so the overall performance will be defined by any block that has the lowest performance which essentially mean largest II. Since execute block carries out almost all compute so II variation on this block will show significant overall performance and resource utilization variations. 
During this experiment we will do actual system runs instead of any emulation modes. For these experiments a pre-built FPGA binary file is provided. The user can also build a binary, procedure for building this file will described in a latter section.

To see how pipeline pragmas with different II are applied to different kernels, open different kernel source files and compare II constraints placed near label "COMPUTE_PIPELINE_PRAGMA:" in each file around line no.296, you will see II as follows:

- krnl_idct      : II=2
- krnl_idct_med  : II=4
- krnl_idct_slow : II=8 

```bash
    vim src/krnl_idct.cpp
    vim src/krnl_idct_med.cpp
    vim src/krnl_idct_slow.cpp     
```
Since we have ran hw_emulation in previous experiment you can go to build folder and have a look at Vitis_hls reports to find out Initiation Interval and Latency for these kernels as well as resource utilization. The resource utilization will have a trend showing decrease in utilization with increase in II.

1. Open and compare synthesis report to note down IIs/Latencies and resource utilization:
 
    ```bash
    vim ./build/reports/krnl_idct_med.hw_emu/hls_reports/krnl_idct_med_csynth.rpt
    vim ./build/reports/krnl_idct.hw_emu/hls_reports/krnl_idct_csynth.rpt
    vim ./build/reports/krnl_idct_slow.hw_emu/hls_reports/krnl_idct_slow_csynth.rpt
    ```
1. Open and modify host code to run "krnl_idct" as follows:

    ```bash
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line no.228 and make sure the kernel name string is "krnl_idct". and build host application as follows:
   
   ```bash
   make compile_host
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
1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
   cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
   vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line no.228 and make sure the kernel name string is "krnl_idct_med". and build host application again as follows:
   
   ```bash
   make compile_host
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
 1. Open and modify host code to run "krnl_idct_med" as follows:

    ```bash
    cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_01/
    vim src/host.cpp
    ```
   
   Go to label "CREATE_KERNEL" near line no.228 and make sure the kernel name string is "krnl_idct_slow". and build host application again as follows:
   
   ```bash
   make compile_host
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
**NOTE**: in the last three experiments, with II going from 2 to , 4 and then 8, we should be performance going down by 2x every time but it was not the case, reason for this is that performance is not only defined by kernel or compute performance but it also depends on memory bandwidth available to CU and host at different times and sometime dictates performance. But one thing should be clear from the last experiment that II variations have significant effect on performance and hardware resource consumption.   


### Summary  

In this lab, you learned:
* How to use dataflow optimization to improve performance
* How II variation during loop pipelining effect performance and resource utilization on FPGA
---------------------------------------

<p align="center"><b>
Finish this module: <a href="lab_05_sw_pipelining_wrapUp.md">Software Optimizations and Wrap Up</a>
</b></p>  
