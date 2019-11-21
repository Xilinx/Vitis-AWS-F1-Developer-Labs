## Module 2: Optimize the data movement between CPU and FPGA

### Preparing to run the labs

Skip this step if the SDAccel environment is already setup.

Before starting this module, perform a fresh reinstall of the AWS EC2 FPGA Development Kit and download the lab contents on your instance. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**, then run the following commands:

```
# Install the AWS EC2 FPGA Development Kit
cd ~
git clone https://github.com/aws/aws-fpga.git
cd aws-fpga                                   
source sdaccel_setup.sh

# Download the SDAccel F1 Developer Labs
cd ~
rm -rf SDAccel-AWS-F1-Developer-Labs
git clone https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs.git SDAccel-AWS-F1-Developer-Labs
```



### Setup for running application on FPGA
```
sudo sh
# Source the SDAccel runtime environment
source /opt/xilinx/xrt/setup.sh
```

### Module overview

This module is divided in 2 labs. Since building FPGA binaries is not instantaneous, both labs will use precompiled FPGA binary. The labs focus on data movement from host to kernel and how this affects the performance. Also, the same FPGA binary is being used for both the labs

1. **Experience the acceleration** \
You will evaluate potential candidates in the algorithm for FPGA acceleration. You will also experience the acceleration gain by running application on FPGA and comparing this with application run on CPU

1. **Data Movement Between the Host and Kernel** \

You will learn the techniques how to optimize the data movements between host and FPGA. You will also go through how ovelapping of host and FPGA execution accelerate the application performance. 

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with SDAccel on AWS.
