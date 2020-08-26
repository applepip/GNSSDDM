%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 
% This file saves a channel impulse response series as HDF5 file
% 
% based on
% LandMobileMultipathChannel_Demo_UrbanCar.m
% Version 3.0
% distributed by http://www.kn-s.dlr.de/satnav
%
% adapted by Frank Schubert, 2010
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

close all
clear all
clear classes
clc

addpath('../helper_functions');

% channel model start:
pth = '../LandMobileMultipathChannelModel30';
addpath(pth);
addpath ([pth,'/Knife_Edge']);
addpath ([pth,'/Parameter_Files']);

Parameters.SampFreq=500;        % Hz
Parameters.MaximumSpeed=50;     % km/h
Parameters.SatElevation=30;     % Deg
Parameters.SatAzimut=-135;       % Deg (North == 0, East == 90, South == 180, West == 270)
Parameters.NumberOfSteps=10000; % 5000 / 1000 = 1 second
Co=2.99e8; % Speed of Light

%%for SNACS, initialize SNACS parameters sec
% write Parameters and ChannelParams to h5 file:
snacs_cir_filename = 'snacs-cir-file_DLRLMS_example-05-2.h5';
% [for SNACS] map the channel model parameter names to the SNACS CIR parameter names:
simulation_parameters.c0 = Co;
simulation_parameters.cir_rate = Parameters.SampFreq;

% write only-LOS signal at the beginning of file.
% length 1 second. 1 s * cir_rate = number of only-LOS cirs:
only_los_offset = 1 * simulation_parameters.cir_rate; 

simulation_parameters.cir_amount = only_los_offset + Parameters.NumberOfSteps;

% [for SNACS] init the SNACS CIR HDF5 file:
init_snacs_cir_file(snacs_cir_filename, simulation_parameters);

% [for SNACS] no channel for the first second to let the loops lock.
% write one second of a A = 1 exp(j*0) component at delay = 0 s to the CIR
% file:
only_los_cir = struct('delay', {0}, 'real', {0}, 'imag', {0});
only_los_cir.delay = 0;
only_los_cir.real = 1;
only_los_cir.imag = 0;
% initialize vectors for the reference distance from transmitter 
% to receiver
ActualPR_vec = zeros(simulation_parameters.cir_amount, 1);
disp('writing only-LOS CIR snapshots...');
for k = 0:only_los_offset-1
    append_one_cir_to_cir_file(snacs_cir_filename, k, only_los_cir);
end
% the next written CIR will start with number: only_los_offset

%% channel model continue...
FontSize=12;

% ---- General Parameters ----

ChannelParams.CarrierFreq=1.57542e9;     % Hz
ChannelParams.SampFreq=Parameters.SampFreq;
ChannelParams.EnableDisplay=0;           % 3D visualization is not available in the free version
ChannelParams.EnableCIRDisplay=0;        % enables CIR display

% ---- Mode Parameters ----

ChannelParams.UserType = 'Car';
ChannelParams.Surrounding = 'Urban';
ChannelParams.AntennaHeight = 2;         % m Height of the Antenna
ChannelParams.MinimalPowerdB=-40;        % Echos below this Limit are not initialised

% ---- UserParameters ---

ChannelParams.DistanceFromRoadMiddle=-5; % negative: continental (right), positive: England (left)

% ---- Graphics Parameters ---           

ChannelParams.GraphicalPlotArea=50;      % 
ChannelParams.ViewVector = [-60,20];     % 3D visualization is not available in the free version
ChannelParams.RoadWidth = 15;            %

% --- Building Params ---

ChannelParams.BuildingRow1=1;            % logigal to switch Building Row right(heading 0 deg) on
ChannelParams.BuildingRow2=1;            % logigal to switch Building Row left (heading 0 deg) on
ChannelParams.BuildingRow1YPosition=-12; % m
ChannelParams.BuildingRow2YPosition=12;  % m

ChannelParams.HouseWidthMean=22;         % m
ChannelParams.HouseWidthSigma=25;        % m
ChannelParams.HouseWidthMin=10;          % m
ChannelParams.HouseHeightMin=4;          % m
ChannelParams.HouseHeightMax=50;         % m
ChannelParams.HouseHeightMean=16;        % m
ChannelParams.HouseHeightSigma=6.4;      % m
ChannelParams.GapWidthMean=27;           % m
ChannelParams.GapWidthSigma=25;          % m
ChannelParams.GapWidthMin=10;            % m
ChannelParams.BuildingGapLikelihood=0.18;% lin Value

% --- Tree Params ---

ChannelParams.TreeHeight = 8;            % m
ChannelParams.TreeDiameter = 5;          % m
ChannelParams.TreeTrunkLength=2;         % m
ChannelParams.TreeTrunkDiameter=.2;      % m

ChannelParams.TreeAttenuation = 1.1;     % dB/m

ChannelParams.TreeRow1Use=1;             % logical switches tree row 1 on
ChannelParams.TreeRow2Use=1;             % logical switches tree row 2 on

ChannelParams.TreeRow1YPosition=-8;      % m
ChannelParams.TreeRow2YPosition=8;       % m

ChannelParams.TreeRow1YSigma=2;          % m
ChannelParams.TreeRow2YSigma=2;          % m

ChannelParams.TreeRow1MeanDistance=60;   % m
ChannelParams.TreeRow2MeanDistance=40;   % m

ChannelParams.TreeRow1DistanceSigma=20;  % m
ChannelParams.TreeRow2DistanceSigma=20;  % m

% --- Pole Params ---

ChannelParams.PoleHeight = 10;           % m
ChannelParams.PoleDiameter = .2;         % m

ChannelParams.PoleRow1Use=1;             % logical switches Pole row 1 on
ChannelParams.PoleRow2Use=0;             % logical switches Pole row 2 on

ChannelParams.PoleRow1YPosition=0;       % m
ChannelParams.PoleRow2YPosition=10;      % m

ChannelParams.PoleRow1YSigma=1;          % m
ChannelParams.PoleRow2YSigma=1;          % m

ChannelParams.PoleRow1MeanDistance=25;   % m
ChannelParams.PoleRow2MeanDistance=10;   % m

ChannelParams.PoleRow1DistanceSigma=10;  % m
ChannelParams.PoleRow2DistanceSigma=10;  % m

%% for SNACS, write channel scenery parameters 
% write channel scenery parameters to HDF5 file:
hdf5write(snacs_cir_filename, '/parameters/model/channel_scenery' , ChannelParams, 'WriteMode', 'append');

%% channel model continue...

% ------------ Initial Settings -------------
% - Anything Below here must not be changed -
% -------------------------------------------

MaximumPossibleSpeed=Co*Parameters.SampFreq/ChannelParams.CarrierFreq/2; % To fulfill the sampling Theorem
SamplingTime=1/Parameters.SampFreq;

% --- Initialising the channel object ---

pause(1)
disp('Initialising the channel ...')
TheChannelObject=LandMobileMultipathChannel(ChannelParams);

TimeVec=0;
ComplexOutputVec=[];

% --- Specify power and delay bins for output statistics ---

pwrvec = [0:-1:-30];            % power bins in dB
dlyvec = [0:10e-9:500e-9];      % delay bins in s

PowerDelayProfile(1:length(pwrvec),1:length(dlyvec)) = 0;   % allocate memory
pwrstp = (pwrvec(end)-pwrvec(1))/(length(pwrvec)-1);        % get step size
dlystp = (dlyvec(end)-dlyvec(1))/(length(dlyvec)-1);        % get step size

% --- start simulation ---

h = waitbar(0,'Simulation running ...');

if ChannelParams.EnableCIRDisplay
    
    % --- init CIR figure ---

    hh = figure;
    subplot(211)
    xlabel('Delay in s')
    ylabel('Power in dB')
    axis([-2e-8,40e-8,0,50])
    set(get(hh,'Children'),'YTickLabel',[-40 -30 -20 -10 0 10]);
    hold on
    grid on
    plot (0,0,'r')
    plot (0,0)
    legend ('Direct paths','Echo paths')
    
    subplot(212)
    xlabel('Delay in s')
    ylabel('Phase in rad')
    axis([-2e-8,40e-8,-pi,pi])
    hold on
    grid on

end

ActualPR = 0;

for dhv=1:Parameters.NumberOfSteps

    TimeVec(end)=dhv/Parameters.SampFreq;

    % --- "drunken" driver movement example ---
    
    ActualSpeed=Parameters.MaximumSpeed/2/3.6*(1+sin(TimeVec(end)/3));   
    SpeedVec(dhv)=ActualSpeed;              % m/s
    ActualHeading=20*sin(TimeVec(end)/3);   % Deg (North == 0, East == 90, South == 180, West == 270)
    
    % --- generate CIR ---

    [TheChannelObject,LOS,LOSDelays,ComplexOutputVec,DelayVec,EchoNumberVec,WayVec(dhv),TimeVec(dhv),ActualPR_change]=generate(TheChannelObject,ActualSpeed,ActualHeading,Parameters.SatElevation,Parameters.SatAzimut);

    waitbar(dhv/Parameters.NumberOfSteps,h)
    
    % --- binning LOS ---
    
    for sfg = 1:length(LOSDelays)
        
        dlybin = round(LOSDelays(sfg)/dlystp) + 1;
        pwrbin = round(20*log10(abs(LOS(sfg)))/pwrstp) + 1;
        
        if pwrbin<=length(pwrvec) & pwrbin>0 & dlybin<=length(dlyvec)
            PowerDelayProfile(pwrbin,dlybin) = PowerDelayProfile(pwrbin,dlybin) + 1;
        end
        
    end  
    
    % --- binning echoes ---
    
    for sfg = 1:length(DelayVec)
        
        dlybin = round(DelayVec(sfg)/dlystp) + 1;
        pwrbin = round(20*log10(abs(ComplexOutputVec(sfg)))/pwrstp) + 1;
        
        if pwrbin<=length(pwrvec) & pwrbin>0 & dlybin<=length(dlyvec)
            PowerDelayProfile(pwrbin,dlybin) = PowerDelayProfile(pwrbin,dlybin) + 1;
        end
        
    end  
    
    if ChannelParams.EnableCIRDisplay
        
        % --- display CIR ---

        figure(hh);
        subplot(211)
        cla
        Time = dhv/Parameters.SampFreq;
        title(['Channel Impulse Response (CIR), T = ',num2str(Time,'%5.2f'),' s, v = ',num2str(ActualSpeed*3.6,'%4.1f'),' km/h'])
        stem(LOSDelays,40 + 20*log10(abs(LOS)),'r');
        stem(DelayVec,40 + 20*log10(abs(ComplexOutputVec)));
        
        subplot(212)
        cla
        stem(LOSDelays,angle(LOS),'r');
        stem(DelayVec,angle(ComplexOutputVec));
        
    end

    %% [for SNACS], write cir to h5 file
    % For that we build one vector from
    % LOS and echoes first:
    % (dhv is the number of the actual CIR snapshot (starting with 1))
    clear echoes
    ActualPR = ActualPR + ActualPR_change;
    ActualLOSDelay = ActualPR / Co;
    echoes_elements = numel(LOSDelays)+numel(DelayVec); % amount of echoes
    echo_zeros = zeros(echoes_elements, 1);
    echoes(1:echoes_elements) = struct('delay', {0}, 'real', {0}, 'imag', {0});
    for k = 1:numel(LOSDelays)
        echoes(k).delay = LOSDelays(k) + ActualLOSDelay;
        echoes(k).real = real(LOS(k));
        echoes(k).imag = imag(LOS(k));
    end
    idx_offset = numel(LOSDelays);
    for k = 1:numel(DelayVec)
        echoes(k+idx_offset).delay = DelayVec(k) + ActualLOSDelay;
        echoes(k+idx_offset).real = real(ComplexOutputVec(k));
        echoes(k+idx_offset).imag = imag(ComplexOutputVec(k));
    end
    cir_number = only_los_offset + dhv-1; % dhv-1: CIR number for SNACS should start at 0
    
    % only_los_offset is added because we added 1 second of a only los
    % component to the CIR file, see above.
    append_one_cir_to_cir_file(snacs_cir_filename, cir_number, echoes);
    
    ActualPR_vec(cir_number + 1) = ActualPR; % +1 since Matlab vectors start at 1

    disp(sprintf('written step #: %d, actual range: %f\n', cir_number, ActualPR));

    %% channel model continue...

end%

%% [for SNACS], write all PR values
hdf5write(snacs_cir_filename, '/reference_range/range_absolut', ActualPR_vec, 'WriteMode', 'append');

%% channel model continue...

close(h);

% --- calculate probability density function ---

PowerDelayProfile = PowerDelayProfile/sum(sum(PowerDelayProfile));

% --- display PowerDelayProfile ---

figure
surf(dlyvec,pwrvec,10*log10(PowerDelayProfile+eps),'LineStyle','none','FaceColor','interp','EdgeLighting','phong');
caxis([-70,-10]);
view(2)
xlabel('delay in s')
ylabel('power in dB')
title('Power delay profile - probability density function')
hc = colorbar;
set(hc,'YLim',[-70,-10])
clear newYTic YTic
YTic = get(hc,'YTickLabel');
axes(hc);
ax = axis;
dta = (ax(3)-ax(4))/(size(YTic,1)-1);
for kk = 1:size(YTic,1)
    oldYTic(kk,:) = [' '];
    newYTic(kk,:) = ['10^',num2str(str2num(YTic(kk,:))/10),''];
    text(1.2,ax(3)-dta*(kk-1),['10^{',num2str(str2num(YTic(kk,:))/10),'}'],'interpreter','tex','horizontalalignment','left','verticalalignment','middle');
end
set(hc,'YTickLabel',oldYTic);

% ---------------------------------

disp(' ');
disp('Simulation finished');

