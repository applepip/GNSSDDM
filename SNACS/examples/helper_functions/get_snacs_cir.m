function [ cir ] = get_snacs_cir(f, num)

raw = hdf5read(f, sprintf('/cirs/%i', num));

echoes_amount = numel(raw);
cir(echoes_amount) = struct('delay', {0}, 'weight', {0 + 0 * j});

for k = 1:echoes_amount
    cir_raw = cell2mat(raw(k).Data);
    cir(k).delay = cir_raw(1);
    cir(k).weight = cir_raw(2) + j * cir_raw(3);
end

end