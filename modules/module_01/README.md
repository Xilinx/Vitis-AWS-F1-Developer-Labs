<table style="width:100%">
  <tr>
    <th width="100%" colspan="5"><h2>SDAccel AWS F1 Developer Labs</h2></th>
  </tr>
  <tr>
    <td width="20%" align="center"><b>Module Overview</b></td>
    <td width="20%" align="center"><a href="./modules/module_01/lab_01_helloworld.md">1. Running Helloworld</a></td>
    <td width="20%" align="center"><a href="./modules/module_01/lab_02_idct.md">2. Developing F1 applications</a></td>
    <td width="20%" align="center"><a href="./modules/module_01/lab_03_wrap_up.md">3. Wrapping-up</a></td>
  </tr>
</table>

#### Overview of the AWS F1 platform and SDAccel flow

The architecture of the AWS F1 platform and the SDAccel development flow are pictured below:

![](./images/module_01/overview/f1_platform.png)

1. Amazon EC2 F1 is a compute instance combining x86 CPUs with Xilinx FPGAs. The FPGAs are programmed with custom hardware accelerators which can accelerate complex workloads up to 30x when compared with servers that use CPUs alone. 
2. An F1 application consists of an x86 executable for the host application and an FPGA binary (also referred to as Amazon FPGA Image or AFI) for the custom hardware accelerators. Communication between the host application and the accelerators are automatically managed by the OpenCL runtime.
3. SDAccel is the development environment used to create F1 applications. It comes with a fully fledged IDE, x86 and FPGA compilers, profiling and debugging tools.
4. The host application is written in C or C++ and uses the OpenCL API to interact with the accelerated functions. The accelerated functions (also referred to as kernels) can be written in C, C++, OpenCL or even RTL.


#### Overview of the this module

This module is divided in 3 labs. It is recommended to complete each lab before proceeding to the next.

1. **Running "Hello World"** \
You will get familiar with the steps involved in running a simple application on the F1 instance. 
1. **Developing and optimizing F1 applications with SDAccel** \
You will use the SDAccel development environment to create, profile and optimize an F1 accelerator. The lab focuses on the Inverse Discrete Cosine Transform (IDCT), a compute intensive function used at the heart of all video codecs.
1. **Wrap-up and next steps** \
You will to close your RDP session, stop your F1 instance and explore next steps to continue your F1 experience after the Xilinx Developer Lab.

Since building FPGA binaries is not instantaneous, all the modules of this Developer Lab will use precompiled FPGA binaries.

---------------------------------------

<p align="center"><b>
Start the next lab: <a href="./modules/module_01/lab_01_helloworld.md">1. Running Helloworld</a>
</b></p>
