#pragma once
#include <iostream>
#include <chrono>
#include <atomic>
#include "types.hpp"


#define POLLING_INTERVAL 1
#define PRINT_UPDATE_INTERVAL 50
#define INTERVAL_SCALING_FACTOR (PRINT_UPDATE_INTERVAL/POLLING_INTERVAL)
// Macros to print information message header and footer

#define printHeader() std::cout<<"=====================================================================\n";
#define printFooter() std::cout<<"=====================================================================\n";

/* ***************************************************************************

measureExecTimes:

Utility function measures the end time for two threads executing in parallel
using atomic done flags that will be toggled by each thread when it completes.
Also prints a visual timeline that displays the time taken by FPGA and CPU

    isEmulationModeEnabled     : flag implies if the run is emulation run (sw/hw)
    cpuDone                    : atomic flag set by cpu thread when done
    fpgaDone                   : atomic flag set by FPAG thread when it is done
    cpuEndTime                 : Measurement that gives the end time for cpu thread
    fpgaEndTime                : Measurement that gives the end time for fpga thread

*************************************************************************** */
void measureExecTimes(  bool isEmulationModeEnabled,
                        std::atomic<bool> &cpuDone,
                        std::atomic<bool> &fpgaDone, 
                        std::chrono::high_resolution_clock::time_point &cpuEndTime,
                        std::chrono::high_resolution_clock::time_point &fpgaEndTime
                      )
{
    printHeader();
    std::cout<<std::endl;
    bool fpgaEndTimeNoted = false;
    bool cpuEndTimeNoted = false;
    int cpuTimeSpentBarLen = 0;
    int fpgaTimeSpentBarLen = 0;
    int dynamicScale = 1;
    std::string tickString("#");
    std::string timeUnit("ms");
    if(isEmulationModeEnabled)
        timeUnit = "s";
    while( !fpgaDone || !cpuDone)
    {
    	if(fpgaDone && !fpgaEndTimeNoted)
    	{
    		fpgaEndTime = std::chrono::high_resolution_clock::now();
    		fpgaEndTimeNoted = true;
    	}
    	if(cpuDone && !cpuEndTimeNoted)
    	{
    		cpuEndTime = std::chrono::high_resolution_clock::now();
    		cpuEndTimeNoted = true;
    	}
        if(!isEmulationModeEnabled)
    	    std::this_thread::sleep_for(std::chrono::milliseconds(POLLING_INTERVAL));
        else
    	    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // check the bar lengths and dynamically calculate scaling
            int fpgaPrintBarLen = fpgaTimeSpentBarLen / (INTERVAL_SCALING_FACTOR*dynamicScale);
            int cpuPrintBarLen  = cpuTimeSpentBarLen  / (INTERVAL_SCALING_FACTOR*dynamicScale);
            if( (cpuPrintBarLen + fpgaPrintBarLen) > 120 )
            {
                dynamicScale = dynamicScale * 2;
                for(int temp=0;temp<250;temp++)
                    std::cout<<" ";
                std::cout<<"\r"<<std::flush;    

            }
        /* 
           **************************************************************
			
			fpgaTimeBar : 
							Create time bar proportional to FPGA
							execution time

 		   **************************************************************
        */
    	if(!fpgaEndTimeNoted)
    	 fpgaTimeSpentBarLen++;

        std::cout<<"[FPGA Time( "<<std::setw(5)<<fpgaTimeSpentBarLen*POLLING_INTERVAL<<""<<timeUnit<< " ) :";
        if(!isEmulationModeEnabled)
        {
            for(int i=0; i < fpgaTimeSpentBarLen/(INTERVAL_SCALING_FACTOR*dynamicScale); i++)
            {
                std::cout<<tickString;
            }
        }
        if(fpgaEndTimeNoted)
            std::cout<<" Done.";
        std::cout<<" ]  ";
        /* 
           **************************************************************
			
		   cpuTimeBar :
							Create time bar proportional to CPU
							execution time
							
 		   **************************************************************
        */

    	if(!cpuEndTimeNoted)
    		cpuTimeSpentBarLen++;
        std::cout<<"[CPU Time("<<std::setw(5)<<cpuTimeSpentBarLen*POLLING_INTERVAL<<""<<timeUnit<< " ) :";
        if(!isEmulationModeEnabled)
        {
            for(int i=0; i < cpuTimeSpentBarLen/(INTERVAL_SCALING_FACTOR * dynamicScale); i++)
            {
                std::cout<<tickString;
            }
        }
        if(cpuEndTimeNoted)
            std::cout<<" Done.";
        std::cout<<" ]  ";
        std::cout<<std::flush;
        std::cout<<"\r"<<std::flush;
        }

    if(!fpgaEndTimeNoted)
	{
		// if cpu finished first
		fpgaEndTime = std::chrono::high_resolution_clock::now();
	}
	if(!cpuEndTimeNoted)
	{
		// if fpga finished first
		cpuEndTime = std::chrono::high_resolution_clock::now();
	}
	std::cout<<std::endl<<std::flush;
    std::cout<<std::endl;
    std::cout<<"Execution Finished"<<std::endl;
    printFooter();
}



