%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%    Supporting script for
%
%    SNACS - The Satellite Navigation Radio Channel Simulator
%
%    plot_snacs_results_example04.m
%
%    MATLAB script for displaying results of SNACS Example 02 simulation.
%
%    Copyright (C) 2011  F. M. Schubert
%
%    This program is free software: you can redistribute it and/or modify
%    it under the terms of the GNU General Public License as published by
%    the Free Software Foundation, either version 3 of the License, or
%    (at your option) any later version.
%
%    This program is distributed in the hope that it will be useful,
%    but WITHOUT ANY WARRANTY; without even the implied warranty of
%    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%    GNU General Public License for more details.
%
%    You should have received a copy of the GNU General Public License
%    along with this program.  If not, see <http://www.gnu.org/licenses/>.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clc
clear all

addpath('../helper_functions');

plotting.length = 0;
plotting.line_width = 1;

plot_snacs_results('/tmp/snacs-result_example01/result.h5', plotting);

% save the figure:
% plot_snacs_results('/tmp/snacs-result_example04/result.h5', true, 1024, 768, '../../documentation/doxygen-sources/graphic/snacs-result_example04.png');

