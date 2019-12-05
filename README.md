## **NOTE:** This branch is for use during re:Invent 2019 only. Examples may have changed and setup might not work correctly. To run this workshop in your own account, please go to [Xilinx SDAccel AWS F1 Developer Labs](https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs)

# AWS re:Invent 19 – Accelerate your C/C++ applications with Amazon EC2 F1 Instances

Welcome to the workshop on accelerating your C/C++ applications with Amazon EC2 F1 instances.

This workshop will provide you with hands-on development experience on AWS EC2 F1 instances using FPGAs to enable custom hardware accelerations. Using F1 instances to deploy hardware accelerations can be useful in many applications to solve complex science, engineering, and business problems that require high bandwidth, enhanced networking, and very high compute capabilities. F1 instances provide diverse development environments: from low-level hardware developers to software developers who are more comfortable with C/C++ and openCL environments [available on our GitHub](https://github.com/aws/aws-fpga).

Once your FPGA design is complete, you can register it as an Amazon FPGA Image(AFI), and deploy it to your F1 instance in just a few clicks. You can reuse your AFIs as many times as you like, and across as many F1 instances as you like. There is no software charge for the development tools when using the FPGA developer AMI and you can program the FPGAs on your F1 instance as many times as you like with no additional fees.  

This workshop will focus on developing accelerated applications using the Xilinx SDAccel development environment.

### Overview
* This workshop is focused on identifying parts of your C/C++ program that are best suited for acceleration on an FPGA. We will also use profiling data to analyze how to further optimize data movement.
* In the interest of time, this workshop will use pre-built AFIs and pre-provisioned F1 instances. 
    * We have modified this branch(Reinvent_2019.1) of the [Xilinx SDAccel AWS F1 Developer Labs](https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs) specifically for use during the CMP411 workshop at Re:Invent 2019.
    * Attendees will not use their personal or corporate accounts for the duration of this workshop.
* Each attendee will receive information to connect to an F1 instance. Use it while following the [setup steps](./setup/instructions.md) before beginning this workshop.
* If at any point, you feel stuck or have questions, please raise your hand and we would be happy to assist you.

> **NOTE:** 
> The labs on [Github](https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs) are available for you to run on your own F1 instance after the workshop ends. 

### Setup
* For this workshop, we have pre-provisioned an F1 instance for every attendee to work on.
* Please go through the instructions to connect to your instance and get started with the workshop.

[**FOLLOW THE INSTRUCTIONS**](./setup/instructions.md)

### Module 1 - Optimize the data movement between CPU and FPGA [Time: ~1.5hrs]
* Profile the software application and determine which functions to accelerate on the FPGA
* Working with a predefined FPGA accelerator, optimize performance by overlapping data transfers and FPGA execution
* Further speed-up the application by overlapping FPGA execution and CPU processing

[**FOLLOW THE INSTRUCTIONS**](./modules/module_01/README.md)

### Module 2 - Methodology for Optimizing Accelerated FPGA Applications [Time: ~4hrs]
* Determine the software functions to be accelerated
* Begin with a baseline design of 2D convolution of an RGBA video and a set of filter coefficients using ffmpeg
* Perform a series of optimization methods to achieve the performance target

> **NOTE:** This module is intended to be a take-home to be done on your own F1 instance, but feel free to get started and ask us questions about it during the workshop!
> This workshop can be accessed on [https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs](https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs) to run on your own F1 instance.

[**FOLLOW THE INSTRUCTIONS**](./modules/module_02/README.md)

---------------------------------------

Visit the [SDAccel Developer Zone](https://www.xilinx.com/products/design-tools/software-zone/sdaccel.html) for more information about FPGA acceleration
