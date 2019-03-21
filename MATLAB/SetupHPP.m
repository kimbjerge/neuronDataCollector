%% This Script connects to the real-time neuron detector version 1.7 
% Template length and width 
tempLength = 10; % Max. 17
tempWidth = 4; % Max. 9
% Template name
tempName = '11011608.bin';
tempNr = 1;
threshold = 0.90;
gradient = 25;
% Sending only one template
numtemplates = 1;

Path = '..\..\SDCardHPP\';
template1 = loadFile(Path, tempName, [tempWidth tempLength], 'float');
figure, surf(template1);
ylabel('Channels');
xlabel('Samples');
title(['Template 1 - ' tempName]);
cmdTemplate1 = CreateTemplateCmd(template1, tempNr); % Create command to update template

pause();

% sends setup commands for using pulse test generator
port = 7;
t = tcpclient('192.168.1.10', port, 'Timeout', 3);
reply = PulseCmd(t, 'g,v')          % Read version number, expected ver. 1.7
reply = PulseCmd(t, 's,p,1')          % Neuron data from SD card (1) or HPP (0)
reply = PulseCmd(t, sprintf('s,n,%d', numTemplates)) % Set number of templates to use

reply = PulseCmd(t, cmdTemplate1)   % Sets template data
reply = PulseCmd(t, sprintf('s,t,%d,%f', tempNr, threshold)) % Set template thredshold 
reply = PulseCmd(t, sprintf('s,g,%d,%d', tempNr, gradient)) % Set template gradient 
reply = PulseCmd(t, sprintf('s,m,%d,26,27,30,31', tempNr)) % Set template channel map
reply = PulseCmd(t, sprintf('s,h,%d,-38,-70,-49,-27', tempNr)) % Set template peak max. limits
reply = PulseCmd(t, sprintf('s,l,%d,-214,-233,-221,-194', tempNr)) % Set template peak min. limits

reply = PulseCmd(t, 'g,c');         % Reads configuration
pause();                            % Check that the configuration in USB terminal window

reply = PulseCmd(t, 'b');           % Set to begin processing neuron samples

pause();
reply = PulseCmd(t, 'e');           % Set to terminate processing samples

