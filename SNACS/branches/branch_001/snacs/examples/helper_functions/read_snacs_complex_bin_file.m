function [ signal ] = read_snacs_complex_bin_file( file, samples, data_type )

disp(sprintf('opening file %s...', file));

fileID = fopen(file, 'r');

if fileID < 0
    error('cannot open file');
end

disp(sprintf('reading %i samples...', samples));
file_samples = samples * 2; % 1 complex sample = 2 doubles on disk!
file_signal = fread(fileID, file_samples, data_type);

signal_r = file_signal(1:2:end); 
signal_i = file_signal(2:2:end);

signal = signal_r + 1i * signal_i;

fclose(fileID);

if size(signal, 1) ~= samples
    error(sprintf('could only read %i from %i samples.', size(signal, 1), samples));
end

end

