## Module 2: Optimize interactions between the host CPU and the FPGA

### Preparing to run the labs

Skip this step if the Vitis environment is already setup.

Before starting this module, perform a fresh reinstall of the AWS EC2 FPGA Development Kit and download the lab contents on your instance. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**, then run the following commands:

```bash  
# Install the AWS EC2 FPGA Development Kit
cd $AWS_FPGA_REPO_DIR
git clone https://github.com/aws/aws-fpga.git                                         
source vitis_setup.sh

# Download the Vitis F1 Developer Labs
cd $AWS_FPGA_REPO_DIR/
rm -rf Vitis-AWS-F1-Developer-Labs
git clone https://github.com/Xilinx/Vitis-AWS-F1-Developer-Labs.git Vitis-AWS-F1-Developer-Labs
```

### Setup for running application on FPGA
```bash
# Source the Vitis runtime environment
export LAB_WORK_DIR=/home/centos/src/project_data/
source $AWS_FPGA_REPO_DIR/vitis_runtime_setup.sh 
```

### Module overview

This module is divided in two labs focusing on interactions between the CPU and the FPGA (data transfers, task invocations) and their impact on overall performance. The application used in this module is a Bloom filter, a space-efficient probabilistic data structure used to test whether an element is a member of a set. Since building FPGA binaries is not instantaneous, a precompiled FPGA binary is provided for both labs.

1. **Experiencing acceleration** \
You will profile the Bloom filter application and evaluate which sections are best suited for FPGA acceleration. You will also experience the acceleration potential of AWS F1 instances by running the application first as a software-only version and then as an optimized FPGA-accelerated version.

1. **Optimizing CPU and FPGA interactions for improved performance** \
You will learn the coding techniques used to create the optimized version run in the first lab. Working with a predefined FPGA accelerator, you will experience how to optimize data movements between host and FPGA, how to efficiently invoke the FPGA kernel and how to overlap computation on the CPU and the FPGA to maximize application performance.

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with Vitis on AWS.

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="host_eval.md">Experience FPGA Acceleration</a>
</b></p>
