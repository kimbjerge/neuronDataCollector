
samples = 30000;

sdCardPath = 'D:\'
filteredResultFile  = [sdCardPath 'FIRFILT.BIN'];
[fileID meassage] = fopen(filteredResultFile, 'r');
if length(meassage) == 0
    filteredSignal = fread(fileID, [32 samples], 'int32');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure, surf(filteredSignal);
figure, plot(filteredSignal(1,:));

fileNxcorrTemplate1 = [sdCardPath 'NXCORT1.BIN'];
[fileID meassage] = fopen(fileNxcorrTemplate1, 'r');
if length(meassage) == 0
    nxcorrT1 = fread(fileID, samples, 'float');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure, plot(nxcorrT1);

fileNxcorrTemplate2 = [sdCardPath 'NXCORT2.BIN'];
[fileID meassage] = fopen(fileNxcorrTemplate2, 'r');
if length(meassage) == 0
    nxcorrT2 = fread(fileID, samples, 'float');
else
    fprintf('Error reading!\n');
end
fclose('all');
figure, plot(nxcorrT2);
