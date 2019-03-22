samples = 30000*10; % 60 seconds at fs=30kHz

sdCardPath = '..\..\SDCardHPP\';

%% Load 32 channel binary neuron samples and filtered data from ZedBoard
% Compares results with similar MATLAB FIR filtering
orgSignal = loadFile(sdCardPath, 'DATA.bin', [32 samples], 'float');

t = tcpclient('192.168.1.10', port, 'Timeout', 3);
reply = SendCmd(t, 'g,v')            % Read version number, expected ver. 1.7