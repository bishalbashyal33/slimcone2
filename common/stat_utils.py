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

import json
import re
import pprint
import common.utils as utils

def computeavgs(resultjson, output='stdout', allstatlayers=list()):
  avgs = dict()

  pp = pprint.PrettyPrinter(indent=2)

  for instancetriple, results in resultjson.items():
    outputlist = results[output]
    runs = len(outputlist)
    
    avgs[instancetriple] = dict()

    for outputelem in outputlist:
      try:
        # outputelem is in json format
        instancestats = json.loads(outputelem)['statistics'] 

        for statlayers in allstatlayers:
          try:
            t = utils.traversedictlevels(avgs[instancetriple], statlayers, True)
            s = utils.traversedictlevels(instancestats, statlayers)

            if 'rounds' in statlayers:
              for round, s in s.items():
                if round not in t:
                  t[round] = dict()
                t[round] = addtosums(t[round], s)
            else:
              t = addtosums(t, s)
          except Exception as e:
            continue

      except Exception as e:
        # deal with elements not in json format (e.g., output from libmemusage)
        instancestats = outputelem # plain text to parse
        for statnames in allstatlayers:
          t = utils.traversedictlevels(avgs[instancetriple], statnames, True)

          s = dict()
          for statname in statnames:
            m = re.search(r'\s*' + re.escape(statname) + r'\s*:\s*(\d+\.*\d*)', instancestats)
            if m is not None:
              s[statname] = m.group(1)
          t = addtosums(t, s)
    for statlayers in allstatlayers:
      t = utils.traversedictlevels(avgs[instancetriple], statlayers)

      if 'rounds' in statlayers:
        for round, _ in t.items():
          t[round] = computeavg(t[round], runs)
      else:
        t = computeavg(t, runs)

  #pp.pprint(avgs)

  return avgs

def addtosums(sums, stats):
  try:
    sums = addtoallkeys(sums, stats)
  except TypeError as te:
    for subkey,  in stats.items():
      if subkey not in sums:
        sums[subkey] = dict()     
      sums[subkey] = addtoallkeys(sums[subkey], stats[subkey])
 
  return sums

def computeavg(sums, runs):
  for k, v in sums.items():
    if isinstance(sums[k], list):
      sums[k] = [[pair[0], pair[1] / float(runs)] for pair in sums[k]]
    else:
      sums[k] = sums[k] / float(runs)
  
  return sums

def computeavghistogram(stats):
  avghist = dict()

  # sum up
  for hist in stats:
    for pair in hist:
      if pair is None or len(pair) == 0:
        continue

      if pair[0] not in avghist:
        avghist[pair[0]] = list()
      avghist[pair[0]] += [pair[1]]
 
  # build avgs
  for k, v in avghist.items():
    avghist[k] = float(sum(v)) / len(v)

  return avghist

def addtoallkeys(sums, stats):
  for key, value in stats.items():
    if isinstance(value, list): 
      if key not in sums:
        # DO NOT use [[0, 0.0]] * len(value)
        # https:/i/stackoverflow.com/questions/21036140/python-two-dimensional-array-changing-an-element
        sums[key] = [[0, 0.0] for _ in range(len(value))]
      
      sums[key] = addtohistogram(sums[key], value)
    else: # str (aka float)
      if key not in sums:
        sums[key] = 0.0

      sums[key] += float(value)

  return sums

def addtohistogram(sums, hist):
  for pair in hist:
    idx = hist.index(pair)
    key = int(pair[0])
    value = float(pair[1])

    sums[idx][0] = key # redundant but ok
    sums[idx][1] += value # sum up

  return sums
