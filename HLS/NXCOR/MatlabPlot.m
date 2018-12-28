load ('nxcorr.dat');
res = nxcorr(:,2);
varSignal = nxcorr(:,3);
xcor = nxcorr(:,4);
figure, plot(res);
title('XCOR template and signal');
figure, plot(varSignal);
title('Variance of signal');
figure, plot(xcor);
title('NXCOR template and signal');

