#!/usr/bin/python3

"""
The MIT License (MIT)
Copyright (c) 2017 Daniel Kocher.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import common.exec_utils as exec_utils

import os
import subprocess

def stat_to_tex(
    statfile,
    xlabel=None,
    ylabel=None,
    plottypes=['line', 'bar'],
    logx=False,
    logy=False,
    title=None,
    factor=1.0,
    legendpos='right',
    legendrows=3,
    bar_width=None,
    ymin=None,
    ymax=None):
  print(statfile)
  last_slash_pos = statfile.rfind('/')
  directory = statfile[:last_slash_pos]
  file_no_extension = statfile[(last_slash_pos + 1):statfile.find('.stat')]

  for plottype in plottypes:
    # build directory string for current plottype
    plot_directory = directory + '/' + plottype + '/'

    # create plot directory if it does not exist
    if not os.path.exists(plot_directory):
      os.makedirs(plot_directory)

    # build texfile string
    texfile = plot_directory + file_no_extension + '.tex'

    # create plot
    plot(
      statfile,
      texfile,
      plottype,
      xlabel,
      ylabel,
      logx,
      logy,
      title,
      factor,
      legendpos,
      legendrows,
      bar_width,
      ymin,
      ymax)

def plot(
    statfile,
    texfile,
    plottype,
    xlabel,
    ylabel,
    logx,
    logy,
    title,
    factor,
    legendpos,
    legendrows,
    bar_width,
    ymin,
    ymax,
    standalone=True):
  # debug
  print('Generating ' + str(texfile) + ' from ' + str(statfile) + ', plot type = ' + str(plottype) + ', xlabel = ' + str(xlabel) +
    ', ylabel = ' + str(ylabel) + (', title = ' + str(title) if title else '') + ', logx = ' + str(logx) + ', logy = ' + str(logy) +
    ', factor = ' + str(factor) + ', legendpos = ' + legendpos + ', legendrows = ' + str(legendrows) +
    ', y = [' + str(ymin) + '; ' + str(ymax) +'], standalone = ' + str(standalone))

  command = list()

  command.append('Rscript')
  command.append('/home/dkocher/frosch/kocher-topk-indexing-code/experiments/r/plot.r')

  command.append('--texfile=' + add_quotes_if_necessary(texfile))
  command.append('--statfile=' + add_quotes_if_necessary(statfile))
  command.append('--xlabel=' + xlabel)
  command.append('--ylabel=' + ylabel)
  command.append('--factor=' + str(factor))
  command.append('--legendpos=' + legendpos)
  command.append('--legendrows=' + str(legendrows))

  if ymin:
    command.append('--ymin=' + str(ymin))

  if ymax:
    command.append('--ymax=' + str(ymax))

  if logx and plottype != 'bar':
    command.append('--logx')

  if logy:
    command.append('--logy')

  if title:
    command.append('--title=' + add_quotes_if_necessary(title))

  if plottype == 'bar':
    if not bar_width:
      bar_width = 0.25
    command.append('--bw=' + str(bar_width))

  if standalone:
    command.append('--standalone')

  command.append('--' + plottype)

  print(command)

  exec_utils.try_spawn_process(command)

def add_quotes_if_necessary(s):
  if s and s[0] == '\'' and s[-1] == '\'':
    # quotes are already present
    return

  if s is not None and ' ' in s:
    s = '\'' + s + '\''

  return s
