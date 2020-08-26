%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% This script plots an CDX (HDF5 CIR) file.
%
% Author: F. Schubert
% Date: 05-05-2011
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% the most interesting parameters are probably:
% plot_params.length = 1; % length to plot in seconds
% interp_params.skip_step = 1; % only every skip_step-th CIR is plotted

% if waitbars are still open from a previous program interruption, call:
% set(0,'ShowHiddenHandles','on')
% delete(get(0,'Children'))

clear all
close all
clear classes
clc

addpath('../../../../../cdx_format/CDX_library_functions');

%% set file parameters:
cdx_file = CDXReadFile('/tmp/snacs-example03/interpolated-channel.h5');
link = 1; % plot the first link in file

%% set plotting parameters:
plot_params.start = 0; % s
plot_params.length = 10; % length to plot in seconds
plot_params.title = '';
plot_params.legend_str = { 'channel impulse responses', 'true delay' };
plot_params.colormap = 'default'; % 'default' or 'gray'
plot_params.dB_range = [-50, 0];
plot_params.reference_delay_color = 'm';
plot_params.reference_delay_linewidth = 3;
plot_params.font_size = 14;
plot_params.save_format = [ 8 6 ];
plot_params.save_fig = false;
plot_params.save_pdf = true;
plot_params.shading = 'flat'; % 'flat', 'faceted', or 'interp'. Legend not supported for 'interp'.
plot_params.move_reference_delay_to_zero = false;
plot_params.enable_windowing = false;
plot_params.adjust_delay = false;
    plot_params.plot_delay_before_los = 1e-6;
    plot_params.plot_delay_after_los = 1e-6;
    
% cir_rate = get_cir_rate(cdx_file);
% if plot_params.length * cir_rate > 2000
%     disp('This will plot more than 2000 CIRs. Are you sure? Press Ctrl-C to abort or any other key to proceed.');
%     pause
% end

[ plot_data, dt, dd, reference_delays ] = plot_discrete_delay_cir_file(cdx_file, link, plot_params);
