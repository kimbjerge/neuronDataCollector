close all
% Number of samples to load
samples = 30000*60; % 60 seconds at fs=30kHz
% Template length and width
tempLength = 17;
tempWidth = 9;
% Channel to filter and plot
chPlot = 16;

sdCardPath = 'F:\';

%% Load 32 channel binary neuron samples and filtered data from ZedBoard
% Compares results with similar MATLAB FIR filtering
orgSignal = loadFile(sdCardPath, 'DATA.bin', [32 samples], 'float');
%figure, surf(orgSignal);
%title('Original signal 32 channels');
%ylabel('Channels');
%xlabel('Samples');

filteredSignal = loadFile(sdCardPath, 'FIRFILT.BIN', [32 samples], 'int16');
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

%% Load templates must correspond contents of CONFIG.txt file
template1 = loadFile(sdCardPath, 'T11_01.BIN', [tempWidth tempLength], 'float');
figure, surf(template1);
ylabel('Channels');
xlabel('Samples');
title('Template 1, T11\_01.BIN');

template2 = loadFile(sdCardPath, 'T38_01.BIN', [tempWidth tempLength], 'float');
figure, surf(template2);
title('Template 2, T38\_01.BIN');
ylabel('Channels');
xlabel('Samples');

template3 = loadFile(sdCardPath, 'T01_04.BIN', [tempWidth tempLength], 'float');
figure, surf(template3);
title('Template 3, T01\_04.BIN');
ylabel('Channels');
xlabel('Samples');

template4 = loadFile(sdCardPath, 'T09_06.BIN', [tempWidth tempLength], 'float');
figure, surf(template4);
title('Template 4, T09\_06.BIN');
ylabel('Channels');
xlabel('Samples');

template5 = loadFile(sdCardPath, 'T61_13.BIN', [tempWidth tempLength], 'float');
figure, surf(template5);
title('Template 5, T61\_13.BIN');
ylabel('Channels');
xlabel('Samples');

template6 = loadFile(sdCardPath, 'T27_14.BIN', [tempWidth tempLength], 'float');
figure, surf(template6);
title('Template 6, T27\_14.BIN');
ylabel('Channels');
xlabel('Samples');

templates = {template1, template2, template3, template4, template5, template6};

%% Compare normalized cross correlation with MATLAB based on filtered data
config = importdata('CONFIG.txt');
numTemplates = size(config.data, 1);
for i=1:numTemplates
    offset = config.data(i, 5);
    signalSearchTemplate = filteredSignal(offset+1:tempWidth+offset,:);
    %signalSearchTemplate = orgSignal(offset+1:tempWidth+offset,:);  
    template = templates{i};
    nxcorrTgold = nxcor(template, signalSearchTemplate);
    nxcorrName = ['NXCORT' num2str(i) '.BIN'];
    nxcorrT = loadFile(sdCardPath, nxcorrName, samples, 'float');
    figure, hold off, plot(nxcorrTgold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT(2:end), 'r'); %1 sample delay due to pipeline
    title(['NXCORR template T' num2str(i) ' (red) vs. MATLAB (black)']);
end
