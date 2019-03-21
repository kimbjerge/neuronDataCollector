function [command] = createTemplateCmd(template, nr)

 % Create command to update template with data
 command = sprintf('s,d,%d,%d,%d', nr, size(template,1), size(template,2)); 

 for l=1:size(template,2)
    for ch=1:size(template,1)
        floatStr = num2str(template(ch,l));
        command = [command ',' floatStr];
    end
 end