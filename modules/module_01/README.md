## Module 1: Introduction to the Vitis Flow


### Preparing to run the labs

Before starting this module, perform a fresh reinstall of the AWS EC2 FPGA Development Kit and download the lab contents on your instance. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**, then run the following commands:

```bash  
# Install the AWS EC2 FPGA Development Kit
cd $AWS_FPGA_REPO_DIR
git clone https://github.com/aws/aws-fpga.git  
cd $AWS_FPGA_REPO_DIR                                     
source vitis_setup.sh

# Download the Vitis F1 Developer Labs
export LAB_WORK_DIR=/home/centos/src/project_data/
cd $LAB_WORK_DIR
rm -rf Vitis-AWS-F1-Developer-Labs
git clone https://github.com/Xilinx/Vitis-AWS-F1-Developer-Labs.git Vitis-AWS-F1-Developer-Labs
```

### Module overview

This module is divided in 3 labs. Since building FPGA binaries is not instantaneous, all the labs will use precompiled FPGA binaries. It is recommended to complete each lab before proceeding to the next.

1. **Running the "Hello World" example** \
You will get familiar with the steps involved in running a simple application on the F1 instance.

1. **Using the Vitis v++ compiler to develop F1 accelerated applications** \
You will use the v++ command line flow to create and profile an F1 accelerator. The lab focuses on the Inverse Discrete Cosine Transform (IDCT), a compute intensive function used at the heart of all video codecs.

1. **Optimizing F1 applications** \
You will continue the IDCT project started in the previous lab and learn how you can use the various reports provided by Vitis to guide the performance optimization process of your application.

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with Vitis on AWS.

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="lab_01_helloworld.md">Running the "Hello World" example</a>
</b></p>
