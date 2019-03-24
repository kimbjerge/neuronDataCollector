function [reply] = SendFloatData(t, data)
% Sends sample data as floats/single to neuron processor
    time = 0.02;
    blockSize = 8;
    data = single(data);
    sizeData = size(data,2);
    for i=1:blockSize:sizeData;
        dataFloat = single([]);
        for j=0:(blockSize-1)
            if (i+j > sizeData)
                dataFloat = [dataFloat; single(zeros(size(data,1),1))];
            else
                dataFloat = [dataFloat; single(data(:,i+j))];
            end
        end
        write(t, dataFloat);
        pause(time);
        reply = char(read(t))
    end

end

