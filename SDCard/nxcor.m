function [ result ] = nxcor( template, signal )
%Performs normalized cross correlation 
%   Flip template to perfrom cross correlation instead of convolution

% Don't flip template version 1.4
templateFixed = round(template*2^15);
%templateFixed = round(fliplr(template)*2^15); % Done in version 1.3
result = normxcorr2(templateFixed, signal);

end

