# SDAccel AWS F1 Developer Labs
Welcome to the SDAccel AWS F1 Workshop. By going through these modules you will gain hands-on experience with AWS F1 and learn how to develop accelerated applications using the Xilinx SDAccel development environment.

### Setup
For this workshop, we have provisioned an F1 instance for you to work on.
Follow the 

### Module 1 - Optimize the data movement between CPU and FPGA
* Determine the software functions to be accelerated and experience the acceleration gain on FPGA
* Start with naive approach of sending the entire data from host to FPGA as a serial execution
* Experience acceleration gain by spltting entire data over multiple buffers and overlapping FPGA processing with CPU processing

[**FOLLOW THE INSTRUCTIONS**](./modules/module_01/README.md)

### Module 2 - Methodology for Optimizing Accelerated FPGA Applications
* Determine the software functions to be accelerated
* Begin with a baseline design of 2D convolution of an RGBA video and a set of filter coefficients using ffmpeg
* Perform a series of optimization methods to achieve the performance target

[**FOLLOW THE INSTRUCTIONS**](./modules/module_02/README.md)

---------------------------------------

Visit the [SDAccel Developer Zone](https://www.xilinx.com/products/design-tools/software-zone/sdaccel.html) for more information about FPGA acceleration
