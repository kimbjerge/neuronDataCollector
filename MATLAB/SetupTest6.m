clear;
close all;
%% This Script connects to the real-time neuron detector version 1.7 
% Duration of experiment in seconds
durationSec = 10; 
% Sending 6 templates
numTemplates = 6;
% Template names
tempName = {'11011608.bin'; '38011608.bin'; '01041608.bin'; ...
            'TMP09_06.bin'; 'TMP61_13.bin'; '27141608.bin'};
% Templates length and width 
tempLength = [16 16 16 17 17 16]; % Max. 17 samples in length
tempWidth = [8 8 8 9 9 8]; % Max. 9 channels
% Template NXCOR thresholds to detect neuron activation
threshold = [0.69, 0.66, 0.61, 0.62, 0.63, 0.64];
% Template min. gradients, measured as 4 samples offset relative to peak 
gradient = [1, 2, 1, 2, 1, 2];
% Template channel mappings
mapping = [1,2,3,4,5,6,7,8,9;
           1,2,3,4,5,6,7,8,9;
           4,5,6,7,8,9,10,11,12;
           6,7,8,9,10,11,12,13,14;
           13,14,15,16,17,18,19,20,21;
           14,15,16,17,18,19,20,21,22];
% Template peak max. limits for each mapped channel
peakMaxLimits = [10030,11030,12030,13030,14030,15030,16030,17030,18030;
                 10031,11031,12031,13031,14031,15031,16031,17031,18031;
                 10032,11032,12032,13032,14032,15032,16032,17032,18032;
                 10033,11033,12033,13033,14033,15033,16033,17033,18033;
                 10034,11034,12034,13034,14034,15034,16034,17034,18034;
                 10035,11035,12035,13035,14035,15035,16035,17035,18035];
% Template peak min. limits for each mapped channel
peakMinLimits = [-10040,-11040,-12040,-13040,-14040,-15040,-16040,-17040,-18040;
                 -10041,-11041,-12041,-13041,-14041,-15041,-16041,-17041,-18041;
                 -10042,-11042,-12042,-13042,-14042,-15042,-16042,-17042,-18042;
                 -10043,-11043,-12043,-13043,-14043,-15043,-16043,-17043,-18043;
                 -10044,-11044,-12044,-13044,-14044,-15044,-16044,-17044,-18044;
                 -10045,-11045,-12045,-13045,-14045,-15045,-16045,-17045,-18045];

% Loads test templates
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

pause(2);

% Sends setup commands for using test templates, use correct DATA.BIN file
port = 7;
t = tcpclient('192.168.1.10', port, 'Timeout', 3);
reply = SendCmd(t, 'g,v')            % Read version number, expected ver. 1.7
reply = SendCmd(t, 's,p,2')          % Neuron data from SD card (2) or HPP (1) 
reply = SendCmd(t, sprintf('s,n,%d', numTemplates)) % Set number of templates to use

for tempNr=1:numTemplates
    template = templates{tempNr};
    cmdTemplate = CreateTemplateCmd(template, tempNr); % Create command to update template
    reply = SendCmd(t, cmdTemplate) % Sets template data
    reply = SendCmd(t, sprintf('s,t,%d,%f', tempNr, threshold(tempNr))) % Set template thredshold 
    reply = SendCmd(t, sprintf('s,g,%d,%d', tempNr, gradient(tempNr))) % Set template gradient 
    cmdMap = CreateArrayCmd('m', tempNr, mapping(tempNr,:)); % Create mapping command
    reply = SendCmd(t, cmdMap) % Set template channel map
    cmdPeakMax = CreateArrayCmd('h', tempNr, peakMaxLimits(tempNr,:));
    reply = SendCmd(t, cmdPeakMax) % Set template peak max. limits
    cmdPeakMin = CreateArrayCmd('l', tempNr, peakMinLimits(tempNr,:));
    reply = SendCmd(t, cmdPeakMin) % Set template peak min. limits
end

reply = SendCmd(t, 'g,c');         % Reads configuration
pause();                           % Check that the configuration in USB terminal window

reply = SendCmd(t, sprintf('s,e,%d', durationSec));
reply = SendCmd(t, 'b');           % Set to begin processing neuron samples

%pause();
%reply = SendCmd(t, 'e');           % Set to terminate processing samples

