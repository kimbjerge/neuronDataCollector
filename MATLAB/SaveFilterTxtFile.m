function [ ] = SaveFilterTxtFile(  y, fileName  )
% Save coefficeints to fileName
    fid = fopen(fileName, 'w');
    for i=1:length(y)
        fprintf(fid, '%1.15f\r\n', y(i));
    end
    fclose(fid);
end

