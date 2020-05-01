/**********
Copyright (c) 2020, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#include <string.h>
#include <stdio.h>
#include <ap_int.h>
#include <hls_stream.h>



#include "types.hpp"


/* *************************************************************************** 

reg:

Simple bridge function which is prohibited to be inlined during
synthesis which forces the insertion of registers.

*************************************************************************** */
template <typename T_reg>
T_reg reg(T_reg x) 
{
  #pragma HLS INLINE off
  return x;
}


/* *************************************************************************** 

read_blocks:

Dataflow block used to interface from input memory to streaming input
channels.
    
    in      	: Pointer to input data essentially generate a memory interface
    out     : stream of output data that connects to memroy interface 

*************************************************************************** */
template<typename out_t>
void read_blocks_noflow(const out_t *in, hls::stream<out_t> &out, unsigned int numBlocks) 
{
  for(unsigned int i = 0; i < numBlocks*2; i++) 
  {
    #pragma HLS loop_tripcount min=2048 max=2048
    #pragma HLS PIPELINE

    out.write(in[i]);
  }
}

/* *************************************************************************** 

write_blocks:

Dataflow block used to interface from streaming output channel to
output memory.

*************************************************************************** */
void write_blocks_noflow(ap_int<512> *out,  hls::stream<int512_t> &in,  unsigned int numBlocks) 
{
	for(unsigned int i = 0; i < numBlocks*2; i++) 
	{
		#pragma HLS loop_tripcount min=2048 max=2048
		#pragma HLS PIPELINE

		out[i] = in.read();
	}
}




/* ***************************************************************************

idct:

Idct algorithm description describes synthesizable idct behavior. 


    inputBlock  :  input data 8x8 block
    coeffs      :  IDCT coefficients 8x8 block
    outputBlock :  IDCT output 8x8 block
    ignoreDC    :  flag to ignore DC value

*************************************************************************** */


void idct_noflow(const int16_t inputBlock[64], 
	  	  const uint16_t coeffs[64], 
	  	  int16_t outputBlock[64], 
	  	  bool ignoreDC
		 )
{
	#pragma HLS INLINE

	int32_t intermed[64];

	const uint16_t w1 = 2841; // 2048*sqrt(2)*cos(1*pi/16)
	const uint16_t w2 = 2676; // 2048*sqrt(2)*cos(2*pi/16)
	const uint16_t w3 = 2408; // 2048*sqrt(2)*cos(3*pi/16)
	const uint16_t w5 = 1609; // 2048*sqrt(2)*cos(5*pi/16)
	const uint16_t w6 = 1108; // 2048*sqrt(2)*cos(6*pi/16)
	const uint16_t w7 = 565;  // 2048*sqrt(2)*cos(7*pi/16)

	const uint16_t w1pw7 = w1 + w7;
	const uint16_t w1mw7 = w1 - w7;
	const uint16_t w2pw6 = w2 + w6;
	const uint16_t w2mw6 = w2 - w6;
	const uint16_t w3pw5 = w3 + w5;
	const uint16_t w3mw5 = w3 - w5;

	const uint16_t r2 = 181; // 256/sqrt(2)
  
	// Horizontal 1-D IDCT.
	for (int y = 0; y < 8; ++y) 
	{
		int y8 = y * 8;

		int32_t x0 = (((ignoreDC && y == 0)
			   		 ? 0 : (inputBlock[y8 + 0] * coeffs[y8 + 0]) << 11)) + 128;

		int32_t x1 = (inputBlock[y8 + 4] * coeffs[y8 + 4]) << 11;
		int32_t x2 = inputBlock[y8 + 6] * coeffs[y8 + 6];
		int32_t x3 = inputBlock[y8 + 2] * coeffs[y8 + 2];
		int32_t x4 = inputBlock[y8 + 1] * coeffs[y8 + 1];
		int32_t x5 = inputBlock[y8 + 7] * coeffs[y8 + 7];
		int32_t x6 = inputBlock[y8 + 5] * coeffs[y8 + 5];
		int32_t x7 = inputBlock[y8 + 3] * coeffs[y8 + 3];
		
		// If all the AC components are zero, then the IDCT is trivial.
		if (x1 ==0 && x2 == 0 && x3 == 0 && x4 == 0 && x5 == 0 && x6 == 0 && x7 == 0) 
		{
			int32_t dc = (x0 - 128) >> 8; // coefficients[0] << 3
			intermed[y8 + 0] = dc;
			intermed[y8 + 1] = dc;
			intermed[y8 + 2] = dc;
			intermed[y8 + 3] = dc;
			intermed[y8 + 4] = dc;
			intermed[y8 + 5] = dc;
			intermed[y8 + 6] = dc;
			intermed[y8 + 7] = dc;
			continue;
    	}

		// Prescale.
		// Stage 1.
		int32_t x8 = w7 * (x4 + x5);
		x4 = x8 + w1mw7*x4;
		x5 = x8 - w1pw7*x5;
		x8 = w3 * (x6 + x7);
		x6 = x8 - w3mw5*x6;
		x7 = x8 - w3pw5*x7;

		// Stage 2.
		x8 = x0 + x1;
		x0 -= x1;
		x1 = w6 * (x3 + x2);
		x2 = x1 - w2pw6*x2;
		x3 = x1 + w2mw6*x3;
		x1 = x4 + x6;
		x4 -= x6;
		x6 = x5 + x7;
		x5 -= x7;

		// Stage 3.
		x7 = x8 + x3;
		x8 -= x3;
		x3 = x0 + x2;
		x0 -= x2;
		x2 = (r2*(x4+x5) + 128) >> 8;
		x4 = (r2*(x4-x5) + 128) >> 8;

		// Stage 4.
		intermed[y8+0] = (x7 + x1) >> 8;
		intermed[y8+1] = (x3 + x2) >> 8;
		intermed[y8+2] = (x0 + x4) >> 8;
		intermed[y8+3] = (x8 + x6) >> 8;
		intermed[y8+4] = (x8 - x6) >> 8;
		intermed[y8+5] = (x0 - x4) >> 8;
		intermed[y8+6] = (x3 - x2) >> 8;
		intermed[y8+7] = (x7 - x1) >> 8;
  	}

  	/* *************************************************************************** 

	Vertical 1-D IDCT:
    Similar to the horizontal 1-D IDCT case, if all the AC components are zero, 
    then the IDCT is trivial. However, after performing the horizontal 1-D IDCT,
    there are typically non-zero AC components, so we do not bother to check for
    the all-zero case.

    *************************************************************************** */

  	for (int32_t x = 0; x < 8; ++x) 
	{
		// Prescale.
		int32_t y0 = (intermed[8*0+x] << 8) + 8192;
		int32_t y1 = intermed[8*4+x] << 8;
		int32_t y2 = intermed[8*6+x];
		int32_t y3 = intermed[8*2+x];
		int32_t y4 = intermed[8*1+x];
		int32_t y5 = intermed[8*7+x];
		int32_t y6 = intermed[8*5+x];
		int32_t y7 = intermed[8*3+x];

		// Stage 1.
		int32_t y8 = reg<int32_t>(w7*reg<int32_t>(y4+y5)) + 4;
		y4 = (y8 + reg<int32_t>(w1mw7*y4)) >> 3;
		y5 = (y8 - reg<int32_t>(w1pw7*y5)) >> 3;
		y8 = reg<int32_t>(w3*reg<int32_t>(y6+y7)) + 4;
		y6 = (y8 - reg<int32_t>(w3mw5*y6)) >> 3;
		y7 = (y8 - reg<int32_t>(w3pw5*y7)) >> 3;

		// Stage 2.
		y8 = y0 + y1;
		y0 -= y1;
		y1 = reg<int32_t>(w6*reg<int32_t>(y3+y2)) + 4;
		y2 = (y1 - reg<int32_t>(w2pw6*y2)) >> 3;
		y3 = (y1 + reg<int32_t>(w2mw6*y3)) >> 3;
		y1 = y4 + y6;
		y4 -= y6;
		y6 = y5 + y7;
		y5 -= y7;

		// Stage 3.
		y7 = y8 + y3;
		y8 -= y3;
		y3 = y0 + y2;
		y0 -= y2;
		y2 = (reg<int32_t>(r2*reg<int32_t>(y4+y5)) + 128) >> 8;
		y4 = (reg<int32_t>(r2*reg<int32_t>(y4-y5)) + 128) >> 8;

		// Stage 4.
		outputBlock[8*0+x] = (y7 + y1) >> 11;
		outputBlock[8*1+x] = (y3 + y2) >> 11;
		outputBlock[8*2+x] = (y0 + y4) >> 11;
		outputBlock[8*3+x] = (y8 + y6) >> 11;
		outputBlock[8*4+x] = (y8 - y6) >> 11;
		outputBlock[8*5+x] = (y0 - y4) >> 11;
		outputBlock[8*6+x] = (y3 - y2) >> 11;
		outputBlock[8*7+x] = (y7 - y1) >> 11;
	}
}

/* *************************************************************************** 

execute_noflow:

Dataflow block used to manage full block computation. It uses wide
arrays for single block computation to allow efficient access with
ii=2 for the 8x8 data elements. 

*************************************************************************** */
void execute_noflow(hls::stream<int512_t> &iblock, 
		     hls::stream<uint512_t> &icoeffs, 
		     hls::stream<int512_t> &oblock, 
		     bool ignoreDC, 
		     unsigned int numBlocks) 
{
  for(unsigned int i = 0; i < numBlocks; i++) 
  {
    /* Use II=2 here as we this will equalize all the dataflow processes and
     * save resources */
    #pragma HLS loop_tripcount min=1024 max=1024
    
//PIPELINE_PRAGMA:

    #pragma HLS PIPELINE II=2
    
    int16_t iiblock[64];
    uint16_t iiq[64];
    int16_t iioutBlocks[64];

    for(short j = 0; j < 64/32; j++) 
    {
    	if(i==0) 
    	{
			ap_uint<512> tmp;
			tmp = icoeffs.read();
			for(short k = 0; k < 32; k++) 
			{
				iiq[j*32+k] = tmp(16*(k+1)-1, 16*k);
			}
      }
    }

    for(short j = 0; j < 64/32; j++) 
    {
		ap_int<512> tmp;
		tmp = iblock.read();
		for(short k = 0; k < 32; k++) 
		{
			iiblock[j*32+k] = tmp(16*(k+1)-1, 16*k);
		}
    }
    
    idct_noflow(iiblock, iiq, iioutBlocks, ignoreDC);
    
    for(short j = 0; j < 64/32; j++) 
    {
		ap_int<512> tmp;
		for(short k = 0; k < 32; k++)
		{
			tmp(16*(k+1)-1, 16*k) = iioutBlocks[j*32+k];
		}
		oblock.write(tmp);
    }
  }
}


/* *************************************************************************** 

krnl_idct_dataflow_noflow:

Top idct kernel function, used to clearly isolate and identify
dataflow blocks.

*************************************************************************** */
void krnl_idct_dataflow_noflow(const ap_int<512> *inBlocks, 
			const ap_uint<512> *coeffs, 
			ap_int<512> *outBlocks, 
			int ignoreDC, 
			unsigned int blocks) 
{

//DATAFLOW_PRAGMA:

	//#pragma HLS DATAFLOW

	hls::stream<int512_t> iblock;
	hls::stream<uint512_t> icoeffs;
	hls::stream<int512_t> ioutBlocks;

	#pragma  HLS stream variable=iblock depth=1024
	#pragma  HLS stream variable=icoeffs     depth=8
	#pragma  HLS stream variable=ioutBlocks depth=1024

//FUNCTION_PIPELINE:
	read_blocks_noflow<uint512_t>(coeffs, icoeffs, 1);
	read_blocks_noflow<int512_t>(inBlocks, iblock, blocks);
	execute_noflow(iblock, icoeffs, ioutBlocks, ignoreDC ? true : false, blocks);
	write_blocks_noflow(outBlocks, ioutBlocks, blocks);
}


/* *************************************************************************** 

krnl_idct_noflow:

Kernel idct interface definition. 

*************************************************************************** */
extern "C" 
{
	void krnl_idct_noflow( const ap_int<512> *inBlocks,
					const ap_uint<512> *coeffs,
					ap_int<512> *outBlocks,
					int ignoreDC,
					unsigned int numBlocks)
	{
	  	// Interface definition for port "inBlocks"
		#pragma HLS INTERFACE m_axi     port=inBlocks offset=slave bundle=gmem0
		#pragma HLS INTERFACE s_axilite port=inBlocks bundle=control

	  	// Interface definition for port "coeffs"
		#pragma HLS INTERFACE m_axi     port=coeffs offset=slave bundle=gmem1
		#pragma HLS INTERFACE s_axilite port=coeffs bundle=control

	  	// Interface definition for port "outBlocks"
		#pragma HLS INTERFACE m_axi     port=outBlocks offset=slave bundle=gmem2
		#pragma HLS INTERFACE s_axilite port=outBlocks bundle=control

		#pragma HLS INTERFACE s_axilite port=ignoreDC bundle=control
		#pragma HLS INTERFACE s_axilite port=numBlocks bundle=control

		#pragma HLS INTERFACE s_axilite port=return bundle=control

		krnl_idct_dataflow_noflow(inBlocks, coeffs, outBlocks, ignoreDC, numBlocks);
	}

}


