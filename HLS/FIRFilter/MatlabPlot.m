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
