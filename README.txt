TODO:
9-3-2019
1. HPP_SfN_15_2 -> test TemplateMatch0 ext menu - not tested
2. HPP_NeuronD_172 -> test HPPIO.cpp called from main
   - order of call to SetPSReady(PS_Ctrl); -> moved??-ok
   - calling SetPSControl(Logic_Ctrl); in main_full??-ok
   - alternatively calling SetDIOAllValues(u32)-ok port 2+3
   - alternatively calling SetDIOAllValuesNoAck(u32)-ok port 2+3
3. Mail to Digital Lynx about problem
14-3-2019
- Tested and found that port 0 and 1 is not working - locks on ack semaphore - interrupts missing

===========================================================================
Template Matching running on ZedBoard and Digital Lynx HPP
All source files in bitbucket located in:
neuronDataCollector (All Vivado projects includes source files relative)


First Vivado 2017.2 versions for ZedBoard:
-----------------------------------------
FIRFilter - solution1 (HLS)
NXCOR - solution1-2 (HLS)
ZedNeuronA - ZedNeuronC

Final Vivado 2017.2 version for ZedBoard:
-----------------------------------------
FIRFilter - solution2 (HLS)
NXCOR - solution3 (HLS)
ZedNeuronD - version 1.3 - 1.5 release


Final Vivado 2018.2.1 version for HPP not working:
-------------------------------------------------
HPP_SfN - Orginial HPP code from Digital Lynx
HPP_SfN_15_2 - First tested HPP version with network - transmitting UDP neuron data to PC
             - Used Vivado 2015.2 for hardware and Vivado 2017.2 SDK for software
HPP_SfN_18_2 - Same ported to 18.2.1

FIRFilter - solution2 (HLS 2015.2)
NXCOR - solution3 (HLS 2015.2)
HPP_NeuronD_182 - version 1.6 release with HLS IP Cores 
     - Tested 31-1-2019 - not working stuck in HPPDataGenerator -> InitHPPDataGenerator(4);
     - Most likely due to port 0-1 TTL outputs not working


Final Vivado 2015.2 version for HPP (SDK version 207.2):
--------------------------------------------------------
HPP_SfN - Orginial HPP code from Digital Lynx
HPP_SfN_15_2 - First tested HPP version with network - transmitting UDP neuron data to PC
             - Used Vivado 2015.2 for hardware and Vivado 2017.2 SDK for software

FIRFilter - solution2 (HLS 2015.2)
NXCOR - solution3 (HLS 2015.2)
HPP_NeuronD - Ready for testing with HLS cores
             - Used Vivado 2015.2 for hardware and Vivado 2017.2 SDK for software


Other HLS core with Vivado 2015.2:
---------------------------------------
FIRFilter15 - solution1 (HLS)
NXCOR15 - solution1 (HLS) - Better perfomance than version


Vivado 2017.2 tested (marts 2019)
---------------------------------
- HPP_NeuronD (Working 8-3-2019) (100 MHz)
    - Vivado 2015.2 (HW) and IP Cores FIRFilter15 and NXCOR15
    - SDK 2017.2 
    - Tested ok
    
- HPP_NeuronD_172 (Working 14-3-2019)
   - IP Cores FIRFilter + NXCORE 2017.2 (125 MHz)
   - Without interrupts connected
   - Ethernet working
   - Reading from SD Card working
   - Working sending data to PC
   - Digital outputs working

- HPP_NeuronD_172_B
   - IP Cores FIRFilter + NXCORE 2017.2 (100 MHz)
   - Ethernet working
   - Reading from SD Card working
   - Duration of 80 sec. processing 60 sec. data
   - Interrupts missing from Lynx

- HPP_NeuronD_172_A
    - Sames as B, without interrupts
    - Not tested

- UltraNeuronD (Vivado 2018.2.1)
    - Tested version 2.0 on Ultra96 
    - Performs 29 sec on 60 sec data with 6 templates
    - Compared to ZedBoard of 48 sec. speedup 1.66 times

Vivado 2017.2 tested (may 2019)
---------------------------------
- HPP_NeuronE (Working 20-5-2019) 
- HPP_NeuronE_172 (Working 20-5-2019)
     - Same as D but FIR filter replaced with IIR filter (6xSOS)
     - Version 3.3 verified working in real mouse brain


Vivado 2017.2 tested (june 2019)
---------------------------------
- HPP_NeuronF (Working 28-6-2019) 
- HPP_NeuronF_172 (Working 28-6-2019)
     - Same as F but NXCOR changed to 25x5 max templates
     - Version 4.0 with coherency and min/max diff peaks

