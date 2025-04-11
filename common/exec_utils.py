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

import common.utils as utils

import subprocess
import os
import sys
import tempfile

def exec_topk(
    binary,
    sorted_datasets,
    sorted_queries,
    sorted_ks,
    runs,
    queries_per_size,
    libmemusage_path,
    max_processes,
    processes):
  """
  TODO
  """

  use_tempfiles = True

  # 2-level dictionary that stores the piped stdout for each process/instance
  # top-key: the path to the binary that finished execution
  # 2nd-level-key: triple of the form (dataset, query, k) (all as strings)
  # 2nd-level-value: list of stdouts for different runs
  outs = dict()

  print('Spawning child processes for all instances requested for ' +
    str(binary['command']) + ':')

  # each (dataset, query, k) combination
  for dataset in sorted_datasets:
    documentname = utils.last_level_from_unix_path(dataset['path'])

    for query in sorted_queries:
      queryname = utils.last_level_from_unix_path(query['path'])

      # only document-related queries (+ '_' to avoid, for example, xmark16
      # queries to be evaluated with xmark1 document)
      if documentname[:-4] + '_' in queryname:
        for k in sorted_ks:
          # debug
          print('  ' +
            'document: ' + documentname + ', query: ' + queryname +
            ', k: ' + str(k) + ', runs: ',
            end='')

          for run in range(0, runs):
            # add preloading of libmemusage shared library to track memory usage
            os.environ["LD_PRELOAD"] = libmemusage_path
            # concatenate suffix consisting of k, dataset, and query
            command_suffix = [str(k), str(dataset['path']), str(query['path'])]

            # try to spawn a new child process for unit under evaluation
            processes, finished_processes = try_spawn_process(
              binary['command'] + command_suffix,
              processes,
              max_processes,
              use_tempfiles)

            # debug
            print(str(run), end=' ')
            sys.stdout.flush()

            # wait for some child process to finish s.t. a new one can be started
            outs = save_process_stdout_to_dict(finished_processes, outs)

          print()

  # debug
  print('Waiting for remaining child processes to terminate. ', end='')

  # wait for all children to finish
  while len(processes) > 0:
    processes, finished_processes = wait_and_update(processes)
    outs = save_process_stdout_to_dict(finished_processes, outs)

  # debug
  print('[' + '\x1b[6;30;42m' + 'ALL INSTANCES TERMINATED SUCCESSFULLY' +
    '\x1b[0m' + ']\n')

  return outs

def try_spawn_process(
    command,
    processes=None,
    max_processes=None,
    use_tempfiles=False):
  """
  Tries to spawn a new subprocess (using Popen) for a given command. If the
  maximum number of processes is reached after this spawn, it waits for one
  child process to finish s.t. subsequent calls to this function can spawn the
  next subprocess.

  command       A sequence of program arguments or else a single string (see
                Popen's args).
  processes     A set of processes (created by calls to Popen) that is updated
                in the process (spawned processes are added, terminated
                processes are removed).
  max_processes The maximum number of concurrently executed (sub)processes.
  use_tempfiles A boolean flag to indicate that stdout/stderr should be piped to
                tempfiles instead of directly to this script.

  Returns the updated processes set and the finished process(es).
  """

  stdout_redir = tempfile.NamedTemporaryFile() if use_tempfiles else subprocess.PIPE
  stderr_redir = tempfile.NamedTemporaryFile() if use_tempfiles else subprocess.PIPE

  process = subprocess.Popen(command, stdout=stdout_redir, stderr=stderr_redir)

  finished = None

  if processes is not None:
    processes.add((process, stdout_redir, stderr_redir))

    if len(processes) >= max_processes:
      processes, finished = wait_and_update(processes)
  else: # processes is None aka 'no parallelization'
    os.wait()

  return processes, finished

def wait_and_update(processes):
  """
  Waits for a process to terminate/finish and updates the given set (processes)
  by removing this process from it.

  processes A set of processes (created by calls to Popen).

  Returns the updated processes set and the finished process(es).
  """

  os.wait()
  finished = [entry for entry in processes if entry[0].poll() is not None]
  processes.difference_update(finished)

  return processes, finished

def save_process_stdout_to_dict(finished_processes, outs):
  """
  TODO
  """

  if finished_processes:
    for process, stdout_redir, stderr_redir in finished_processes:
      finished_binary = process.args[0]
      finished_k = process.args[-3]
      finished_dataset = process.args[-2]
      finished_query = process.args[-1]

      if stdout_redir == subprocess.PIPE:
        # this also implies that stderr_redir == subprocess.PIPE
        out, err = process.communicate()
      else: # tempfile
        stdout_redir.seek(0) # goto beginning of tempfile
        out = stdout_redir.read() # read tempfile
        stdout_redir.close() # delete tempfile

        stderr_redir.seek(0)
        err = stderr_redir.read()
        stderr_redir.close()

      instance_triple = (finished_dataset, finished_query, finished_k)

      if instance_triple not in outs:
        outs[instance_triple] = dict()

      for output in ['stdout', 'stderr']:
        if output not in outs[instance_triple]:
          outs[instance_triple][output] = list()

      outs[instance_triple]['stdout'].append(out.decode('utf-8'))
      outs[instance_triple]['stderr'].append(err.decode('utf-8'))

  return outs
