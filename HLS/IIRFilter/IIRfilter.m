function [y] = IIRfilter(b,a,x,bits)
% IIR filter implementaion like done in HLS

tabs = length(b);
xin = zeros(size(b));
yout = zeros(size(a));
y = zeros(size(x));
if bits > 0
    an = round(a*2^bits);
    bn = round(b*2^bits);
else
    an = a;
    bn = b;
end
for i=1:length(x)

    for k=0:tabs-2
        j = tabs-k;
        xin(j) = xin(j-1);
        yout(j) = yout(j-1);
    end
    
   xin(1) = x(i);
   yout(1) = 0;
   
   ysum = 0;
   for j=1:tabs
       ysum = ysum + floor(bn(j)*xin(j));
       prod = an(j)*yout(j);
       ysum = ysum - round(prod);
   end
   
   result = ysum/2^bits;
   yout(1) = result;
        
   y(i) = round(result);
end
