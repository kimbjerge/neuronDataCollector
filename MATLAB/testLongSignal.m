close all
%samples = 30000*60;
samples = 30000;
tempLength = 16;
tempWidth = 8;
chPlot = 16;

sdCardPath = 'D:\';

%orgSignal = loadFile(sdCardPath, 'rawData4x32x30000x60.BIN', [32 samples], 'float');
orgSignal = loadFile(sdCardPath, 'DATA.BIN', [32 samples], 'float');
%figure(1), surf(orgSignal);
title('Original signal 32 channels');
ylabel('Channels');
xlabel('Samples');
figure(3), hold off, plot(orgSignal(chPlot,:));
title(['Original signal channel' num2str(chPlot)]);

template1 = loadFile(sdCardPath, 'T11_01.BIN', [tempWidth tempLength], 'float');
template1 = template1*2^15;
figure, surf(template1);
ylabel('Channels');
xlabel('Samples');
title('Template 1, T11\_01.BIN');

template2 = loadFile(sdCardPath, 'T38_01.BIN', [tempWidth tempLength], 'float');
template2 = template2*2^15;
figure, surf(template2);
title('Template 2, T38\_ 01.BIN');
ylabel('Channels');
xlabel('Samples');

signalSearchTemplate = orgSignal(2:tempWidth+1,:);

nxcorrT1gold = normxcorr2(template1, signalSearchTemplate);
nxcorrT1 = loadFile(sdCardPath, 'NXCORT1.BIN', samples, 'float');
figure, hold off, plot(nxcorrT1gold(8,7:samples+7)); % hold on, plot(nxcorrT1);
title('NXCORR template T1 (red) vs. golden (blue)');

nxcorrT2gold = normxcorr2(template2, signalSearchTemplate);
nxcorrT2 = loadFile(sdCardPath, 'NXCORT2.BIN', samples, 'float');
figure, hold off, plot(nxcorrT2gold(8,7:samples+7)); % hold on, plot(nxcorrT2);
title('NXCORR template T2 (red) vs. golden (blue)');
