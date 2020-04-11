
<table>
 <tr>
 <td align="center"><h1>Methodology for Optimizing Accelerated FPGA Applications
 </td>
 </tr>
</table>

# 7. Running the Accelerator in Hardware

Until now, the results of all the previous labs have been run in hardware emulation mode to give you an idea of how the optimization improves performance, while reducing the compilation time needed to build the system. In this section, you will build and run each of the previous optimizations in hardware on AWS F1.

After each run is finished, kernel execution time and throughput is printed out. The numbers will be filled in the table at the end of the section. Your numbers might vary.  
Note the following data:

* **Total Data**: Calculated by (frame number x frame size).
* **Total Time**: Measured in hardware Timeline Trace report. For a fair comparison, this will include the data transfer and kernel execution time.
* **Throughput**: Calculated by Total Data Processed(MB)/Total Time(s)

>**IMPORTANT**: Generating of xclbin on hardware can take significant time to complete. For your convenience, pre-built xclbin files are provided under /design/xclbin folder.

To run the labs on F1 hardware, you need to follow the steps listed below. The same steps are applied to all five labs, so we will not repeat it in every section.


```bash
export LAB_WORK_DIR=/home/centos/src/project_data
cd $LAB_WORK_DIR/Vitis-AWS-F1-Developer-Labs/modules/module_03/design/build/baseline

# Source the Vitis runtime environment
source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh

# Execute the host application with the .awsxclbin FPGA binary for Step "baseline"
./convolution.exe  --kernel_name convolve_fpga ../../video.mp4 ../../xclbin/baseline/fpga_container_hw.awsxclbin

```


## Run the Baseline Application on Hardware

Use the previously introduced method to run baseline design on F1 hardware.

The host code will print out kernel execution time and throughput at the end of execution. These numbers will be filled into the table at the bottom as benchmarking comparison data.

You should see similar result for baseline run:

```
FPGA Time:       1087.77 s
FPGA Throughput: 0.959894 MB/s
```

## Run the Memory Transfer Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware.
You should see similar result for localbuf run:

```
FPGA Time:       127.487 s
FPGA Throughput: 8.1902 MB/s
```

## Run Fixed Point Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware.
You should see similar result for fixedpoint run:

```
FPGA Time:       31.2099 s
FPGA Throughput: 33.4554 MB/s
```

## Run Dataflow Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware.
You should see similar result for dataflow run:

```
FPGA Time:       5.70991 s
FPGA Throughput: 182.865 MB/s
```

## Run Multiple Compute Units Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware.
You should see similar result for multicu run:

```
FPGA Time:       3.2669 s
FPGA Throughput: 319.612 MB/s
```

## Performance Table

The final performance benchmarking table displays as follows.

| Step                            | Image Size   | Number of Frames  | Time (Hardware) (s) | Throughput (MBps) |
| :-----------------------        | :----------- | ------------: | ------------------: | ----------------: |
| baseline                        |     1920x1080 |           132 |              1087 | 0.96              |
| localbuf                        |     1920x1080 |           132 |                127.487 | 8.19 (8.5x)         |
| fixed-point data                |     1920x1080 |           132 |                31.21 | 33.45 (4x)        |
| dataflow                        |     1920x1080 |           132 |                5.71 | 182.86 (5.46x)        |
| multi-CU                        |     1920x1080 |           132 |                3.27 | 319 (1.74x)       |

---------------------------------------


## Conclusion

Congratulations! You have successfully completed all the modules of this lab to convert a standard CPU-based application into an FPGA accelerated application, running with nearly 300X the throughput when running on the AWS F1 VU9P card. You set performance objectives, and then you employed a series of optimizations to achieve your objectives.

1. You created a Vitis application from a basic C application.
1. You familiarized yourself with the reports generated during software and hardware emulation.
1. You explored various methods of optimizing your HLS kernel.
1. You learned how to set an OpenCL API command queue to execute out-of-order for improved performance.
1. You enabled your kernel to run on multiple CUs.
1. You used the HLS dataflow directive, and explored how it affected your application.
1. You ran the optimized application on the AWS F1 VU9P card to see the actual performance gains.
1. You get familiar with running accelerated applications on AWS F1 FPGA instance.

</br>
<hr/>
<p align="center"><b><a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>
