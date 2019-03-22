function [ reply ] = SendCmd ( t, cmd )
%Writes a command to pulse detector and returns answer

time = 0.1;
cmd = uint8(cmd);
write(t, cmd);
pause(time);
reply = char(read(t));

end

