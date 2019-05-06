clear
%close all

fs = 30000;
fc1 = 100;
fc2 = 12000;
order = 6;
[b,a] = butter(order,[fc1/(fs/2) fc2/(fs/2)]);
freqz(b,a)
%y1 = filter(b, a, x); % MATLAB double version
%y2 = IIRfilter(b, a, x, 24); % MATLAB fixed point version

%% SOS sections
sos = tf2sos(b,a);
%[sos,g] = tf2sos(b,a)
%[z,p,k] = butter(order, [2*fc1/(fs/2) 2*fc2/(fs/2)]);
%[sos, g] = zp2sos(z,p,k);
%gmean = nthroot(g,size(sos,1))
%sos = zp2sos(z,p,k);
%fvtool(sos,'Analysis','freq')
sos = flipud(sos); % Reverse order of SOS sections with gain compensation as last SOS section

% xin = x;
% for i=1:size(sos,1)
%     yout = IIRfilter(sos(i, 1:3), sos(i, 4:6), xin, 23); 
%     xin = round(yout); 
% end
% 
% figure
% subplot(3,1,1);
% plot(20*log10(abs(fft(y1))))
% title('MATLAB frequency response (double)');
% subplot(3,1,2);
% plot(20*log10(abs(fft(y2))))
% title('MATLAB frequency response (fixed)');
% subplot(3,1,3);
% plot(20*log10(abs(fft(xin))))
% title('MATLAB frequency response (fixed-SOS)');
% 
% figure, 
% plot(y2);
% hold on;
% plot(xin);
% plot(xin-y1);
% title('MATLAB double (blue) vs. MATLAB fixed-SOS');
% rms(xin-y1)

sosArray = [];
for i=1:size(sos,1)
    sosArray = [sosArray sos(i,:)];
end

name = 'IIR12';
SaveFilterHeaderFile(sosArray, 'IIRFilter_coeffs.h');
SaveFilterTxtFile(sosArray, [name '.txt']);
SaveFilterBinFile(32, sosArray, [name '.bin']);
