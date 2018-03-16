## Running the "Hello World" Example

The "hello world" example is an OpenCL application with a simple vector-addition accelerator. This example uses a precompiled FPGA binary to reduce compilation time and streamline the lab.

1. Open a new terminal by right-clicking anywhere in the Desktop area and selecting **Open Terminal**.

1.  Source the SDAccel environment  

    ```bash
    cd ~/aws-fpga
    source sdaccel_setup.sh
    cd ~
    ```
	*Note: the sdaccel_setup.sh script might generate warning messages, but these can be safely ignored.*

1.  Compile the host application

    ```bash
    # Go to the example directory
    cd ~/SDAccel-AWS-F1-Developer-Labs/modules/module_01/helloworld

    # Compile the host application (./helloworld)
    make TARGETS=hw DEVICES=$AWS_PLATFORM exe
    ```

1. Confirm the presence of the precompiled FPGA binary.

    ```bash
    ls -la ./xclbin/vector_addition.hw.xilinx_aws-vu9p-f1_4ddr-xpr-2pr_4_0.awsxclbin
    ```

1. Execute the host application with the precompiled FPGA binary on the F1 instance.

    ```bash
    sudo sh
    source /opt/Xilinx/SDx/2017.1.rte/setup.sh
    ./helloworld
    ```

1. The host application executes using the vector_addition kernel running in the FPGA and produces the following results:

    ```shell
    Device/Slot[0] (/dev/xdma0, 0:0:1d.0)
    xclProbe found 1 FPGA slots with XDMA driver running
    platform Name: Xilinx
    Vendor Name : Xilinx
    Found Platform
    Found Device=xilinx:aws-vu9p-f1:4ddr-xpr-2pr:4.0
    XCLBIN File Name: vector_addition
    INFO: Importing ./vector_addition.hw.xilinx_aws-vu9p-f1_4ddr-xpr-2pr_4_0.awsxclbin
    Loading: './vector_addition.hw.xilinx_aws-vu9p-f1_4ddr-xpr-2pr_4_0.awsxclbin'
    Result =
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    Hello World !!!
    TEST PASSED
    sh-4.2#
    ```

1. You compiled a host application and successfully executed it on F1 using a pre-compiled Amazon FPGA Image (AFI).

1. Close your terminal.

    ```bash
    exit
    exit
    ```

This concludes this first lab.

---------------------------------------

<p align="center"><b>
Start the next lab: <a href="lab_02_idct.md">2. Developing, profiling and optimizing F1 applications with SDAccel</a>
</b></p>
