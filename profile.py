#!/usr/bin/python3

"""
The MIT License (MIT)
Copyright (c) 2017 Daniel Kocher

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

import common.config as config
import common.utils as utils
import common.exec_utils as exec_utils
import common.xml_utils as xml_utils

import argparse
import sys
import pickle

parser = argparse.ArgumentParser()

# Specify directory to store the executables
parser.add_argument(
  '--tmpdir',
  default='./tmp/',
  help="directory to temporarily store the C++ binary")
# Specify directory to store the results
parser.add_argument(
  '--resultdir',
  default='./results/',
  help="directory to store the results")
# Specify the number of runs the average is computed with
parser.add_argument(
  '--runs',
  type=int,
  default=5,
  help="number of runs for a single experiment (robustness parameter)")
# Specify a list of all values of k the tests are executed for
parser.add_argument(
  '--k',
  nargs='+',
  metavar='K-VALUES',
  default=10,
  help="a list of all k values the experiment is executed for")
# Specify the number of processes used to profile multiple instances
# concurrently. This may speed up the executions. However, for profiling, keep
# in mind that this may influence the timing results if more than 1 process is
# used.
parser.add_argument(
  '--maxprocesses',
  type=int,
  default=1,
  help="max. number of parallel processes during experiments (robustness parameter)")
# Specify the path to the memusage shared library
parser.add_argument(
  '--libmemusagepath',
  default='/lib/x86_64-linux-gnu/libmemusage.so',
  help="path to the memusage shared library")
# Specify the directory the documents are located in
parser.add_argument(
  'datasetdir',
  help="directory the documents are located in (all XML files will be used)")
# Specify the directory the queries are located in
parser.add_argument(
  'queriesdir',
  help="directory the queries are located in (all XML files will be used)")
# Specify the executable
parser.add_argument(
  'executable',
  help="path to the executable used for the experiments (k, document path, and query path will be appended, thus also specify other command-line options in this string, if any)")

args = parser.parse_args()

# gather xml files used as input for profiling

# debug
print('\nGathering information about datasets and queries.')

queries = xml_utils.gather_files(args.queriesdir)
queries_per_size = utils.get_equal_attribute_min_count(queries, 'size_nodes')
datasets = xml_utils.gather_files(args.datasetdir)

# get tree sizes in nodes
query_sizes_nodes = sorted(set([p['size_nodes'] for p in queries]))
dataset_sizes_nodes = sorted(set([p['size_nodes'] for p in datasets]))

# get tree sizes in MB
query_sizes_mb = sorted(set([p['size_mb'] for p in queries]))
dataset_sizes_mb = sorted(set([p['size_mb'] for p in datasets]))

# get query paths by query size in nodes
query_paths_by_size_nodes = utils.get_subdict(
queries,
query_sizes_nodes,
resultKey='path',
criteriaKey='size_nodes')

dataset_paths_by_size_mb = utils.get_subdict(
datasets,
dataset_sizes_mb,
resultKey='path',
criteriaKey='size_mb')

# sort necessary data structures once to be consistent throughout script
sorted_datasets = sorted(datasets, key=lambda entry: entry['size_mb'])
sorted_queries = sorted(queries, key=lambda entry: entry['size_nodes'])
sorted_ks = sorted(args.k)

# get binary configurations
executable = config.prepare_configuration(args, queries_per_size)

executable_binary = executable['command'][0]

processes = set()
outs = list()

outs += [executable['name']] # 0
outs += [exec_utils.exec_topk(
  executable,
  sorted_datasets,
  sorted_queries,
  sorted_ks,
  args.runs,
  queries_per_size,
  args.libmemusagepath,
  args.maxprocesses,
  processes)] # 1
outs += [sorted_datasets] # 2
outs += [sorted_queries] # 3
outs += [sorted_ks] # 4
outs += [query_sizes_nodes] # 5
outs += [query_sizes_mb] # 6
outs += [query_paths_by_size_nodes] # 7
outs += [dataset_sizes_mb] # 8
outs += [dataset_sizes_nodes] # 9
outs += [dataset_paths_by_size_mb] # 10

outs += [query_paths_by_size_nodes]

with open(args.resultdir + '/outs.dict', 'wb') as fp:
  pickle.dump(outs, fp)
