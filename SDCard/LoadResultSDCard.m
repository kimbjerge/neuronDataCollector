close all
% Number of samples to load
%samples = 30000*60; % 60 seconds at fs=30kHz
samples = 3000; % 60 seconds at fs=30kHz
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
templatesConfig = loadConfig('CONFIG.txt');
numTemplates = length(templatesConfig);
templates = cell(numTemplates,1);
for i=1:numTemplates
    template = loadFile(sdCardPath, templatesConfig(i).name, [templatesConfig(i).width templatesConfig(i).length], 'float');
    figure, surf(template);
    ylabel('Channels');
    xlabel('Samples');
    title(['Template ' num2str(i) ' file ' templatesConfig(i).name]);
    templates{i} = template;
end

pause(1);

%% Compare normalized cross correlation with MATLAB based on filtered data
for i=1:numTemplates
    offset = templatesConfig(i).offset;
    width = templatesConfig(i).width;
    %signalSearchTemplate = filteredSignal(offset+1:width+offset,:);
    %
    signalSearchTemplate = filteredSignal([27,28,31,32],:);
    %signalSearchTemplate = orgSignal(offset+1:tempWidth+offset,:);  
    template = templates{i};
    nxcorrTgold = nxcor(template, signalSearchTemplate);
    nxcorrName = ['NXCORT' num2str(i) '.BIN'];
    nxcorrT = loadFile(sdCardPath, nxcorrName, samples, 'float');
    figure, hold off, plot(nxcorrTgold(width,1:end-1), 'k'), hold on, plot(nxcorrT(2:end), 'r'); %1 sample delay due to pipeline
    title(['NXCORR template T' num2str(i) ' (red) vs. MATLAB (black)']);
end
