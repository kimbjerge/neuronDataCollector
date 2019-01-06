function [ ] = SaveFilterHeaderFile( y, fileName )
% Save coefficeints to fileName
    format long;
    fid = fopen(fileName, 'w');
    %fprintf(fid, 'const double BW%d[FIR_TAPS] = {\r\n', fc);
    fprintf(fid, 'const double FIR_coeffs[FIR_TAPS] = {\r\n');
    for i=1:length(y)
        if (i < length(y))
            fprintf(fid, '%1.15f,', y(i));
            if (mod(i,3) == 0)
              fprintf(fid, '\r\n');
            end 
        else
            fprintf(fid, '%1.15f\r\n', y(i));
        end
    end
    fprintf(fid, '};\r\n');
    fclose(fid);
end

