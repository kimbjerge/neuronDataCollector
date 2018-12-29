close all
samples = 30000*60;
tempLength = 16;
tempWidth = 8;
tempOffset = 7;
chPlot = 16;

sdCardPath = 'D:\';

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
title(['Original (blue) vs. Filtered (red) signal channel' num2str(chPlot)]);

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


signalSearchTemplate = filteredSignal(2:tempWidth+1,:);
%signalSearchTemplate = orgSignal(2:tempWidth+1,:);

template1 = round(template1*2^15);
nxcorrT1gold = normxcorr2(template1, signalSearchTemplate);
nxcorrT1 = loadFile(sdCardPath, 'NXCORT1.BIN', samples, 'float');
figure, hold off, plot(nxcorrT1gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT1(2:end), 'r'); %1 sample delay due to pipeline
%plot(nxcorrT1/(max(nxcorrT1)*2));
title('NXCORR template T1 (red) vs. golden (blue)');

template2 = round(template2*2^15);
nxcorrT2gold = normxcorr2(template2, signalSearchTemplate);
nxcorrT2 = loadFile(sdCardPath, 'NXCORT2.BIN', samples, 'float');
figure, hold off, plot(nxcorrT2gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT2(2:end), 'r'); %1 sample delay due to pipeline
%plot(-nxcorrT2/(max(nxcorrT2)*2));
title('NXCORR template T2 (red) vs. golden (blue)');

signalSearchTemplate = filteredSignal(5:tempWidth+4,:);

template3 = round(template3*2^15);
nxcorrT3gold = normxcorr2(template3, signalSearchTemplate);
nxcorrT3 = loadFile(sdCardPath, 'NXCORT3.BIN', samples, 'float');
figure, hold off, plot(nxcorrT3gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT3(2:end), 'r'); %1 sample delay due to pipeline
%plot(nxcorrT3/(max(nxcorrT3)*2));
title('NXCORR template T3 (red) vs. golden (blue)');

signalSearchTemplate = filteredSignal(15:tempWidth+14,:);

template4 = round(template4*2^15);
nxcorrT4gold = normxcorr2(template4, signalSearchTemplate);
nxcorrT4 = loadFile(sdCardPath, 'NXCORT4.BIN', samples, 'float');
figure, hold off, plot(nxcorrT4gold(tempWidth,1:end-1), 'k'), hold on, plot(nxcorrT4(2:end), 'r'); %1 sample delay due to pipeline
%plot(-nxcorrT4/(max(nxcorrT4)*2));
title('NXCORR template T4 (red) vs. golden (blue)');

