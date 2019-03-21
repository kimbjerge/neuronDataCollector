%% This Script connects to the real-time neuron detector version 1.7 
% Sending 6 templates
numTemplates = 6;
% Templates length and width 
tempWidth = [8 8 8 9 9 8]; % Max. 9
tempLength = [16 16 16 17 17 16]; % Max. 17
% Template names
tempName = {'11011608.bin'; '38011608.bin'; '01041608.bin'; ...
            'TMP09_06.bin'; 'TMP61_13.bin'; '27141608.bin'};
threshold = [0.69, 0.66, 0.61, 0.62, 0.63, 0.64];
gradient = [1, 2, 1, 2, 1, 2];
mapping = [1,2,3,4,5,6,7,8,9;
           1,2,3,4,5,6,7,8,9;
           4,5,6,7,8,9,10,11,12;
           6,7,8,9,10,11,12,13,14;
           13,14,15,16,17,18,19,20,21;
           14,15,16,17,18,19,20,21,22];

Path = '..\SDCard\';
templates = cell(numTemplates,1);
for i=1:numTemplates
    template = loadFile(Path, tempName{i}, [tempWidth(i) tempLength(i)], 'float');
    figure, surf(template);
    ylabel('Channels');
    xlabel('Samples');
    title(['Template ' num2str(i) ' file '  tempName{i}]);
    templates{i} = template;
end

pause();

% sends setup commands for using pulse test generator
port = 7;
t = tcpclient('192.168.1.10', port, 'Timeout', 3);
reply = PulseCmd(t, 'g,v')          % Read version number, expected ver. 1.7
reply = PulseCmd(t, 's,p,1')          % Neuron data from SD card (1) or HPP (0)
reply = PulseCmd(t, sprintf('s,n,%d', numTemplates)) % Set number of templates to use

for tempNr=1:numTemplates
    cmdTemplate = CreateTemplateCmd(template1, tempNr); % Create command to update template
    reply = PulseCmd(t, cmdTemplate)   % Sets template data
    reply = PulseCmd(t, sprintf('s,t,%d,%f', tempNr, threshold(tempNr))) % Set template thredshold 
    reply = PulseCmd(t, sprintf('s,g,%d,%d', tempNr, gradient(tempNr))) % Set template gradient 
    cmdMap = [sprintf('s,m,%d,', tempNr) num2str(mapping(tempNr,:),'%d,')];
    reply = PulseCmd(t, cmdMap) % Set template channel map
    reply = PulseCmd(t, sprintf('s,h,%d,10033,11033,12033,13033,14033,15033,16033,17033,18033', tempNr)) % Set template peak max. limits
    reply = PulseCmd(t, sprintf('s,l,%d,-10044,-11044,-12044,-13044,-14044,-15044,-16044,-17044,-18044', tempNr)) % Set template peak min. limits
end

num2str
reply = PulseCmd(t, 'g,c');         % Reads configuration
pause();                            % Check that the configuration in USB terminal window

reply = PulseCmd(t, 'b');           % Set to begin processing neuron samples

pause();
reply = PulseCmd(t, 'e');           % Set to terminate processing samples

