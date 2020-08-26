function [ Result, time_axis, delay_axis ] = interpolate_cirs(CIRFileName, interp, sim_params, reference_ranges)

Result = zeros(interp.samples_amount, interp.steps);

fsmpl = 2 * interp.bandwidth;
Ts = 1/fsmpl;
Om = 2*pi*interp.bandwidth;

% should we move the CIR to delay = 0?
add_absolute_delay = zeros(sim_params.cir_amount, 1);
if interp.move_to_delay0 == true
        add_absolute_delay = reference_ranges / sim_params.c0;
end

add_absolute_delay = add_absolute_delay + interp.add_delay_offset;

wb = waitbar(0, 'CIR interpolation...');
for i = 1:sim_params.cir_amount
    % interpolate
    if mod(i, interp.skip_amount) == 0
        waitbar(i/sim_params.cir_amount);
        [ cir ] = get_snacs_cir(CIRFileName, i-1);  % i-1: cirs start with 0
        indx = i/interp.skip_amount;
        for o = 1:numel(cir)
            for k = 0:interp.samples_amount-1
                fac = Om * (k * Ts - (cir(o).delay + add_absolute_delay(i)));
                if fac == 0
                    Result(k+1, indx) = Result(k+1, indx) + 1;
                else
                    Result(k+1, indx) = Result(k+1, indx) + cir(o).weight * sin(fac)/fac;
                end
            end
        end
    end
end
close(wb);
time_axis = 0:1/sim_params.cir_rate*interp.skip_amount:sim_params.cir_amount / sim_params.cir_rate;
time_axis = time_axis(1:end-1);

delay_axis = (0:interp.samples_amount-1) / fsmpl;

end
