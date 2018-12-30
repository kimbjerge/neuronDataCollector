close all
% Number of samples to load
samples = 30000*60; % 60 seconds at fs=30kHz
% Template length and width
tempLength = 16;
tempWidth = 8;
tempOffset = 7;
% Channel to filter and plot
chPlot = 16;

sdCardPath = 'D:\';

%% Load 32 channel binary neuron samples and filtered data from ZedBoard
% Compares results with similar MATLAB FIR filtering
orgSignal = loadFile(sdCardPath, 'DATA.BIN', [32 samples], 'float');
%figure, surf(orgSignal);
%title('Original signal 32 channels');
%ylabel('Channels');
%xlabel('Samples');

filteredSignal = loadFile(sdCardPath, 'FIRFILT.BIN', [32 samples], 'int32');
%figure, surf(filteredSignal);
%title('Filtered signal 32 channels');
%ylabel('Channels');
%xlabel('Samples');
figure, hold off, plot(orgSignal(chPlot,:)), hold on, plot(filteredSignal(chPlot,:));
title(['Original (blue) vs. Filtered (red) signal channel ' num2str(chPlot)]);

filteredSignalCh = filteredSignal(chPlot, :);
filteredSignalCh(1:end-1) = filteredSignalCh(2:end); % 1 sample delay
b = load([sdCardPath 'FIR.txt']);
filteredSignalGolden = filter(b, 1, orgSignal(chPlot,:));
diffSignal = filteredSignalGolden(1:end-1) - filteredSignalCh(1:end-1);
figure, plot(diffSignal); 
title(['Difference of MATLAB vs. Filtered signal from channel ' num2str(chPlot)]);

%% Load templates
template1 = loadFile(sdCardPath, 'T11_01.BIN', [tempWidth tempLength], 'float');
figure, surf(template1);
ylabel('Channels');
xlabel('Samples');
title('Template 1, T11\_01.BIN');

template2 = loadFile(sdCardPath, 'T38_01.BIN', [tempWidth tempLength], 'float');
figure, surf(template2);
title('Template 2, T38\_ 01.BIN');
ylabel('Channels');
xlabel('Samples');

template3 = loadFile(sdCardPath, 'T01_04.BIN', [tempWidth tempLength], 'float');
figure, surf(template3);
title('Template 3, T01\_ 04.BIN');
ylabel('Channels');
xlabel('Samples');

template4 = loadFile(sdCardPath, 'T27_14.BIN', [tempWidth tempLength], 'float');
figure, surf(template4);
title('Template 4, T27\_ 24.BIN');
ylabel('Channels');
xlabel('Samples');

%% Compare normalized cross correlation with MATLAB based on filtered data
signalSearchTemplate = filteredSignal(2:tempWidth+1,:);
%signalSearchTemplate = orgSignal(2:tempWidth+1,:);

nxcorrT1gold = nxcor(template1, signalSearchTemplate);
nxcorrT1 = loadFile(sdCardPath, 'NXCORT1.BIN', samples, 'float');
figure, hold off, plot(nxcorrT1gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT1(2:end), 'r'); %1 sample delay due to pipeline
title('NXCORR template T1 (red) vs. MATLAB (black)');

nxcorrT2gold = nxcor(template2, signalSearchTemplate);
nxcorrT2 = loadFile(sdCardPath, 'NXCORT2.BIN', samples, 'float');
figure, hold off, plot(nxcorrT2gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT2(2:end), 'r'); %1 sample delay due to pipeline
title('NXCORR template T2 (red) vs. MATLAB (black)');

signalSearchTemplate = filteredSignal(5:tempWidth+4,:);

nxcorrT3gold = nxcor(template3, signalSearchTemplate);
nxcorrT3 = loadFile(sdCardPath, 'NXCORT3.BIN', samples, 'float');
figure, hold off, plot(nxcorrT3gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT3(2:end), 'r'); %1 sample delay due to pipeline
title('NXCORR template T3 (red) vs. MATLAB (black)');

signalSearchTemplate = filteredSignal(15:tempWidth+14,:);

nxcorrT4gold = nxcor(template4, signalSearchTemplate);
nxcorrT4 = loadFile(sdCardPath, 'NXCORT4.BIN', samples, 'float');
figure, hold off, plot(nxcorrT4gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT4(2:end), 'r'); %1 sample delay due to pipeline
title('NXCORR template T4 (red) vs. MATLAB (black)');

