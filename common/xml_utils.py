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

from os import walk
from os.path import isfile, isdir, getsize, join
from sys import stderr
from xml.sax import make_parser, handler

class TreeSizeHandler(handler.ContentHandler):
  """
  Helper class to figure out the size of a parsed tree wrt. number of nodes.
  """

  def __init__(self):
    self.tree_size = 0

  def startElement(self, name, attrs):
    for attr in attrs.keys():
      self.tree_size += 2

  def endElement(self, name):
    self.tree_size += 1

  def characters(self, content):
    self.tree_size += 1

  @property
  def tree_size(self):
    return self.__tree_size

  @tree_size.setter
  def tree_size(self, tree_size):
    self.__tree_size = tree_size

def gather_files(xml_dir):
  """
  Recursively gathers all files having a '.xml' file extension in the given
  directory and stores information about them in a list. The list is sorted wrt.
  the file size in MB.

  xml_dir The directory to scan recursively for '.xml' files.

  Returns a sorted list of dicts. An entry has the form {
    'path': <full-path-to-file>,
    'size_mb': <filesize-in-MB>,
    'size_nodes': <tree-size-in-nodes>}.
  """

  xml_parser = make_parser()
  tree_size_handler = TreeSizeHandler()
  xml_parser.setContentHandler(tree_size_handler)

  gathered_files = list()

  # scan directories recursively for xml files to be used for the experiments
  for (dirpath, dirnames, filenames) in walk(xml_dir):
    for filename in filenames:
      print('\t' + filename)

      if filename.endswith('.xml'):
        # parse xml file
        file_path = join(dirpath, filename)
        xml_parser.parse(file_path)

        gathered_files.append({
          # full path to file
          'path': file_path,
          # size in MB, rounded to 2 decimals
          'size_mb': round((getsize(join(dirpath, filename)) / (1024 * 1024)), 2),
          # size in nodes, not rounded since int anyways
          'size_nodes': tree_size_handler.tree_size
        })

        tree_size_handler.tree_size = 0

  print()

  # sort by file size (to get plots correct)
  return sorted(gathered_files, key=lambda k: k['size_mb'])
