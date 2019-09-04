
<table>
 <tr>
 <td align="center"><h1>Methodology for Optimizing Accelerated FPGA Applications
 </td>
 </tr>
</table>

# 7. Running the Accelerator in Hardware

Until now, the results of all the previous labs have been run in hardware emulation mode to give you an idea of how the optimization improves performance, while reducing the compilation time needed to build the system. In this section, you will build and run each of the previous optimizations in hardware on an Alveo accelerator card.

After each run is finished, record the performance data from the Timeline Trace report, and fill in the table at the end of the section. Your numbers might vary.  
Note the following data:

* **Total Data**: Calculated by (frame number x frame size).
* **Total Time**: Measured in hardware Timeline Trace report. For a fair comparison, this will include the data transfer and kernel execution time.
* **Throughput**: Calculated by Total Data Processed(MB)/Total Time(s)

>**IMPORTANT**: Each of the steps in this lab compiles the hardware kernel and can take significant time to complete. To save your time, pre-built xclbin files are provided under /design/xclbin folder.

To run the labs on F1 hardware, you need to follow the steps listed below. The same steps are applied to all five labs, so we will not repeat it in every section.

1. Build xclbin file for hardware run.

```
make build TARGET=hw STEP=[baseline/localbuf/fixedpoint/dataflow/multicu] SOLUTION=1
```

2. Create awsxclbin file for F1 configuration.

```
$SDACCEL_DIR/tools/create_sdaccel_afi.sh -xclbin=<input_xilinx_fpga_binary_xclbin_filename>
		-o=<output_aws_fpga_binary_awsxclbin_filename_root> -s3_bucket=<bucket-name> -s3_dcp_key=<dcp-folder-name> -s3_logs_key=<logs-folder-name>
```

3. Step2 will create a *_afi_id.txt file, open this file and record the AFI Id.

4. Check the AFI creation state using AFI ID as shown below.
```
aws ec2 describe-fpga-images --fpga-image-ids <AFI ID>
```

If the state is shown as 'available', it indicates AFI creation is completed.

```
"State": {
              "Code": "available"
        },

```

5. Run host application.

```
sudo sh
# Source the SDAccel runtime environment
source /opt/xilinx/xrt/setup.sh
# Execute the host application with the .awsxclbin FPGA binary
cd build/baseline
./convolution.exe  --kernel_name convolve_fpga ../../video.mp4 ./fpga_container_hw.awsxclbin
```

6. Generate reports.

```
mv ./sdaccel_profile_summary.csv sdaccel_profile_summary_hw.csv
mv ./sdaccel_profile_summary.csv sdaccel_profile_summary_hw.csv
cd ../../design/makefile
make gen_report TARGET=hw STEP=baseline
```


## Run the Baseline Application on Hardware

Use the previously introduced method to run baseline design on F1 hardware. View timeline report after application run is finished.

```
make view_timeline_trace TARGET=hw STEP=baseline
```

You should see a Timeline Trace report similar to the following figure.

![][baseline_hw_timeline]

The two markers record the start and end point of the execution, so the execution time can be roughly calculated as 1090-3 = 1087s.

The total MBs processed is 1920 x 1080 x 4(Bytes) x 132(frames) = 1095 MBs. Therefore, the throughput can be calculated as 1095(MB)/1087(s)= 1 MB/s. Use this number as the benchmark baseline to measure future optimizations.  

## Run the Memory Transfer Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware. View timeline report after application run is finished.

```
make view_timeline_trace TARGET=hw STEP=localbuf
```

You should see a Timeline Trace report for the hardware run similar to the following figure.

![][localbuf_hw_timeline]

The two markers record the start and end point of the execution, so execution time can be roughly calculated as 131.09-3.46 = 127.63s.

## Run Fixed Point Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware. View timeline report after application run is finished.

```
make view_timeline_trace TARGET=hw STEP=fixedpoint
```

You should see a Timeline Trace report for the hardware run similar to the following figure.

![][fixedtype_hw_timeline]

 The two markers record the start and end point of the execution, so the execution time can be roughly calculated as 34.57-3.35 = 31.22s.

## Run Dataflow Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware. View timeline report after application run is finished.

```
make view_timeline_trace TARGET=hw STEP=dataflow
```

You should see a Timeline Trace report for the hardware run, similar to the following figure.

![][dataflow_hw_timeline]

The two markers record the start and end point of the execution, so the execution time can be roughly calculated as 19.09-13.39 = 5.7s.

### Run Multiple Compute Units Lab on Hardware

Use the previously introduced method to run baseline design on F1 hardware. View timeline report after application run is finished.

```
make view_timeline_trace TARGET=hw STEP=multicu
```

You should see a Timeline Trace report for the hardware run, similar to the following figure.

![][hostopt_hw_timeline]

 The two markers record the start and end point of the execution, so execution time can be roughly calculated as 17.48-13.86 = 3.62s.

### Performance Table

The final performance benchmarking table displays as follows.

| Step                            | Image Size   | Number of Frames  | Time (Hardware) (s) | Throughput (MBps) |
| :-----------------------        | :----------- | ------------: | ------------------: | ----------------: |
| baseline                        |     1920x1080 |           132 |              1087 | 1              |
| localbuf                        |     1920x1080 |           132 |                127.63 | 8.58 (8.58x)         |
| fixed-point data                |     1920x1080 |           132 |                31.22 | 35 (4x)        |
| dataflow                        |     1920x1080 |           132 |                5.7 | 192.1 (5.5x)        |
| multi-CU                        |     1920x1080 |           132 |                3.62 | 302 (1.57x)       |

---------------------------------------

[baseline_hw_timeline]:./images/baseline_hw_timeline_aws.JPG "Baseline version hardware Timeline Trace Report"
[localbuf_hw_timeline]:./images/localbuf_hw_timeline_aws.JPG "Local buffer version hardware Timeline Trace Report"
[fixedtype_hw_timeline]:./images/fixedtype_hw_timeline_aws.JPG "Fixed-type data version hardware Timeline Trace Report"
[dataflow_hw_timeline]:./images/dataflow_hw_timeline_aws.JPG "Dataflow version hardware Timeline Trace Report"
[hostopt_hw_timeline]: ./images/multicu_hw_timeline_aws.JPG "Host code optimization version hardware timeline trace report"

## Conclusion

Congratulations! You have successfully completed all the modules of this lab to convert a standard CPU-based application into an FPGA accelerated application, running with nearly 300X the throughput when running on the AWS F1 VU9P card. You set performance objectives, and then you employed a series of optimizations to achieve your objectives.

1. You created an SDAccel application from a basic C application.
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
