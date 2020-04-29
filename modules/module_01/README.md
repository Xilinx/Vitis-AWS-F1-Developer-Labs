## Module 1: Introduction to the Vitis Acceleration Flow


### Preparing to run the labs

Before starting this module, perform a fresh reinstall of the AWS EC2 FPGA Development Kit and download the lab contents on your instance. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**, then run the following commands:

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

### Module overview
The module is divided into multiple labs. The main goals of these module are:
1. **Vitis Acceleration Environment** \
You will get familiar with the steps involved in setting up the environment, building simple Hello World host application and running it on the AWS F1 instance using FPGA card.
1. **Becoming Familiar With IDCT Application** \
You will have a look at the application structure, the use of OpenCL APIs for host side and accelerated function(kernel) modeling style.
1. **Application Performance Analysis**\
You will build and launch application in different emulation modes to generate different type of traces and performance reports and also verify FPGA output results against software reference model
1. **Using Vitis Analyzer to Review Application Timeline Traces and Tune Performance**\
In this part you will analyze application performance using Vitis Analyzer tool to visualize different application timeline traces and Performance Summaries.
1. **Optimizing FPGA Accelerated Function Implementation using Dataflow**\
In this part you will use learn about Dataflow hardware optimization that will enable significant acceleration. 
1. **Host Code Optimizations for Performance Improvement**\
You will modify host code to improve performance

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with Vitis on AWS.

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="lab_01_vitis_env_setup.md">Vitis Acceleration Environment</a>
</b></p>
