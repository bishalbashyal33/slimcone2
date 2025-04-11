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

import re
import sys
import pprint

"""
Utility functions.
"""

def traversedictlevels(d, levelkeys, putmissingkeys=False):
  t = d
  for levelkey in levelkeys:
    if putmissingkeys and levelkey not in t:
      t[levelkey] = dict()

    if levelkey in t:
      t = t[levelkey]
    else:
      break

  return t

def unrolldictkeys(d, levels=None, printkeys=False):
  if printkeys:
    print('dict keys:')
    pp = pprint.PrettyPrinter(indent=2)

  t = d.copy()
  processed = 0
  while isinstance(t, dict):
    if printkeys:
      print('level', processed)

    keylist = list(t.keys())
    
    if printkeys:
      pp.pprint(keylist)
    
    t = t[keylist[0]]

    processed += 1

    if levels is not None and processed >= levels:
      break

  return list(t.keys())

def pprintdictkeys(d, levels=2):
  unrolldictkeys(d, levels, True)

def unrolldict(root):
  dicts = [([], root)]
  unrolled = dict()
  seen = set()

  for path, nested in dicts:
    if id(nested) in seen:
      continue

    seen.add(id(nested))

    for key, value in nested.items():
      newpath = path + [key]
      prefix = key

      if isinstance(value, dict):
        dicts.append((newpath, value))
      else:
        unrolled[prefix] = value

  return unrolled

def ranking_to_clustered_dict(l):
  """
  Converts a given list of lists l (representing a top-k ranking into a
  clustered dict.

  l The ranking (list of lists) to be converted to a clustered dict.

  Returns a dict mapping dn to a list of strings of all id_i that have dn as
  key.
  """

  clustered_dict = dict()

  for e in l:
    key = e[0]
    value = e[1]

    if value in clustered_dict:
      clustered_dict[value].append(key)
    else:
      clustered_dict[value] = [key]

  return clustered_dict

def lazy_compare_dicts(lhs, rhs):
  """
  Compares two given dicts, lhs and rhs, lazily. Lazy in this context means
  that all but the largest key are compared exactly. The largest key is
  compared lazily in the sense that only the number of mapped values are
  compared. This comparison function is useful for testing top-k query
  results.

  lhs The first dict to compare.
  rhs The second dict to compare.

  Returns True if lhs and rhs are equal in the sense of lazy comparison, False
  otherwise.
  """

  if len(lhs) != len(rhs):
    return False

  index = 1
  for lhs_lower_bound in sorted(lhs.keys(), key=lambda distance: int(float(distance))):
    lhs_elements = lhs[lhs_lower_bound]

    if lhs_lower_bound not in rhs:
      return False

    rhs_elements = rhs[lhs_lower_bound]

    if index < len(lhs):
      # exact compare for all but the last key
      if sorted(lhs_elements) != sorted(rhs_elements):
        return False
    else:
      # lazy compare of last key
      if len(lhs_elements) != len(rhs_elements):
        return False

    index += 1

  return True

def get_equal_attribute_min_count(dict_list, attribute='size_nodes'):
  """
  Retrieves the minimum count of a value mapped from a given attribute in a
  given list of dictionaries (dict_list).

  dict_list A list of dictionaries that is searched.
  attribute The attribute to be searched for.

  Returns the minimum count of a value mapped from attribute in dict_list.
  """

  unique = dict()

  for entry in dict_list:
    if entry[attribute] in unique:
      unique[entry[attribute]] += 1
    else:
      unique[entry[attribute]] = 1

  max_key = min(unique.keys(), key=(lambda key: unique[key]))

  return unique[max_key]

def extract_rounds_tables_topk(
    all_outs,
    sorted_datasets,
    sorted_queries,
    sorted_ks,
    prefix='Round',
    output_to_search='stdout'):
  tables = dict() 
 
  re_prefix = re.escape(prefix)

  for dataset in sorted_datasets:
    for query in sorted_queries:
      for k in sorted_ks:
        rounds_outs = all_outs[(dataset['path'], query['path'], k)][output_to_search]

        #print(type(rounds_outs), len(rounds_outs))
        
        round_dict = dict()
        round_dict[(dataset['path'], query['path'], k)] = dict()        

        for round_out in rounds_outs:
          split_rounds = [x.strip() for x in re.split(prefix + '\s*(\d+)', round_out)][1:]

          all_rounds = dict()
          for split_round in split_rounds:
            if split_rounds.index(split_round) % 2 != 0:
              all_rounds[int(split_rounds[split_rounds.index(split_round) - 1])] = [x.strip() for x in split_round.split('\n')]
          
          for key, val in all_rounds.items():
            if key not in round_dict: 
              round_dict[(dataset['path'], query['path'], k)][key] = list()
          
            round_dict[(dataset['path'], query['path'], k)][key] += [val]
        
        #print(round_dict)
        #tables[round] = out_rounds
  

def extract_tables_topk(
    all_outs,
    sorted_datasets,
    sorted_queries,
    sorted_ks,
    prefix='timing filter',
    output_to_search='stdout'):
  """
  Retrieves tables containing the values associated with the prefix in the
  stdout of unit-under-evaluation and the witness.

  all_outs        A two dimensional dictionary mapping
                  [(dataset-path, query-path, k)][binary] entries to stdout of
                  the corresponding execution of binary.
  sorted_datasets A sorted list of all datasets.
  sorted_queries  A sorted list of all queries.
  sorted_ks       A sorted list of all values of k.
  prefix          A string prefix for which the entries in outs is searched for,
                  e.g., if one wants to extract the time from an stdout
                  containing a substring of the form 'querying time: 3.14', the
                  prefix needs to be 'querying time'.

  Returns two dictionaries (one for the unit under evaluation, one for the
  witness) that map entries of the form (dataset-path, query-path, k) to the
  extracted value (from the corresponding entry in outs).

  TODO: make more generic (i.e., extract a single table)
  TODO: update documentation
  """

  tables = dict()

  re_prefix = re.escape(prefix)

  for dataset in sorted_datasets:
    for query in sorted_queries:
      for k in sorted_ks:
        value = extract_value(
          all_outs[(dataset['path'], query['path'], k)][output_to_search],
          dataset['path'],
          query['path'],
          re_prefix)

        if value:
          tables[(dataset['path'], query['path'], k)] = value

  return tables

def extract_value(outs, dataset, query, re_prefix):
  """
  TODO
  """

  re_full = r'\s*' + re_prefix + r'[a-zA-Z0-9\-/\.\(\)_,\s]*:\s*(\d+\.*\d*)'

  run_values = list()

  #print(type(outs), len(outs))

  for out in outs:
    try:
      r = re.search(re_full, out)
      value = float(r.group(1))
      #unit = r.group(3)

      run_values.append(value)

    except AttributeError:
      sys.stderr.write('AttributeError in extract_time: Unable to'
        ' find regular expression ' + re_full + ' in ' + out)
      continue

  avg_value = sum(run_values) / len(run_values) if run_values else None

  return avg_value

def get_subdict(dictionary, criteria, resultKey, criteriaKey):
  """
  TODO
  """

  subdictionary = dict()

  for criterium in criteria:
    subdictionary[criterium] = \
      [p[resultKey] for p in dictionary if p[criteriaKey] == criterium]
    subdictionary[criterium] = sorted(subdictionary[criterium])

  return subdictionary

def last_level_from_unix_path(path):
  return path[path.rfind('/') + 1:]
