# Before You Begin

## AWS F1 environment setup

Skip this step if the SDAccel environment is already setup
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



## Setup for running application on FPGA
```
sudo sh
# Source the SDAccel runtime environment
source /opt/xilinx/xrt/setup.sh
```
