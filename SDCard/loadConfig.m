function [ templates ] = loadConfig( name )

template.name = '';
template.width = 0;
template.length = 0;
template.threshold = 0.0;
template.count = 0;
template.max = 0;
template.min = 0;
template.offset = 0;
template.cfgName = 0;

[fileID, meassage] = fopen(name, 'r');
if isempty(meassage)
    numTemplates = 0;
    readOK = 2;
    while (readOK > 1) 
        tline = fgetl(fileID);
        readOK = length(tline);
        if (readOK > 1 && tline(1) ~= '%')
            numTemplates = numTemplates + 1;
            dataLine = sscanf(tline, '%s %d %d %f %d %d %d %d %s');
            template.name = char(dataLine(1:12))';
            template.width = dataLine(13);
            template.length = dataLine(14);
            template.threshold = dataLine(15);
            template.count = dataLine(16);
            template.max = dataLine(17);
            template.min = dataLine(18);
            template.offset = dataLine(19);
            template.cfgName = char(dataLine(20:end))';
            templates(numTemplates) = template;
        end
    end
    fclose(fileID);
else
    printf('Error reading configuration: %s ', name);
end

end

