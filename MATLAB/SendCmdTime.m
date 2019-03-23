function [ reply ] = SendCmdTime ( t, cmd, time )
%Writes a command to neuron processor and returns answer
% Waits for answer within time
    cmd = uint8([cmd '\n']);
    write(t, cmd);
    pause(time);
    reply = char(read(t));
end

