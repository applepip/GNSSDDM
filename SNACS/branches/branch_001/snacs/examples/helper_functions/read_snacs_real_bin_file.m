function [ signal ] = read_snacs_bin_file( file, samples, data_type )

disp(sprintf('opening file %s...', file));

fileID = fopen(file, 'r');

if fileID < 0
    error('cannot open file');
end

disp(sprintf('reading %i samples...', samples));
signal = fread(fileID, samples, data_type);

fclose(fileID);

if size(signal, 1) ~= samples
    error(sprintf('could only read %i from %i samples', size(signal, 1), samples));
end

end

