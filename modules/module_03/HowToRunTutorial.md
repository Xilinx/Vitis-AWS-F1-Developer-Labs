<table>
 <tr>
 <td align="center"><h1>Applying Methodology for Creating an Optimized Accelerated FPGA Application
 </td>
 </tr>
</table>

# Understanding the Makefile

The source files required for this lab are all under the `design` folder. The `design` folder contains three sub-directories: `cpu_src` with the original application code, `src` with the accelerated application code, and `makefile`.

Under the `src` folder, there are six sub-folders, which contain the source files for each step. The following directories correspond to each step of the tutorial.

| Tutorial Step                                             | Associated Directory  |
|-------------------------------------------------------------|------------------------ |
| Creating a Vitis application from C application          |     [src/baseline](./design/src/baseline)        |
| Optimizing memory transfers                                  |     [src/localbuf](./design/src/localbuf)        |
| Optimizing by using fixed point data types:                 |     [src/fixedpoint](./design/src/fixedpoint)      |
| Optimizing with dataflow                                    |     [src/dataflow](./design/src/dataflow)        |
| Using Out-of-order queues and multiple kernels              |     [src/multicu](./design/src/multicu)         |

Makefile targets can be used to run software emulation, hardware emulation, as well as hardware on the Alveo accelerator card.

## Makefile Targets for Building Designs

To build the design, use the following command.

```
make build TARGET=<sw_emu/hw_emu/hw> STEP=<baseline/localbuf/fixedpoint/dataflow/multicu/> SOLUTION=1
```

Along with the `src` directory, that can be used to update the code based on tutorials, the `reference-files` directory is also provided. You can directly use files in the `reference-files` directory by setting `SOLUTION=1` for the makefile targets.

For example, to build the hardware emulation xclbin for the baseline lab, use the following command.

```
make build TARGET=hw_emu STEP=baseline SOLUTION=1
```

## Makefile Targets for Running Designs

To run the design in emulation or hardware, use the following command.

```
make run TARGET=<sw_emu/hw_emu/hw> STEP=<baseline/localbuf/fixedpoint/dataflow/multicu> SOLUTION=1 [NUM_FRAMES=1]
```

The NUM_FRAMES option defines the input video frame number used for the host application execution. For software and hardware emulation, it is recommended to set NUM_FRAMES to **1** to reduce emulation time. For hardware run, do not use this option; a full video file will be used.

For example, to run hardware emulation for baseline step, use the following command.

```
make run TARGET=hw_emu STEP=baseline SOLUTION=1 NUM_FRAMES=1
```

## Makefile Targets for Clean Up

To clean the previously generated build, use the following command.

```
make clean TARGET=hw_emu STEP=baseline
```

This will clean the hardware emulation-generated files for the baseline version design.

</br>
<hr/>
<p align="center"><b><a href="../../README.md">Return to Start of Tutorial</a> — <a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>
