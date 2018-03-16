## Module 1: Introduction to the SDAccel Flow


### Downloading the lab contents

Before starting this module, perform a fresh reinstall of the lab contents on your instance. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**, then run the following commands:

```bash  
cd ~
rm -rf SDAccel-AWS-F1-Developer-Labs
git clone https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs.git
```

### Module overview

This module is divided in 3 labs. Since building FPGA binaries is not instantaneous, all the labs will use precompiled FPGA binaries. It is recommended to complete each lab before proceeding to the next.

1. **Running the "Hello World" example** \
You will get familiar with the steps involved in running a simple application on the F1 instance. \

1. **Introduction to the SDAccel development environment** \
You will use the SDAccel development environment to create and profile an F1 accelerator. The lab focuses on the Inverse Discrete Cosine Transform (IDCT), a compute intensive function used at the heart of all video codecs. \

1. **Using the SDAccel GUI to optimize F1 applications** \
You will continue the IDCT project started in the previous lab and learn how you can use the various reports provided by SDAccel to guide the performance optimization process of your application. \

After you complete the last lab, you will be guided to close your RDP session, stop your F1 instance and explore next steps to continue your experience with SDAccel on AWS. 

---------------------------------------

<p align="center"><b>
Start the first lab: <a href="lab_01_helloworld.md">Running the "Hello World" example</a>
</b></p>
