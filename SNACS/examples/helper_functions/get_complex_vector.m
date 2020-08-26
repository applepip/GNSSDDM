%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%    Supporting script for
%
%    SNACS - The Satellite Navigation Radio Channel Simulator
%
%    get_complex_vector.m
%
%    Matlab script reading a vector of complex values from a HDF5 file.
%
%    Copyright (C) 2009  F. M. Schubert
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

function [ out ] = get_complex_vector(file_name, path_name, dataset_name)

%% new version, reading compound dataset using low level HDF5 functions.
% source:
% http://www.hdfgroup.uiuc.edu/UserSupport/examples-by-api/matlab/HDF5_M_Examples/h5ex_t_cmpd.m

file = H5F.open (file_name, 'H5F_ACC_RDONLY', 'H5P_DEFAULT');

group = H5G.open(file, path_name);

dset = H5D.open (group, dataset_name);

%
% Get dataspace and allocate memory for read buffer.
%
space = H5D.get_space (dset);
[numdims dims maxdims] = H5S.get_simple_extent_dims (space);
dims = fliplr(dims');

%
% Create the compound datatype for memory.
%
wdata.real = 0.0;
wdata.imag = 0.0;

memtype = H5T.create ('H5T_COMPOUND', H5ML.sizeof (wdata));

H5T.insert (memtype, 'real',H5ML.hoffset (wdata(1), 'real'),'H5T_NATIVE_DOUBLE');
H5T.insert (memtype, 'imag',H5ML.hoffset (wdata(1), 'imag'), 'H5T_NATIVE_DOUBLE');

%
% Read the data.
%
rdata=H5D.read (dset, memtype, 'H5S_ALL', 'H5S_ALL', 'H5P_DEFAULT');

out = rdata.real + j .* rdata.imag;

%
% Close and release resources.
%
H5D.close (dset);
H5S.close (space);
H5T.close (memtype);
H5G.close (group);
H5F.close (file);


%% old, slow version:
% raw = hdf5read(f, path);
% 
% amount = numel(raw);
% 
% out = zeros(amount, 1);
% 
% for k = 1:amount
%     cir_raw = cell2mat(raw(k).Data);
%     out(k) = cir_raw(1) + j * cir_raw(2);
% end

end