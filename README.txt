
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


Final Vivado 2018.2.1 version for HPP:
-----------------------------------------
HPP_SfN - Orginial HPP code from Digital Lynx
HPP_SfN_15_2 - First tested HPP version with network - transmitting UDP neuron data to PC
             - Used Vivado 2015.2 for hardware and Vivado 2017.2 SDK for software
HPP_SfN_18_2 - Same ported to 18.2.1

FIRFilter - solution2 (HLS 2017.2)
NXCOR - solution3 (HLS 2017.2)
HPP_NeuronD_182 - version 1.6 release with HLS IP Cores


Vivado versions that failes for HPP:
-----------------------------------------
HPP_NeuronD_152_err - problem with ngc2edif.exe loading DLL
HPP_SfN_17_2_err - problem with ngc2edif.exe loading DLL


Other HLS core with Vivado 2015.2 (not tested):
---------------------------------------
FIRFilter15 - solution1 (HLS)
NXCOR15 - solution1 (HLS) - Better perfomance than version

