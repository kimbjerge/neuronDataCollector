function [command] = CreateTemplateCmd(template, nr)
% Create command to update template with data width/channels and length
 command = sprintf('s,d,%d,%d,%d,', nr, size(template,1), size(template,2)); 

 for l=1:size(template,2) % Length of template
     samples = num2str(template(:,l)', '%.5f,');
     command = [command samples];
 end