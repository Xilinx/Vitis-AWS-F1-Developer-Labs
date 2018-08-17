COMMON_REPO := ${AWS_FPGA_REPO_DIR}/SDAccel/examples/xilinx

#Common Includes
include $(COMMON_REPO)/utility/boards.mk
include $(COMMON_REPO)/libs/xcl2/xcl2.mk
include $(COMMON_REPO)/libs/opencl/opencl.mk

# Host Application
IDCT.exe_SRCS=./src/idct.cpp
IDCT.exe_HDRS=
IDCT.exe_CXXFLAGS=-I./src/ $(opencl_CXXFLAGS) $(xcl2_CXXFLAGS) $(oclHelper_CXXFLAGS) -std=c++0x
IDCT.exe_LDFLAGS=$(opencl_LDFLAGS)

EXES=IDCT.exe

# Kernel
krnl_idct_SRCS=./src/krnl_idct.cpp
krnl_idct_CLFLAGS= -k krnl_idct
krnl_idct_LDCLFLAGS=\
        --sp krnl_idct_1.m_axi_gmem0:bank0 \
        --sp krnl_idct_1.m_axi_gmem1:bank0 \
	--sp krnl_idct_1.m_axi_gmem2:bank1 \
	--kernel_frequency 250 \

XOS=krnl_idct

# xclbins
krnl_idct_XOS=krnl_idct
XCLBINS=krnl_idct

# check
check_EXE=IDCT.exe
check_XCLBINS=krnl_idct

CHECKS=check

include $(COMMON_REPO)/utility/rules.mk
