.PHONY: help

help::
	@echo  " Makefile Usage:"
	@echo  ""
	@echo  "  make build TARGET=<sw_emu/hw_emu/hw> "
	@echo  "  Command to generate the design for specified target"
	@echo  ""
	@echo  "  make run TARGET=<sw_emu/hw_emu/hw> "
	@echo  "  Command to generate and run the design for specified target"
	@echo  ""
	@echo  "  make clean TARGET=<sw_emu/hw_emu/hw> "
	@echo  "  Command to remove the generated non-hardware files for specified target"
	@echo  ""


## platform selection
## Several supported platforms are liste here.
## Or you can add other platfom and set PLATFORM variable to it

PLATFORM := $(AWS_PLATFORM)
#PLATFORM := xilinx_u200_xdma_201830_2
#PLATFORM := xilinx_aws-vu9p-f1-04261818_dynamic_5_0

ENABLE_STALL_TRACE ?= no

## TARGET can be set as:
## sw_emu: software emulation
## hw_emu: hardware Emulation
## hw: hardware run
TARGET := sw_emu

## Below are the names for host executable and xclbin.
## Please keep it unchanged.
HOST_EXE := host.exe
XO_NAME := krnl_idct.$(TARGET)
XO_NAME_NOFLOW := krnl_idct_noflow.$(TARGET)
XO_NAME_MED := krnl_idct_med.$(TARGET)
XO_NAME_SLOW := krnl_idct_slow.$(TARGET)
XCLBIN := krnl_idct.$(TARGET).awsxclbin

SRC_REPO := ./src

ifeq ($(TARGET), sw_emu)
BUILD_DIR ?= ./build_sw_emu
endif

ifeq ($(TARGET), hw_emu)
BUILD_DIR ?= ./build_hw_emu
endif

ifeq ($(TARGET), hw)
BUILD_DIR ?= ./build
endif




################ Set the size of emulation data
EMU_FULL ?= 1 
BATCH_SIZE ?= 32
NUM_OF_BATCHES ?= 8
MAX_SCHEDULED_BATCHES ?= 1

## Host Application files repository

#HOST_SRC_CPP := $(SRC_REPO)/xcl2.cpp
HOST_SRC_CPP := $(SRC_REPO)/host.cpp 
HOST_SRC_CPP += $(SRC_REPO)/idct.cpp
## Kernel Source Files repository

KERNEL_SRC_CPP := $(SRC_REPO)/krnl_idct.cpp
KERNEL_NOFLOW_SRC_CPP := $(SRC_REPO)/krnl_idct_noflow.cpp
KERNEL_MED_SRC_CPP := $(SRC_REPO)/krnl_idct_med.cpp
KERNEL_SLOW_SRC_CPP := $(SRC_REPO)/krnl_idct_slow.cpp
KERNEL_INCLUDES = -I$(SRC_REPO)
# Host Compiler Global Settings and Include Libraries

# Definition of include file locations
#XRT_PATH = $(XILINX_XRT)
OPENCL_INCLUDE:= $(XRT_PATH)/include


ifneq ($(shell expr $(shell g++ -dumpversion) \>= 5), 1)
ifndef XILINX_VIVADO
$(error [ERROR]: g++ version older. Please use 5.0 or above.)
else
CXX := $(XILINX_VIVADO)/tps/lnx64/gcc-6.2.0/bin/g++
$(warning [WARNING]: g++ version older. Using g++ provided by the tool : $(CXX))
endif
endif


XCL2_SRC = ./includes/xcl2/xcl2.cpp

CXXFLAGS += -I./includes/xcl2


#CXXFLAGS += -D__USE_XOPEN2K8
CXXFLAGS += -I$(XILINX_XRT)/include/
CXXFLAGS += -I$(XILINX_VIVADO)/include/
CXXFLAGS += -I$(SRC_REPO)
CXXFLAGS += -O0 -g -Wall -fmessage-length=0 -std=c++11
CXXFLAGS += $(XCL2_SRC)


CXXLDFLAGS := -L$(XILINX_XRT)/lib/
CXXLDFLAGS += -lOpenCL -lpthread -lrt -lstdc++ -lxilinxopencl 

## Kernel Compiler and Linker Flags

VPPFLAGS := -t $(TARGET)
VPPFLAGS += --platform $(PLATFORM) -R1 --save-temps
VPPFLAGS += --temp_dir $(BUILD_DIR)
VPPFLAGS += --log_dir $(BUILD_DIR)
VPPFLAGS += --profile_kernel data:all:all:all:all

VPPFLAGS += --trace_memory DDR[3]

ifeq ($(ENABLE_STALL_TRACE),yes)
VPPFLAGS += --profile_kernel stall:all:all:all
endif

VPPFLAGS += --config krnl_idct.cfg

## Host Executable File Generation

compile_host: $(BUILD_DIR)/$(HOST_EXE)

$(BUILD_DIR)/$(HOST_EXE): $(HOST_SRC_CPP)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(HOST_SRC_CPP) $(CXXLDFLAGS)
	cp ./xrt.ini $(BUILD_DIR)/xrt.ini

## Kernel XO and Xclbin File Generation

$(BUILD_DIR)/$(XO_NAME).xo: $(KERNEL_SRC_CPP)
	mkdir -p $(BUILD_DIR)
	v++ $(VPPFLAGS) -c -k krnl_idct $(KERNEL_INCLUDES) $(KERNEL_SRC_CPP) -o $@

$(BUILD_DIR)/$(XO_NAME_NOFLOW).xo: $(KERNEL_NOFLOW_SRC_CPP)
	mkdir -p $(BUILD_DIR)
	v++ $(VPPFLAGS) -c -k krnl_idct_noflow $(KERNEL_INCLUDES) $(KERNEL_NOFLOW_SRC_CPP) -o $@

$(BUILD_DIR)/$(XO_NAME_MED).xo: $(KERNEL_MED_SRC_CPP)
	mkdir -p $(BUILD_DIR)
	v++ $(VPPFLAGS) -c -k krnl_idct_med $(KERNEL_INCLUDES) $(KERNEL_MED_SRC_CPP) -o $@

$(BUILD_DIR)/$(XO_NAME_SLOW).xo: $(KERNEL_SLOW_SRC_CPP)
	mkdir -p $(BUILD_DIR)
	v++ $(VPPFLAGS) -c -k krnl_idct_slow $(KERNEL_INCLUDES) $(KERNEL_SLOW_SRC_CPP) -o $@

#$(BUILD_DIR)/$(XO_NAME_MED).xo $(BUILD_DIR)/$(XO_NAME_SLOW).xo

$(BUILD_DIR)/$(XCLBIN): $(BUILD_DIR)/$(XO_NAME).xo $(BUILD_DIR)/$(XO_NAME_MED).xo $(BUILD_DIR)/$(XO_NAME_SLOW).xo $(BUILD_DIR)/$(XO_NAME_NOFLOW).xo

	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(TARGET)
	v++ $(VPPFLAGS) -l -o $@ $(BUILD_DIR)/$(XO_NAME).xo $(BUILD_DIR)/$(XO_NAME_MED).xo $(BUILD_DIR)/$(XO_NAME_SLOW).xo $(BUILD_DIR)/$(XO_NAME_NOFLOW).xo
	
#$(BUILD_DIR)/$(XO_NAME_MED).xo $(BUILD_DIR)/$(XO_NAME_SLOW).xo

## Emulation Files Generation

EMCONFIG_FILE = emconfig.json

$(BUILD_DIR)/$(EMCONFIG_FILE):
	 emconfigutil --nd 1  --platform $(PLATFORM) --od $(BUILD_DIR)

# primary build targets

.PHONY: all clean

## build the design without running host application

ifeq ($(TARGET), hw)
build: $(BUILD_DIR)/$(HOST_EXE) $(BUILD_DIR)/$(XCLBIN) $(BUILD_DIR)/$(EMCONFIG_FILE)
else
build: $(BUILD_DIR)/$(HOST_EXE) $(BUILD_DIR)/$(XCLBIN) $(BUILD_DIR)/$(EMCONFIG_FILE)
endif

## build the design and then run host application

run: build
	cp xrt.ini $(BUILD_DIR);
ifeq ($(TARGET), hw)
	cd $(BUILD_DIR) && unset XCL_EMULATION_MODE;    ./host.exe  ./$(XCLBIN) $(BATCH_SIZE) $(NUM_OF_BATCHES) $(MAX_SCHEDULED_BATCHES) $(EMU_FULL)

else
	cd $(BUILD_DIR) && XCL_EMULATION_MODE=$(TARGET) ./host.exe  ./$(XCLBIN) $(BATCH_SIZE) $(NUM_OF_BATCHES) $(MAX_SCHEDULED_BATCHES) $(EMU_FULL)
endif
	
	
## Clean generated files

clean:
	rm -rf $(BUILD_DIR) host*.exe
