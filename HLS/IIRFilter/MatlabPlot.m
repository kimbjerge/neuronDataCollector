clear 
close all
load ('eq_impulse.dat');
h = eq_impulse(:,3);
%h = h/max(h);
x = eq_impulse(:,2);
%x = x/max(x);
plot(x);
hold on
plot(h);
figure,
freqz(h);

%%
fs = 30000;

% % Notch filter
% fp = 5000;  % Pole frequency 0.1*fs (forstærkning)
% rp = 0.1; % Pole radius
% 
% fn = 5000; % Zero frequency 0.2*fs (dæmpning)
% rn = 1; % Zero radius
% 
% wp = 2*pi*fp/fs;
% wn = 2*pi*fn/fs;
% 
% % Cofficients numerator 
% b0 = 1;
% b1 = -2*rn*cos(wn);
% b2 = rn^2;
% b = [b0 b1 b2];
% 
% % Coefficients denominator
% a0 = 1;
% a1 = -2*rp*cos(wp);
% a2 = rp^2;
% a = [a0 a1 a2];

fc1 = 300;
fc2 = 6000;
order = 6;
[b,a] = butter(order,[fc1/(fs/2) fc2/(fs/2)]);

freqz(b,a)
y1 = filter(b, a, x);
% figure
% subplot(2,1,1);
% plot(20*log10(abs(fft(y1))))
% 
y2 = IIRfilter(b, a, x, 31);
% subplot(2,1,2);
% plot(20*log10(abs(fft(y2))))

%figure, plot(y2-y1);

%% SOS sections
[z,p,k] = butter(order, [fc1/(fs/2) fc2/(fs/2)]);
[sos, g] = zp2sos(z,p,k);
%sos = zp2sos(z,p,k);
%fvtool(sos,'Analysis','freq')

xin = x*g;
for i=1:size(sos,1)
    yout = IIRfilter(sos(i, 1:3), sos(i, 4:6), xin, 17); %20 bits
    xin = round(yout); 
end
figure
subplot(3,1,1);
%plot(xin);
plot(20*log10(abs(fft(y1))))
subplot(3,1,2);
%plot(xin);
plot(20*log10(abs(fft(y2))))
subplot(3,1,3);
plot(20*log10(abs(fft(xin))))

figure, 
plot(y1);
hold on;
plot(xin);
plot(y1-xin);
rms(y1-xin)

%% Results
figure
subplot(2,1,1);
plot(20*log10(abs(fft(x))))
subplot(2,1,2);
plot(20*log10(abs(fft(h))))

figure, 
plot(h);
hold on;
plot(xin);


