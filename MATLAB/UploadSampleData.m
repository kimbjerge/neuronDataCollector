clear
close all;
samples = 30000*10; % 60 seconds at fs=30kHz
channels = 32;
port = 7;

sdCardPath = '..\..\SDCardHPP\'; % Final data
%sdCardPath = '..\..\SDCardD\'; % Test data
orgSignal = loadFile(sdCardPath, 'DATA.bin', [channels samples], 'float');

t = tcpclient('192.168.1.10', port, 'Timeout', 3);
%reply = SendCmd(t, sprintf('s,u,%d', samples*channels))            % Upload data to HPP
reply = SendCmd(t, sprintf('f,u,KBETEST.bin,%d', samples*channels*4))            % Upload data to SD Card
reply = SendFloatData(t, orgSignal)