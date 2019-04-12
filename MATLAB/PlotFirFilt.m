close all
% Number of samples to load
samples = 30000*30; % 60 seconds at fs=30kHz
%samples = 3000; % 1 seconds at fs=30kHz
% Channel to filter and plot
chPlot = 16;

sdCardPath = '';

%% Load 32 channel binary neuron samples and filtered data from ZedBoard
% Compares results with similar MATLAB FIR filtering


rawSignal = loadFile(sdCardPath, 'RAWDATA.BIN', [32 samples], 'int16');
filteredSignal = loadFile(sdCardPath, 'FIRFILT.BIN', [32 samples], 'int16');
figure, 
plot(rawSignal(chPlot,end-20000:end));
hold on
plot(filteredSignal(chPlot,end-20000:end));
