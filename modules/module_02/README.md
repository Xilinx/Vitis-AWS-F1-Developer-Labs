## Module 1: Optimize the data movement between CPU and FPGA

### Prerequisites

Run the following command 

```
source /opt/xilinx/xrt/setup.sh
```

### Module overview

This module is divided in 2 labs. Since building FPGA binaries is not instantaneous, both labs will use precompiled FPGA binary. The labs focus on data movement from host to kernel and how this affects the performance. Also, the same FPGA binary is being used for both the labs

1. **Experience the acceleration** \
You will evaluate potential candidates in the algorithm for FPGA acceleration. You will also experience the acceleration gain by running application on FPGA and comparing this with application run on CPU

1. **Data Movement Between the Host and Kernel** \
You will learn the techniques how to optimize the data movements between host and FPGA. You will also go through how ovelapping of host and FPGA execution accelerate the application performance. 

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with SDAccel on AWS.

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="host_eval.md">Experience FPGA Acceleration</a>
</b></p>
