clear
close all;
length = 60; % 60 FIR taps
channels = 32; 
port = 7;

sdCardPath = ''; % Final data
firData = loadFile(sdCardPath, 'FIR.bin', [length channels], 'float');
t = tcpclient('192.168.1.10', port, 'Timeout', 3);
%
reply = SendCmd(t, sprintf('f,u,FIR.bin,%d', length*channels*4))   % Upload FIR binary coefficients
reply = SendFloatData(t, firData)