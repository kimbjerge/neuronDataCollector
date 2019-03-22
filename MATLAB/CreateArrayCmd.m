function [command] = CreateArrayCmd(cmd, nr, array)
    command = [sprintf('s,%s,%d,', cmd, nr) num2str(array,'%d,')];
end

