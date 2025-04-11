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
import csv
import argparse

parser = argparse.ArgumentParser()

parser.add_argument('statfile', help="the stat file to be modified")

args = parser.parse_args()

new_lines = list()

with open(args.statfile, "r") as fin:
    column_count = 0
    data_column_count = 0
    data_set_count = 0

    for lineidx, line in enumerate(fin.readlines()):
        entries = line.strip().split(";")

        if entries[-1] == '':
            entries = entries[:-1]

        if lineidx == 0:
            column_count = len(entries)
            data_column_count = column_count - 1

            suffixes = set()
            for entry in entries[1:]:
                entry_parts = entry.split(',')
                try:
                    suffixes.add(entry_parts[1])
                except:
                    pass # just continue

            data_set_count = len(suffixes)
            column_count_per_data_set = int(data_column_count / data_set_count)

        new_line = [ None ] * column_count
        new_lineidx = 0

        new_line[new_lineidx] = entries[new_lineidx]
        new_lineidx += 1

        for i in range(column_count_per_data_set):
            for j in range(data_set_count):
                new_line[new_lineidx] = entries[(i + 1) + j * data_set_count]
                new_lineidx += 1

        new_lines.append(";".join(new_line) + ";\n")

with open(args.statfile, "w") as fout:
    fout.writelines(new_lines)
