function [ result ] = nxcor( template, signal )
%Performs normalized cross correlation 
%   Flip template to perfrom cross correlation instead of convolution

templateFixed = round(fliplr(template)*2^15);
result = normxcorr2(templateFixed, signal);

end

