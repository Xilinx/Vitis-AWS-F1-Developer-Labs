<table>
<tr>
 <td align="center"><h1>Module 1: Optimize the data movement between CPU and FPGA
 </td>
</tr>
</table>

# Module overview

This module is divided in two labs focusing on interactions between the host CPU and the FPGA (data transfers, task invocations) and their impact on overall performance. The application used in this module is a Bloom filter, a space-efficient probabilistic data structure used to test whether an element is a member of a set. Since building AFIs (FPGA binaries) is not instantaneous, a pre-built AFI has been provided for both labs. 

1. **Experiencing acceleration** \
You will profile the Bloom filter application and evaluate which sections are best suited for FPGA acceleration. You will also experience the acceleration potential of AWS F1 instances by running the application first as a software-only version and then as an optimized FPGA-accelerated version.

1. **Optimizing host CPU and FPGA interactions for improved performance** \
You will learn the coding techniques used to create the optimized version run in the first lab. Working with a predefined FPGA accelerator, you will experience how to optimize data movements between host and FPGA, how to efficiently schedule the FPGA accelerator and how to overlap computation on the host CPU and the FPGA to maximize application performance. 


---------------------------------------

<p align="center"><b>
Start the first lab: <a href="host_eval.md">Experience FPGA Acceleration</a>
</b></p>
