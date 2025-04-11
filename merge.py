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

import argparse
import sys

parser = argparse.ArgumentParser()

parser.add_argument(
  '--columns',
  nargs='*',
  type=int,
  default=None,
  help="skip the columns specified here (untested)")
parser.add_argument(
  '--hassym',
  action='store_true',
  default=False,
  help="set this to flag if the files to merge contain a column SYM")
parser.add_argument(
  'statfiles',
  nargs='+',
  help="a sequence of stat files (of the same type) that should be merged")
parser.add_argument(
  'outputfile',
  help="the output file consisting of the merged columns of all input files")

args = parser.parse_args()

try:
  for statfileindex, statfile in enumerate(args.statfiles):
    with open(statfile, 'r') as fp:
        continue
except Exception as e:
  print(e)
  print(".stat file ", statfile, " does not exist; skipping ...")
  sys.exit(1)

lines = list()
for statfileindex, statfile in enumerate(args.statfiles):
  with open(statfile, 'r') as fp:
    lineindex = 0
    for line in fp.readlines():
      allentries = [x.strip() for x in line.strip().split(';')[:-1]]

      varied = allentries[0]

      if args.hassym:
        sym = allentries[1]
        entries = allentries[2:]
      else:
        entries = allentries[1:]


      if statfileindex == 0:
        lines.append(varied + ';')

        if args.hassym:
          lines[lineindex] += sym + ';'

      for entryindex, entry in enumerate(entries):
        if args.columns and (entryindex + 1) not in args.columns:
          continue

        try:
          lines[lineindex] += entry.strip() + ';'
        except Exception as e:
          print(e)
          print(lineindex, ' / ', len(lines))
          print('initial # of columns:', len(lines))
          print('# of columns:', len(entries))

      lineindex += 1

with open(args.outputfile, 'w') as out:
  for line in lines:
    print(line, file=out)
