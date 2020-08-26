#!/usr/bin/env python
#   Example file for
#
#   SNACS - The Satellite Navigation Radio Channel Simulator
#
#   Copyright (C) 2012  F. M. Schubert
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#   This file generates an artifical times series with 1 line-of-sight path
#   that begins to accelerate slowly up to a constant speed.

# # initialization
import os
import sys
from numpy import *
import matplotlib.pyplot as plt
import argparse

# sys.path.append('../../../../../cdx_format/CDX_library_python')
import WriteContinuousDelayCDXFile as cdx

# parse command line arguments:
parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", help="file name of CDX file to be created")
parser.add_argument("-l", "--length", help="length of channel to be computed in s", type=float)
parser.add_argument("-r", "--rate", help="CIR rate in Hz", type=float)
parser.add_argument("-s", "--speed", help="final speed in m/s", type=float)
parser.add_argument("-a", "--accelerate", help="start accelerating at second in s", type=float)
parser.add_argument("-c", "--constant", help="constant speed at second in s", type=float)
parser.add_argument("-t", "--transmitter", help="transmitter frequency in Hz", type=float)
args = parser.parse_args()

# parameter setting
file_name = args.file;
len = args.length;  # signal length in s
cir_rate = args.rate
speed = args.speed
transmitter_frequency = args.transmitter

# set parameters for CDX file:
parameters = dict(delay_type='continuous-delay',
                  c0=3e8,
                  cir_rate=cir_rate,
                  nof_links=1,
                  enable_compression=False,
                  transmitter_frequency=transmitter_frequency)

# create CDX file:
cdx_file = cdx.WriteContinuousDelayCDXFile(file_name, parameters)

nof_cirs = len * parameters['cir_rate']

start_acceleration = args.accelerate;  # in s
full_speed_at = args.constant;  # in s

carrier_wavelength = parameters['c0'] / parameters['transmitter_frequency']

# # parameter checks

# TODO check for maximum vehicle speed:
maximum_possible_speed = parameters['c0'] * parameters['cir_rate'] / parameters['transmitter_frequency'] / 2  # sampling theorem

cir_start_acceleration = start_acceleration * parameters['cir_rate']
cir_full_speed_at = full_speed_at * parameters['cir_rate']
cirs_acceleration = cir_full_speed_at - cir_start_acceleration;

speeds = zeros((nof_cirs, 1));
distances = zeros((nof_cirs, 1));
los_complex_amplitudes = zeros((nof_cirs, 1), dtype=complex);


# # generate channel impulse response
for k in arange(nof_cirs):
    # print k
    cir_number = k - 1  # k-1: CIR number should start at 0
    
    act_speed = 0
    
    if (k < cir_start_acceleration):
        # no movement, delay = 0:
        delay = 0;
    elif (k >= cir_start_acceleration) and (k < cir_full_speed_at):
        # acceleration using a cos function:
        speeds[k] = speed * (cos(pi + (k - cir_start_acceleration) / cirs_acceleration * pi) + 1) / 2
        
        # linear acceleration:
        #         speeds[k] = speed * (k-cir_start_acceleration) / cirs_acceleration;
    elif (k >= cir_full_speed_at):
        # constant full speed:
        speeds[k] = speed;
    
    # calculate distance using the speed. k * 1/cir_rate is the current
    # time
    if k < 2:
        distances[k] = 0
    else:
        distances[k] = distances[k - 1] + speeds[k] * 1 / parameters['cir_rate']

    
    # build struct for writing to HDF5 file:
    cir = empty(1, dtype=cdx.cir_dtype)
    
    # calculate the line of sight complex amplitude:
    los_complex_amplitudes[k] = 1.0 * exp(-1j * (2.0 * pi / carrier_wavelength) * distances[k])
    
    # calculate LOS delay from distance using speed of light:
    cir[0]["delays"] = distances[k] / parameters['c0']
    cir[0]["real"] = los_complex_amplitudes[k].real
    cir[0]["imag"] = los_complex_amplitudes[k].imag
    reference_delay = distances[k] / parameters['c0']
    
    # write this CIR to file:
    cdx_file.append_cir(cir, reference_delay)
    
# # plot speed, distance, amplitude:
x_ax = arange(0, nof_cirs / parameters['cir_rate'], 1. / parameters['cir_rate'])

plt.figure()

plt.subplot(221)
plt.plot(x_ax, speeds)
plt.xlabel('Time [s]')
plt.ylabel('Speed [m/s]')
plt.grid()
plt.title('Receiver Speed')
plt.ylim([-10, speed + 20]);

#
# if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_receiver-speed.png');  end
#

plt.subplot(223)
plt.plot(x_ax, distances)
plt.title('Distance Receiver - Transmitter')
plt.grid()
plt.xlabel('Time [s]')
plt.ylabel('Distance [m]')
plt.ylim([-10, distances[-1] + 20])

#
# if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_distance.png'); end
#

plt.subplot(222)
plt.title('Line-Of-Sight Amplitude')
plt.plot(x_ax, abs(los_complex_amplitudes))
plt.grid()
plt.xlabel('Time [s]')
plt.ylabel('Magnitude')
plt.ylim([-10, abs(los_complex_amplitudes[-1]) + 10])

#
# if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_amplitude.png');  end
#

plt.subplot(224)
plt.title('Line-Of-Sight Phase');
plt.plot(x_ax, angle(los_complex_amplitudes))
plt.grid()
plt.xlabel('Time [s]')
plt.ylabel('Phase [rad]')
plt.ylim([-4, 4]);

#
# if save_figures save_figure_as_png('../../documentation/doxygen-sources/graphic/example03_phase.png');  end
#
#

# IMPORTANT: close the file
del cdx_file;

# show figure
plt.show()

# sys.exit()

print 'all done.'
