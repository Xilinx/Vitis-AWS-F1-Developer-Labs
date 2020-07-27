# Module 1: Introduction to the Vitis Acceleration Flow

  - [Preparing to Run the Labs](#preparing-to-run-the-labs)
  - [Module Overview](#module-overview)

## Preparing to Run the Labs

Before starting this module, perform a fresh reinstall of the AWS EC2 FPGA Development Kit and download the lab contents on your instance. Open a new terminal by right-clicking anywhere in the desktop area and by selecting **Open Terminal**, then run the following commands:

```bash  
# Install the AWS EC2 FPGA Development Kit
git clone https://github.com/aws/aws-fpga.git $AWS_FPGA_REPO_DIR
cd $AWS_FPGA_REPO_DIR
source vitis_setup.sh

# Download the Vitis F1 Developer Labs
export LAB_WORK_DIR=/home/centos/src/project_data/
cd $LAB_WORK_DIR
rm -rf Vitis-AWS-F1-Developer-Labs
git clone https://github.com/Xilinx/Vitis-AWS-F1-Developer-Labs.git Vitis-AWS-F1-Developer-Labs
```

## Module Overview

The module is divided into multiple labs. The main goals of this module are:

1. **Vitis&trade; Acceleration Environment** \
To familiarize you with the steps involved in setting up the environment, building simple **Hello World** host application, and running it on the AWS F1 instance using FPGA card.
1. **Becoming Familiar With IDCT Application** \
To discuss the application structure, the use of OpenCL APIs for host side, and the modeling style used for accelerated function(kernel).
1. **Application Performance Analysis**\
To build and launch an application in different emulation modes to generate different types of traces and performance reports and also verify FPGA output results against software reference model
1. **IDCT Kernel Hardware Optimizations and Performance Analysis**\
To demonstrate the Dataflow hardware optimization that enables significant acceleration and discuss some micro-architecture variations by using different loop pipelining constraints.
1. **Host Host Code Performance Optimizations and Wrap Up**\
To illustrate how Vitis analysis tools and reports can be used to optimally structure host code for significant performance improvements

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance, and explore next steps to continue your experience with Vitis on AWS.

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="lab_01_vitis_env_setup.md">Vitis Acceleration Environment</a>
</b></p>
