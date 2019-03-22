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
reply = SendCmd(t, 'g,v')          % Read version number, expected ver. 1.7
reply = SendCmd(t, 's,p,2')        % Neuron data from SD card (2) or HPP (1)
reply = SendCmd(t, sprintf('s,n,%d', numTemplates)) % Set number of templates to use

reply = SendCmd(t, cmdTemplate1)   % Sets template data
reply = SendCmd(t, sprintf('s,t,%d,%f', tempNr, threshold)) % Set template thredshold 
reply = SendCmd(t, sprintf('s,g,%d,%d', tempNr, gradient)) % Set template gradient 
reply = SendCmd(t, sprintf('s,m,%d,26,27,30,31,', tempNr)) % Set template channel map
reply = SendCmd(t, sprintf('s,h,%d,-38,-70,-49,-27,', tempNr)) % Set template peak max. limits
reply = SendCmd(t, sprintf('s,l,%d,-214,-233,-221,-194,', tempNr)) % Set template peak min. limits

reply = SendCmd(t, 'g,c');         % Reads configuration
pause();                            % Check that the configuration in USB terminal window

reply = SendCmd(t, 'b');           % Set to begin processing neuron samples

%pause();
%reply = SendCmd(t, 'e');           % Set to terminate processing samples

