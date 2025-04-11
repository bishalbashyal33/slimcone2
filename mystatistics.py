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

import common.utils as utils
import common.stat_utils as stat_utils

import argparse
import sys
import pickle
import pprint
import statistics

import json
import random

parser = argparse.ArgumentParser()

parser.add_argument(
  '--resultdir',
  default='./results/',
  help="directory to store the extracted stat files (CSV format)")
parser.add_argument(
  'results',
  nargs='+',
  help="path to single file that has been serialized to disk by profile.py (to extract the statistics in separate stat files)")

args = parser.parse_args()

for result in args.results:
  with open(result, 'rb') as fp:
      outs = pickle.load(fp)

      name = outs[0]
      results = outs[1]
      sorted_datasets = outs[2]
      sorted_queries = outs[3]
      sorted_ks = outs[4]
      query_sizes_nodes = outs[5]
      query_sizes_mb = outs[6]
      query_paths_by_size_nodes = outs[7]
      dataset_sizes_mb = outs[8]
      dataset_sizes_nodes = outs[9]
      dataset_paths_by_size_mb = outs[10]

pp = pprint.PrettyPrinter(indent=2)

# compute total time (build time + query time)
try:
    for key, val in results.items():
        for i, jsonstr in enumerate(val['stdout']):
            d = json.loads(jsonstr)
            toextend = d['statistics']['filter']['global']
            toextend['timingtotal'] = \
                str(float(d['statistics']['index']['timingindexing']) + float(toextend['timingfiltering']))
            val['stdout'][i] = json.dumps(d)
except:
    # non-index methods
    pass

basestatlayers = [['index'], ['filter', 'global']]
roundstatlayers = [['filter', 'rounds']]
nonjsonstats = [['heap peak']]

# avgs is a dictionary [instance_triple]['index' | 'filter']['global' | 'rounds'][roundno] = dict of stats for index | global filter | round filter
try:
  avgs = stat_utils.computeavgs(results, output='stdout', allstatlayers=basestatlayers + roundstatlayers)

  for instancetriple, instancestats in stat_utils.computeavgs(results, output='stderr', allstatlayers=nonjsonstats).items():
    for statkey, stat in instancestats.items():
      avgs[instancetriple][statkey] = stat
except Exception as e:
  sys.stderr.write(e)
  sys.stderr.write('Unable to compute averages. Wrong output format?\n')
  sys.exit(1)

avgsbydocumentsize = dict()
avgsbyquerysize = dict()
avgsbyk = dict()
avgsbyround = dict()

# avgsbydocumentsize
for k in sorted_ks:
  if k not in avgsbydocumentsize:
    avgsbydocumentsize[k] = dict()

  for dataset in sorted_datasets:
    if dataset['size_mb'] not in avgsbydocumentsize[k]:
      avgsbydocumentsize[k][dataset['size_mb']] = dict()

    for query in sorted_queries:
      if query['size_nodes'] not in avgsbydocumentsize[k][dataset['size_mb']]:
        avgsbydocumentsize[k][dataset['size_mb']][query['size_nodes']] = list()

      documentname = utils.last_level_from_unix_path(dataset['path'])
      queryname = utils.last_level_from_unix_path(query['path'])

      # only document-related queries
      if documentname[:-4] + '_' in queryname:
        instance_triple = (dataset['path'], query['path'], k)
        instance_stats = avgs[instance_triple]

        if instance_stats not in avgsbydocumentsize[k][dataset['size_mb']][query['size_nodes']]:
          avgsbydocumentsize[k][dataset['size_mb']][query['size_nodes']].append(instance_stats)

# avgsbyquerysize
for k in sorted_ks:
  if k not in avgsbyquerysize:
    avgsbyquerysize[k] = dict()

  for query in sorted_queries:
    if query['size_nodes'] not in avgsbyquerysize[k]:
      avgsbyquerysize[k][query['size_nodes']] = dict()

    for dataset in sorted_datasets:
      if dataset['size_mb'] not in avgsbyquerysize[k][query['size_nodes']]:
        avgsbyquerysize[k][query['size_nodes']][dataset['size_mb']] = list()

      documentname = utils.last_level_from_unix_path(dataset['path'])
      queryname = utils.last_level_from_unix_path(query['path'])

      # only document-related queries
      if documentname[:-4] + '_' in queryname:
        instance_triple = (dataset['path'], query['path'], k)
        instance_stats = avgs[instance_triple]

        if instance_stats not in avgsbyquerysize[k][query['size_nodes']][dataset['size_mb']]:
          avgsbyquerysize[k][query['size_nodes']][dataset['size_mb']].append(instance_stats)

# avgsbyk
for query in sorted_queries:
  if query['size_nodes'] not in avgsbyk:
    avgsbyk[query['size_nodes']] = dict()

  for k in sorted_ks:
    if k not in avgsbyk[query['size_nodes']]:
      avgsbyk[query['size_nodes']][k] = dict()

    for dataset in sorted_datasets:
      if dataset['size_mb'] not in avgsbyk[query['size_nodes']][k]:
        avgsbyk[query['size_nodes']][k][dataset['size_mb']] = list()

      documentname = utils.last_level_from_unix_path(dataset['path'])
      queryname = utils.last_level_from_unix_path(query['path'])

      # only document-related queries
      if documentname[:-4] + '_' in queryname:
        instance_triple = (dataset['path'], query['path'], k)
        instance_stats = avgs[instance_triple]

        if instance_stats not in avgsbyk[query['size_nodes']][k][dataset['size_mb']]:
          avgsbyk[query['size_nodes']][k][dataset['size_mb']].append(instance_stats)

# avgsbyrounds
try:
  for dataset in sorted_datasets:
    datasetfilename = dataset['path'].split('/')[-1].split('.')[0]

    for query in sorted_queries:
      documentname = utils.last_level_from_unix_path(dataset['path'])
      queryname = utils.last_level_from_unix_path(query['path'])

      # only document-related queries
      if documentname[:-4] +'_' in queryname:
        queryfilename = query['path'].split('/')[-2].split('.')[0] + '-' + query['path'].split('/')[-1].split('.')[0]

        level1key = datasetfilename + '-' + queryfilename
        #print(level1key)

        if level1key not in avgsbyround:
          avgsbyround[level1key] = dict()

        for k in sorted_ks:
          instance_triple = (dataset['path'], query['path'], k)
          instance_stats = avgs[instance_triple]

          for round, stats in instance_stats['filter']['rounds'].items():
            round = int(round)

            if round not in avgsbyround[level1key]:
              avgsbyround[level1key][round] = dict()

            if k not in avgsbyround[level1key][round]:
              avgsbyround[level1key][round][k] = list()

            if stats not in avgsbyround[level1key][round][k]:
              avgsbyround[level1key][round][k].append(stats)

        for _, v in avgsbyround[level1key].items():
          for k in sorted_ks:
            if k not in v:
              v[k] = list()
except Exception as e:
  # nothing to do
  sys.stderr.write('This algorithm seems to have no rounds. Skipping.\n')

baseplots = [
    ('vary document size', 'k', 'T', 'Q', avgsbydocumentsize, {82.09: 'TB', 111.12: 'XMark1', 222.9: 'XMark2', 446.71: 'XMark4', 895.03: 'XMark8', 1791.0: 'XMark16', 2161.17: 'DBLP', 6137.29: 'SP'}),
    ('vary query size', 'k', 'Q', 'T', avgsbyquerysize, None),
    ('vary k', 'q', 'k', 'T', avgsbyk, None)]
roundplots = [('rounds', '', 'round', 'k', avgsbyround, None)]

for statlayers in basestatlayers + nonjsonstats:
  for plotname, plotsuffix, varied, hor, plotstats, mapping in baseplots:
    statkeys = set()
    maxv2length = 0
    for k1, v1 in sorted(plotstats.items()):
      for k2, v2 in sorted(v1.items()):
        maxv2length = max(maxv2length, len(v2))
        for k3, v3 in sorted(v2.items()):
          statkeys = list(utils.traversedictlevels(v3[0], statlayers).keys())

    d = dict()
    for k1, v1 in sorted(plotstats.items()):
      if k1 not in d:
        d[k1] = dict()

      for k2, v2 in sorted(v1.items()):
        if k2 not in d[k1]:
          d[k1][k2] = dict()

        for k3, v3 in sorted(v2.items()):
          if k3 not in d[k1][k2]:
            d[k1][k2][k3] = dict()

          t = utils.traversedictlevels(d[k1][k2][k3], statlayers, True)

          for statkey in sorted(statkeys):
            if statkey not in t:
              t[statkey] = list()

            for e in v3:
              te = utils.traversedictlevels(e, statlayers)
              t[statkey] += [te[statkey]]

            if 'hist' == statkey[-4:]:
              t[statkey] = stat_utils.computeavghistogram(t[statkey])
            else:
              t[statkey] = float(sum(t[statkey])) / max(1, len(v3))

    #pp.pprint(d)

    # write statfiles
    for statkey in statkeys:
      if 'hist' == statkey[-4:]:
        for k1, v1 in sorted(d.items()):
          filename = args.resultdir + '-'.join(statkey.split()) + '-' + '-'.join(plotname.split()) + '-' + '-'.join(plotsuffix.split()) + str(k1) + '.stat'
          print(filename)

          vs = set()
          v3s = list()
          line = 'val'
          for k2, v2 in sorted(v1.items()):
            for k3, v3 in sorted(v2.items()):
              line += ';{}, {} = {}, {} = {}'.format(name, hor, k3, varied, k2)

              histentry = utils.traversedictlevels(v3, statlayers)[statkey]
              vs = vs.union(list(histentry.keys()))
              v3s += [histentry]
          line += ';\n'

          for v in sorted(vs):
            line += '{}'.format(v)
            for v3 in v3s:
              line += ';{}'.format(v3[v] if v in v3 else '')
            line += ';\n'

          with open(filename, 'w') as f:
            f.write(line)
      else:
        for k1, v1 in sorted(d.items()):
          filename = args.resultdir + '-'.join(statkey.split()) + '-' + '-'.join(plotname.split()) + '-' + '-'.join(plotsuffix.split()) + str(k1) + '.stat'
          print(filename)

          line = ''
          for k2, v2 in sorted(v1.items()):
            line += '{}'.format(varied)

            if mapping:
              line += ';SYM'

            for k3 in sorted(v2.keys()):
              line += ';{}, {} = {}'.format(name, hor, k3)
            line += ';\n'
            break # header line only once

          for k2, v2 in sorted(v1.items()):
            line += '{}'.format(k2)

            if mapping:
              line += ';{}'.format(mapping[k2] if k2 in mapping.keys() else "")

            for k3, v3 in sorted(v2.items()):
              line += ';{}'.format(utils.traversedictlevels(v3, statlayers)[statkey])

            # add empty columns in statfile
            if len(v2) < maxv2length:
              for i in range(maxv2length - len(v2)):
                line += ';'
            line += ';\n'

          with open(filename, 'w') as f:
            f.write(line)

for statlayers in roundstatlayers:
  for plotname, plotsuffix, varied, hor, plotstats, mapping in roundplots:
    statkeys = set()
    maxv2length = 0
    for k1, v1 in sorted(plotstats.items()):
      for k2, v2 in sorted(v1.items()):
        maxv2length = max(maxv2length, len(v2))
        for k3, v3 in sorted(v2.items()):
          if len(v3) > 0:
            statkeys = list(v3[0].keys())

    d = dict()
    for k1, v1 in sorted(plotstats.items()):
      if k1 not in d:
        d[k1] = dict()

      for k2, v2 in sorted(v1.items()):
        if k2 not in d[k1]:
          d[k1][k2] = dict()

        for k3, v3 in sorted(v2.items()):
          if k3 not in d[k1][k2]:
            d[k1][k2][k3] = dict()

          t = utils.traversedictlevels(d[k1][k2][k3], statlayers, True)

          for statkey in sorted(statkeys):
            if statkey not in t:
              t[statkey] = list()

            for e in v3:
              te = utils.traversedictlevels(e, statlayers)
              t[statkey] += [te[statkey]]

            if 'hist' == statkey[-4:]:
              t[statkey] = stat_utils.computeavghistogram(t[statkey])
            else:
              t[statkey] = float(sum(t[statkey])) / max(1, len(v3))

    # write statfiles
    for statkey in statkeys:
      if 'hist' == statkey[-4:]:
        for k1, v1 in sorted(d.items()):
          filename = args.resultdir + '-'.join(statkey.split()) + '-' + '-'.join(plotname.split()) + '-' + '-'.join(plotsuffix.split()) + str(k1) + '.stat'
          print(filename)

          vs = set()
          v3s = list()
          line = 'val'
          for k2, v2 in sorted(v1.items()):
            for k3, v3 in sorted(v2.items()):
              line += ';{}, {} = {}'.format(name, hor, k3)

              histentry = utils.traversedictlevels(v3, statlayers)[statkey]
              vs = vs.union(list(histentry.keys()))
              v3s += [histentry]
          line += ';\n'

          for v in sorted(vs):
            line += '{}'.format(v)
            for v3 in v3s:
              line += ';{}'.format(v3[v] if v in v3 else '')
            line += ';\n'

          with open(filename, 'w') as f:
            f.write(line)
      else:
        for k1, v1 in sorted(d.items()):
          filename = args.resultdir + '-'.join(statkey.split()) + '-' + '-'.join(plotname.split()) + '-' + '-'.join(plotsuffix.split()) + str(k1) + '.stat'
          print(filename)

          line = ''
          for k2, v2 in sorted(v1.items()):
            line += '{}'.format(varied)

            if mapping:
              line += ';SYM'

            for k3 in sorted(v2.keys()):
              line += ';{}, {} = {}'.format(name, hor, k3)
            line += ';\n'
            break # header line only once

          for k2, v2 in sorted(v1.items()):
            line += '{}'.format(k2)

            if mapping:
              line += ';{}'.format(mapping[k2] if k2 in mapping.keys() else "")

            for k3, v3 in sorted(v2.items()):
              val = utils.traversedictlevels(v3, statlayers)[statkey]
              line += ';{}'.format(val)

            # add empty columns in statfile
            if len(v2) < maxv2length:
              for i in range(maxv2length - len(v2)):
                line += ';'
            line += ';\n'

          with open(filename, 'w') as f:
            f.write(line)
