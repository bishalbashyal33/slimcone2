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

import sys
import argparse

parser = argparse.ArgumentParser()

parser.add_argument(
  '--hassym',
  action='store_true',
  default=False,
  help="set this to flag if the files to merge contain a column SYM")
parser.add_argument(
  '--transform-documents',
  action='store_true',
  default=False,
  help="transform header entries of TB/DBLP/SP from size values to their names")
parser.add_argument('statfile', help="the stat file to be modified")

args = parser.parse_args()

documents = dict({
  "82.09": "TB",
  "2161.17": "DBLP",
  "6137.29": "SP"
})

newfilecontent = ''

with open(args.statfile, 'r') as fp:
    for i, line in enumerate(fp):
        if i == 0:
            oldfirstline = line
            parts = oldfirstline.strip().split(';')[:-1]
            newfirstline = str(parts[0])

            start = 1
            if args.hassym:
                newfirstline += ';' + str(parts[1])
                start = 2

            try:
                suffixes = list()
                prefixes = list()
                for part in parts[start:]:
                    prefixes.append(part.strip().split(',')[0])
                    suffixes.append(part.strip().split(',')[1])

                # all suffixes are equal, cut them
                if len(set(suffixes)) <= 1 and not(args.transform_documents):
                    for prefix in prefixes:
                        newfirstline += ';' + prefix
                else:
                    newfirstline = oldfirstline

                    if args.transform_documents:
                        for size, name in documents.items():
                            newfirstline = newfirstline.replace(
                              "T = {}".format(size), name)
            except:
                newfirstline = oldfirstline

            newfilecontent += newfirstline.strip()
            newfilecontent += (';' if not newfilecontent.endswith(';') else '')
            newfilecontent += '\n'
        else:
            newfilecontent += line

with open(args.statfile, 'w') as fp:
    fp.write(newfilecontent)
