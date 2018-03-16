## Using the SDAccel GUI to optimize F1 applications

This lab builds on the previous one ([Introduction to the SDAccel development environment](lab_02_idct.md)) which gave an overview of the SDAccel development environment and explained the various performance analysis capabilities provided by the tool. In this lab you will utilize these analysis capabilities to drive and measure code optimizations. This lab illustrates the DATAFLOW optimization for the kernel and software pipelining for the host application.

Please note that although the entire lab is performed on an F1 instance, only the final step of this lab really needs to be run on F1. All the interactive development, profiling and optimization steps would normally be performed on-premise or on a cost-effective AWS EC2 instance such as C4. However, to avoid switching from C4 to F1 instances during this lab, all the steps are performed on the F1 instance.

If you have closed the SDAccel GUI at the end of the previous lab, reopen it and reload the workspace containing the IDCT project:
    ```bash
    cd ~/SDAccel-AWS-F1-Developer-Labs/modules/module_01/idct
    sdx -workspace ./workspace
    ```

### Optimizating the IDCT kernel

Looking at the **HLS Report**, we identified that the read, execute and write functions of the **krnl_idct_dataflow** function have roughly the same latency and are executing sequentially. We still start by focusing on this performance aspect.

1. Open **krnl_idct.cpp** file (from the **Project Explorer** window).  

1. Using the **Outline** viewer, navigate to the **krnl_idct_dataflow** function.

1. Observe that the three functions are communicating using `hls::streams` objects. These objects model a FIFO-based communication scheme. This is the recommended coding style which should be used whenever possible to exhibit streaming behavior and allow DATAFLOW optimization.

1. Enable the DATAFLOW optimization by uncommenting the **#pragma DATAFLOW** present in the **krnl_idct_dataflow** function (line 322). 
	
    - The DATAFLOW optimization allows each of the subsequent functions to execute as independent processes. 
    - This results in overlapping and pipelined execution of the read, execute and write functions instead of sequential execution.
    - The FIFO channels between the different processes do not need to buffer the complete dataset anymore but can directly stream the data to the next block. 

1. Comment out the three **#pragma HLS stream** statements on lines 327, 328 and 329.

1. Save the file (**Ctrl-S**).

1. Rebuild the **Emulation-HW** configuration of the project by clicking the ![](../../images/module_01/lab_02_idct/BuildButton.PNG) button.
	- The build process for hardware emulation takes a couple of minutes to run.

1. Open the new **HLS Report** (in the **Reports** view, open the**Emulation-HW** folder and expand the **binary_container_1** folder).

1. Compare the new latency numbers reported in the **Performance Estimates** section with the previous numbers and you will note considerable improvement based on the DATAFLOW optimization.  
    - Latency (min/max):
    - Interval (min/max):

1. Run the **Emulation-HW** flow by clicking the run button, ![](../../images/module_01/lab_02_idct/RunButton.PNG). 

1. After the run finishes with the `RUN COMPLETE` message, open the new **Profile Summary** for the **Emulation-HW** run and select the **Kernels & Compute Units** tab.  

1. Compare the **Kernel Total Time (ms)** with the results from the unoptimized run.


### Optimizating the host code

1. Open **idct.cpp** file (from the **Project Explorer** window).  

1. Using the **Outline** viewer, navigate to the **runFPGA** function.

	For each block of 8x8 values, the **runFPGA** function writes data to the FPGA, runs the kernel, and reads results back. 
	
	Communication with the FPGA is handled by OpenCL API calls made within the cu.write, cu.run and cu.read functions.
	- **clEnqueueMigrateMemObjects** schedules the transfer of data to or from the FPGA.
	- **clEnqueueTask** schedules the executing of the kernel.

	These OpenCL functions use events to signal their completion and synchronize execution.
	
1. Open the **Application Timeline** of the **Emulation-HW** run.  

	The green segments at the bottom indicate when the IDCT kernel is running.

1. Notice that there are gaps between each of the green segments.

1. Zoom in by performing a **Left mouse drag** across one of these gaps to get a more detailed view.  

    ![](../../images/module_01/lab_02_idct/ZoomApplicationTimeline.PNG)

    - The two green segments correspond to two consecutive invocations of the IDCT kernel. 
    - The gap between the two segments is indicative of idle time between these two invocations. 
    - The **Data Transfer** section of the timeline shows that **Read** and **Write** operations are happening when the kernel is idle. 
    - The **Read** operation is to retrieve the results from the execution which just finished and the **Write** operation is to send inputs for the next execution. 
    - This represents a sequential execution flow of each iteration.  
    
1. Close the **Application Timeline**.    
	
1. In the **idct.cpp** file, go to the **oclDct::write** function.

	- Observe that on line 353, the function synchronizes on the **outEvVec** event through a call to **clWaitForEvents**. 
	- This event is generated by the completion of the **clEnqueueMigrateMemObjects** call in the **oclDct::read** function (line 429).
	- Effectively the next execution of the **oclDct::write** function is gated by the completion of the previous **oclDct::read** function, resulting in the sequential behavior observed in the **Application Timeline**. 
	
	
1. Use the **Outline** viewer to locate the definition of the **NUM_SCHED** macro in the **idct.cpp** file.
	
	- This macro defines the depth of the event queue.
	- The value of **1** explains the observed behavior: new tasks (write, run, read) are only enqueued when the previous has completed effectively synchronizing each loop iteration. 
	- By increasing the value of the **NUM_SCHED** macro, we increase the depth of the event queue and enable more blocks to be enqueued for processing. This will result in the write, run and read tasks to overlap and allow the kernel to execute continuously. 
	- This technique is called **software pipelining**. 
	
1. Modify line 213 to increase the value of **NUM_SCHED** to 6 as follows:
	```C
	#define NUM_SCHED 6
	```
1. Save the file (**Ctrl-S**) and rerun hardware emulation by clicking the run button ![](../../images/module_01/lab_02_idct/RunButton.PNG). 

    - Since only the **idct.cpp** file was change, the incremental makefile rebuilds only the host code before running emulation.
    - This results in a much faster iteration loop since it is usually the compilation of the kernel to hardware which takes the most time.
    
1. Once completed, reopen the **Application Timeline** and observe how **software pipelining** enables overlapping of data transfers and kernel exectution.

    ![](../../images/module_01/lab_02_idct/ZoomApplicationTimelineEnd.PNG)
	
	Note: system tasks might slow down communication between the application and the hardware simulation, impacting on the measured performance results. The effect of software pipelining is considerably higher when running on the actual hardware. 

### Building the FPGA binary to execute on F1 

Once the application and kernel code have been optimized, the next step is to create an FPGA binary for execution on the F1 instance. 

Creating the FPGA binary is a two-step process:
* First SDAccel is used to build the Xilinx FPGA binary (.xclbin file).
* Then the AWS **create_sdaccel_afi.sh** script is used to create the AWS FPGA binary (.awsxclbin file) and regiser a secured and encrypted Amazon FPGA Image (AFI).

The **create_sdaccel_afi.sh** script does the following:
* Starts a background process to create the AFI
* Generates a \<timestamp\>_afi_id.txt which contains the FPGA Image Identifier (or AFI ID) and Global FPGA Image Identifier (or AGFI ID) of the generated AFI
* Creates the *.awsxclbin AWS FPGA binary file which is read by the host application to determine which AFI should be loaded in the FPGA.

These steps would take too long to complete during this lab, therefore a precompiled FPGA binary is used to continue this lab and execute on F1.

### Executing on F1 

1. Close the SDAccel GUI and report to the terminal from which you started the tool.

1. Confirm that FPGA binary (.awsxclbin file) is indeed present.

	```bash
	# Go the lab folder
	cd ~/SDAccel-AWS-F1-Developer-Labs-master/modules/module_01/idct
	
	# List contents of the ./xclbin directory to look for the .awsxclbin FPGA binary
	ls -la ./xclbin
	```

1. Retrieve the AFI Id from the \<timestamp\>_afi_id.txt file.
	
	```bash
	more ./xclbin/18_03_08-054838_afi_id.txt
	```	

1. Confirm that the AFI is ready and available using the retrieved AFI Id.

	``` bash
	aws ec2 describe-fpga-images --fpga-image-ids afi-019771ed57a751918
	```
   
   The output of this command should contain:

    ```json
    ...
    "State": {
        "Code": "available"
    },
    ...
    ```

1. Copy the host application executable built in the SDAccel workspace to the local directory and execute the accelerated application on F1 using the FPGA binary.

	```bash
    # Copy the host application executable
    cp ~/SDAccel-AWS-F1-Developer-Labs-master/modules/module_01/idct/workspace/IDCT/Emulation-HW/IDCT.exe .

    sudo sh
    # Source the SDAccel runtime environment
    source /opt/Xilinx/SDx/2017.1.rte/setup.sh
    # Execute the host application with the .awsxclbin FPGA binary
    ./IDCT.exe ./xclbin/krnl_idct.hw.xilinx_aws-vu9p-f1_4ddr-xpr-2pr_4_0.awsxclbin
    ```

    Note the performance difference between the IDCT running on the CPU and the IDCT running in the FPGA.


### Summary  

In this lab, you learned:
* How to use the various reports generated by SDAccel to drive optimization decisions
* How to use pragmas to increase kernel performance
* How to use software pipelining to increase system performance
 
---------------------------------------

<p align="center"><b>
Finish this module: <a href="lab_04_wrap_up.md">Wrap-up and Next Steps</a>
</b></p>  
