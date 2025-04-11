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

from datetime import datetime
from shutil import copy, rmtree
import os
import sys

def print_configuration(executable, args, queries_per_size):
  """
  Prints the configuration of the unit under evaluation, the witness, and the
  arguments parsed from the command-line.

  executable        TODO
  args              TODO
  queries_per_size  TODO

  Returns nothing.
  """

  configuration = 'Profiling Configuration:\n\t' + \
    'tmp directory: ' + args.tmpdir + '\n\t' + \
    'result directory: ' + args.resultdir + '\n\t' + \
    'dataset directory: ' + args.datasetdir + '\n\t' + \
    'queries directory: ' + args.queriesdir + '\n\t' + \
    'executable: ' + executable['name'] + '\n\t' + \
    'number of runs: ' + str(args.runs) + '\n\t' + \
    'values of k: ' + str(args.k) + '\n\t' + \
    'queries per size: ' + str(queries_per_size) + '\n\t' + \
    'max. number of processes: ' + str(args.maxprocesses) + '\n\t' + \
    'path to libmemusage.so: ' + str(args.libmemusagepath) + \
    '\n'

  print(configuration)

def prepare_configuration(args, queries_per_size):
  """
  Prepares the underlying (Unix) system for the experiments, i.e.,
    - checks the arguments parsed from the command-line,
    - sets up all necessary (sub)directories for the experiments,
    - copies the executables of witness and unit under evaluation to the tmp-dir
      to ensure that they are not modified during the experiments, and
    - prints the final configuration to stdout.

  args              TODO
  queries_per_size  TODO

  Returns nothing.
  """

  executable_arg = args.executable

  executable_name = executable_arg
  executable_command = executable_arg

  executable = dict()
  if ':' in executable_arg:
    split_executable_args = executable_arg.split(':')
    executable_name = split_executable_args[0]
    executable_command = split_executable_args[1]

  executable['name'] = executable_name
  executable['command'] = executable_command.split()
  executable_binary = executable['command'][0]

  terminate = False

  if not os.path.exists(args.libmemusagepath):
    sys.stderr.write('Cannot find libmemusage.so in ' + args.libmemusagepath +
      '.\n')
    terminate = True

  if terminate:
    exit(-1)

  # create tmp subdirectory if it does not exist yet
  if not os.path.exists(args.tmpdir):
    os.makedirs(args.tmpdir)

  # create results subdirectory if it does not exist yet
  if not os.path.exists(args.resultdir):
    os.makedirs(args.resultdir)

  # unique subdirectory, always create
  print(str(executable));
  args.resultdir = args.resultdir + datetime.now().strftime('%Y%m%d_%H%M%S') + \
    '-' + executable['name'] + '/'


  os.makedirs(args.resultdir)

  # copy witness executable into tmp folder
  copy(executable_binary, args.tmpdir + 'executable')
  # update stored path to destination of copied witness binary
  executable['command'][0] = args.tmpdir + 'executable'

  print_configuration(executable, args, queries_per_size)

  return executable
