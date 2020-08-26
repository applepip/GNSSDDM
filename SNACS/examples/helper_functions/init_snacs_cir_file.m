function init_snacs_cir_file(file, parameters)

dset_details.Location = '/parameters';
dset_details.Name = 'generation_time';

time = clock; % [year month day hour minute seconds]

dset = sprintf('%02d-%02d-%02d %02d:%02d:%2.2f', time(1), time(2), time(3), time(4), time(5), time(6));

hdf5write(file, dset_details, dset);

% hdf5write('compound_test1.h5', dset_details, cir, ...
%            attr_details, attr, 'WriteMode', 'append');

hdf5write(file, '/parameters/simulation/c0' , parameters.c0, 'WriteMode', 'append');
hdf5write(file, '/parameters/simulation/cir_rate' , parameters.cir_rate, 'WriteMode', 'append');
hdf5write(file, '/parameters/simulation/cir_amount' , parameters.cir_amount, 'WriteMode', 'append');

end