close all
samples = 10000;

sdCardPath = 'D:\'
sampleFile  = [sdCardPath 'DATA.BIN'];
[fileID meassage] = fopen(sampleFile, 'r');
if length(meassage) == 0
    orgSignal = fread(fileID, [32 samples], 'float');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure(1), surf(orgSignal);
title('Original signal 32 channels');
figure(3), hold off, plot(orgSignal(1,:));
title('Original signal channel 1');

filteredResultFile  = [sdCardPath 'FIRFILT.BIN'];
[fileID meassage] = fopen(filteredResultFile, 'r');
if length(meassage) == 0
    filteredSignal = fread(fileID, [32 samples], 'int32');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure(2), surf(filteredSignal);
title('Filtered signal 32 channels');
figure(3), hold on, plot(filteredSignal(1,:));
title('Filtered signal channel 1');

fileNxcorrTemplate1 = [sdCardPath 'NXCORT1.BIN'];
[fileID meassage] = fopen(fileNxcorrTemplate1, 'r');
if length(meassage) == 0
    nxcorrT1 = fread(fileID, samples, 'float');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure, plot(nxcorrT1);
title('NXCORR template T1');

fileNxcorrTemplate2 = [sdCardPath 'NXCORT2.BIN'];
[fileID meassage] = fopen(fileNxcorrTemplate2, 'r');
if length(meassage) == 0
    nxcorrT2 = fread(fileID, samples, 'float');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure, plot(nxcorrT2);
title('NXCORR template T2');
