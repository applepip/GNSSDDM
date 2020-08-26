function append_one_cir_to_cir_file(file, cir_number, cir)

hdf5write(file, sprintf('/cirs/%i', cir_number), cir, 'WriteMode', 'append');

end