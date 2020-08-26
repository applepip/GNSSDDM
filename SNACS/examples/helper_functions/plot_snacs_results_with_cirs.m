function plot_snacs_results_with_cirs(CIRFileName, files, legend_str, Int, plotting)

% CIRs file: CIRFileName

% SNACS result files: files = {'../Results/result30.h5', '...', ...};

% descriptions for SNACS result files: legend_str = { 'urban channel, 50 dB-Hz', 'DLL result' };

% more parameters:
Len = 5; % in seconds
CorrTime = 0.001;

% read CIR parameters from file:
sim_params.c0 = hdf5read(CIRFileName, '/parameters/simulation/c0');
sim_params.cir_rate = hdf5read(CIRFileName, '/parameters/simulation/cir_rate');
sim_params.cir_amount = hdf5read(CIRFileName, '/parameters/simulation/cir_amount');

disp('channel model simulation parameters:');
sim_params

initEpochs = ceil(Len / CorrTime);

num = size(files,1);

% get PR results from result files
disp('reading simulation results...');
PRs = zeros(initEpochs, num);
for i = 1:num
   res_file_name = cell2struct(files(i), 'files' , 1);
   prf = hdf5read(res_file_name.files, '/snSDRGPS/pr');
   PRs(:, i) = prf(1:initEpochs);
end

if Len > sim_params.cir_amount / sim_params.cir_rate  % do we have enough CIRs in file?
    error('Not enough cirs in file.');
    return;
end

if sim_params.cir_amount / sim_params.cir_rate > initEpochs * CorrTime % do we have more CIRs in file than SDR results?
    sim_params.cir_amount = initEpochs * CorrTime * sim_params.cir_rate; % trim CIR amount to available PRs
    disp(sprintf('CIR amount is bigger than SRE Epochs. CIR amount trimmed to %i', sim_params.cir_amount));
end

disp(sprintf('processing %i CIRs. CIR rate:  %.2f Hz', sim_params.cir_amount, sim_params.cir_rate));

disp('reading reference ranges...');
ReferencePR_increments = hdf5read(CIRFileName, '/reference_range/range_increments');
ReferencePR = zeros(sim_params.cir_amount, 1);
range = 0;
for i = 1:length(ReferencePR)
    range = range + ReferencePR_increments(i);
    ReferencePR(i) = range;
end

% determine remaining parameters for interpolation:
Int.Smples = Int.MaxLen * Int.Freq;
Int.Steps = sim_params.cir_amount / Int.SkipFac;
if Int.Steps ~= floor(Int.Steps)
   error(' sim_params.cir_amount / Int.SkipFac is not integer ');
end

% Int.Steps = floor(Int.Steps);

interp_cir_file = [ CIRFileName, '-interp.h5' ];

if exist(interp_cir_file, 'file') 
    % check if interpolation of simulation parameters were changed:
    int_file = hdf5read(interp_cir_file, '/interpolation_parameters');
    sim_file = hdf5read(interp_cir_file, '/simulation_parameters');

    if int_file ~= Int || sim_file ~= sim_params
        create_intcir = 1;
    else
        create_intcir = 0;
    end
%         create_intcir = 0;
else
        create_intcir = 1;
end
    
if create_intcir
    % create interpolated CIRs:
    disp('reading cirs and doing interpolation...');
    [ IntResult, dt, dd ] = interpolate_cirs(CIRFileName, Int, sim_params, ReferencePR);
    disp('writing file with interpolation result to disk...');
    hdf5write(interp_cir_file, '/interpolation_parameters', Int);
    hdf5write(interp_cir_file, '/simulation_parameters', sim_params, 'WriteMode', 'append');
    hdf5write(interp_cir_file, '/interpolation_result', IntResult, 'WriteMode', 'append');
    hdf5write(interp_cir_file, '/time_axis', dt, 'WriteMode', 'append');
    hdf5write(interp_cir_file, '/distance_axis', dd, 'WriteMode', 'append');
else
    disp('loading file with interpolated cirs...');
    IntResult = hdf5read(interp_cir_file, '/interpolation_result');
    dt = hdf5read(interp_cir_file, '/time_axis');
    dd = hdf5read(interp_cir_file, '/distance_axis');
end
disp('done.');


% calculate reference PR
RefPRplot = zeros(1, Len * sim_params.cir_rate / Int.SkipFac);
for i=1:length(RefPRplot)
    RefPRplot(i) = ReferencePR(i * Int.SkipFac) + Int.MinOffset * sim_params.c0; % in m
end

% calculate mean:

range_diffs = zeros(sim_params.cir_amount, num);
means = zeros(num, 1);
for i = 1:num
   range_diffs(:, i) = ReferencePR - PRs(:, i);
   means(i) = mean(range_diffs(:, i));
   % TODO calc rms
end



disp('plotting...');

figure(2);
surf(dt, dd, 20*log10(abs(IntResult)));
hold on;
title([ 'CIRs, reference pseudorange and DLL output']);
%view(2);
caxis([-60, 0]);
h=gcf;
hchild = get(h,'Children');
hchild1 = get(hchild(1),'Children');
set(hchild1,'LineStyle','none');
% set(hchild1,'FaceColor','interp');
set(hchild1,'EdgeLighting','phong');
view(2);
c=colormap('gray'); d=c(end:-1:1,:); colormap(d);
colorbar;
xlabel('time [s]');
ylabel('delay [m]');

plot(dt, RefPRplot, 'g','LineWidth', 1.5);

PRplot = zeros(initEpochs, num);
x_vec = 0:CorrTime:Len;
x_vec = x_vec(1:end-1);

for o=1:num
    for i=1:length(PRplot)
        PRplot(i, o) = PRs(i, o) + Int.MinOffset * sim_params.c0; % in m
    end
    switch o
        case 1
            col = 'b';
        case 2
            col = 'c';
        case 3
            col = 'r';
        case 4
            col = 'm';
    end
    plot(x_vec, PRplot(:, o), col, 'LineWidth', 1.5);
    hold on;
end
legend(legend_str);

disp('saving plot...');
set(gcf,'paperunits', 'inches');
set(gcf,'paperpos',[0 0 8 6]);
set(gcf,'PaperSize',[8 6]);
print('-r300','-dpdf', [ 'rural-fig2', '.pdf' ]);
% % print('-r300','-djpeg', 'Figure10.jpg');
saveas(gcf, [ 'rural-fig2', '.fig' ]);
disp('done.');

end