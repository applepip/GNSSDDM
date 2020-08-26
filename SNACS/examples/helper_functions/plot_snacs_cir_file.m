function [ reference_ranges, delay_axis, sim_params ] = plot_snacs_cir_file(cir_file_name, interp, plotting)

%% parameters:

%% let's start

% read CIR parameters from file:
disp('reading CIR data file...');
sim_params.c0 = hdf5read(cir_file_name, '/parameters/simulation/c0');
sim_params.cir_rate = hdf5read(cir_file_name, '/parameters/simulation/cir_rate');
sim_params.cir_amount = hdf5read(cir_file_name, '/parameters/simulation/cir_amount');

if sim_params.cir_amount < plotting.length * sim_params.cir_rate
    error('plotting.length too long. not enough CIRs!');
else
    if sim_params.cir_amount > plotting.length * sim_params.cir_rate
        sim_params.cir_amount = plotting.length * sim_params.cir_rate;
    end
end

disp('reading reference ranges...');
reference_ranges = hdf5read(cir_file_name, '/reference_range/range_absolut');
% time axis for reference range:
xax_ref_range = 0:1/sim_params.cir_rate:plotting.length-1/sim_params.cir_rate;

% determine remaining parameters for interpolation:
interp.samples_amount = round(2*interp.max_length * interp.bandwidth);
interp.steps = sim_params.cir_amount / interp.skip_amount;
if interp.steps ~= floor(interp.steps)
    error(' sim_params.cir_amount / interp.skip_amount is not integer ');
end

disp('interpolation...');
[ interpolation_result, time_axis, delay_axis ] = interpolate_cirs(cir_file_name, interp, sim_params, reference_ranges);

% since we added offset for better plotting, we need to correct
% delay_axis:
delay_axis = delay_axis - interp.add_delay_offset;

%% plotting
disp('plotting...');
f1 = figure(1);
hold on;
% plot CIRs:
s1 = surf(delay_axis/1e-9, time_axis, 20*log10(abs(interpolation_result)'));
set(s1,'LineStyle','none');
set(s1,'EdgeLighting','phong');

% plot reference ranges:
% p1 = plot(xax_ref_range, reference_ranges - plotting.add_offset, 'm-', 'LineWidth', plotting.line_width);
% if plotting.show_title; title('Interpolated CIRs and reference range', 'FontSize', plotting.font_size); end;

ylabel('time/s', 'FontSize', plotting.font_size);
xlabel('delay/ns', 'FontSize', plotting.font_size);
zlabel('power/dB', 'FontSize', plotting.font_size);
ylim([ time_axis(1) time_axis(end) ])
xlim([ delay_axis(1)/1e-9 delay_axis(end)/1e-9 ]);

view(2);
caxis(plotting.range_dB);
c1 = colorbar;
ylabel(c1, 'dB');

disp('done.');

end