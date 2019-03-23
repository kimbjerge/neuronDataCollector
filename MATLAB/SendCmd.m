function [ reply ] = SendCmd ( t, cmd )
%Writes a command to neuron processor and returns answer
    time = 0.1;
    cmd = uint8([cmd '\n']);
    write(t, cmd);
    pause(time);
    reply = char(read(t));
end

