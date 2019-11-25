
runOnfpga_$(TARGET).xo: $(SRCDIR)/compute_score_fpga.cpp $(SRCDIR)/hls_stream_utils.h
	v++ -c -g -t $(TARGET) -R 1 -k runOnfpga \
		--platform xilinx_u200_xdma_201830_2 \
		--save-temps \
		--temp_dir ./temp_dir \
		--report_dir ./report_dir \
		--log_dir ./log_dir \
		--profile_kernel data:all:all:all:all \
		-I$(SRCDIR) \
		-DPARALLELISATION=$(PF) \
		$(SRCDIR)/compute_score_fpga.cpp \
		-o ./runOnfpga_$(TARGET).xo

runOnfpga_$(TARGET).xclbin: ./runOnfpga_$(TARGET).xo
	v++ -l -g -t $(TARGET) -R 1 \
		--platform xilinx_u200_xdma_201830_2 \
		--temp_dir ./temp_dir \
		--report_dir ./report_dir \
		--log_dir ./log_dir \
		--profile_kernel data:all:all:all:all \
		-I$(SRCDIR) \
		./runOnfpga_$(TARGET).xo \
		-o ./runOnfpga_$(TARGET).xclbin 

host: $(SRCDIR)/*.cpp $(SRCDIR)/*.c $(SRCDIR)/*.h
	mkdir -p $(BUILDDIR)
	g++ -D__USE_XOPEN2K8 -D__USE_XOPEN2K8 \
		-I$(XILINX_XRT)/include/ \
		-I$(SRCDIR) \
		-O3 -Wall -fmessage-length=0 -std=c++11\
		$(HOST_SRC_CPP) \
		-L$(XILINX_XRT)/lib/ \
		-lxilinxopencl -lpthread -lrt \
		-o $(BUILDDIR)/host

emconfig.json:
	cp $(SRCDIR)/emconfig.json .

xclbin: runOnfpga_$(TARGET).xclbin

xo: runOnfpga_$(TARGET).xo

clean:
	rm -rf temp_dir log_dir ../build report_dir *log host runOnfpga* *.csv *summary .run .Xil vitis* *jou xilinx*
