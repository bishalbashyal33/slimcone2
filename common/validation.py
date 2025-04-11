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

"""
TODO:
  - make tempfile option (to avoid PIPE bufsize overflow hang)
"""

import common.utils as utils
import common.exec_utils as exec_utils

import sys
import os
import re
import json

def validate_topk(all_witness_outs, all_uue_outs, terminateonfailure=False):
  """
  TODO
  """

  print('Validating outputs.')
  sys.stdout.flush()

  witness_name = all_witness_outs[0]
  witness_results = all_witness_outs[1]

  uue_name = all_uue_outs[0]
  uue_results = all_uue_outs[1]

  error_message = ''  

  for instance_triple, witness_execution in witness_results.items():
    # extract dataset, query, and k from instance_triple
    dataset = instance_triple[0]
    query = instance_triple[1]
    k = instance_triple[2]

    # extract output of witness
    witness_outs_runs = witness_execution['stdout']

    # get entry from uue_outs
    uue_execution = uue_results[instance_triple]
    # extract output of uue
    uue_outs_runs = uue_execution['stdout']

    print('Validating instance (' + uue_name + ', ' + witness_name +
      ', T=' + utils.last_level_from_unix_path(dataset) +
      ', Q=' + utils.last_level_from_unix_path(query) +
      ', k=' + str(k),
      end=') ')

    validation_result = False
    for uue_out_run, witness_out_run in zip(uue_outs_runs, witness_outs_runs):
      validation_result = validate_topk_rankings(
        uue_out_run,
        witness_out_run,
        dataset,
        query)

      if validation_result is False:
        print('['+ '\x1b[6;30;41m' + 'FAILURE' + '\x1b[0m' + ']')
        
        error_message += 'Error while validating outputs for instance (' + \
          uue_name + ', ' + witness_name + \
          ', T=' + utils.last_level_from_unix_path(dataset) + \
          ', Q=' + str(query) + \
          ', k=' + k + ').\n'

        if terminateonfailure:
          return False, error_message

    if validation_result is not False:
      print('[' + '\x1b[6;30;42m' + 'SUCCESS' + '\x1b[0m' + ']')

  if error_message != '':
    return False, error_message
  
  return True, None

def validate_topk_rankings(uue_stdout, witness_stdout, dataset, query):
  """
  Validates two given outputs (i.e., strings), uue_stdout and witness_stdout,
  wrt. the top-k rankings. Each output is searched for a substring of the form
  'ranking(<query-path>, <dataset-path>):' and remainder of the string is then
  taken as ranking string. The ranking strings of uue and witness are then
  forwarded to the validate_strings function, using its default arguments for
  conversion_function and comparison_function.

  uue_stdout      The output of the unit under evaluation.
  witness_stdout  The output of the witness.
  dataset         The path of the dataset the ranking is validated of.
  query           The path of the query the ranking is validated of.

  Returns True if the rankings are equal by definition of lazy_compare_dicts,
  False otherwise.
  """

  #try:
  uue_json = json.loads(uue_stdout)
  witness_json = json.loads(witness_stdout)
  #except Exception as e:
    #print(uue_stdout)
    #print(witness_stdout)

  uue_ranking = None
  witness_ranking = None

  try:
    uue_ranking = uue_json['ranking']
  except Exception as ex:
    sys.stderr.write('Unable to find key \"ranking\" in UUE result\n')

  try:
    witness_ranking = witness_json['ranking']
  except Exception as ex:
    sys.stderr.write('Unable to find key \"ranking\" in UUE result\n')

  #print('uue ranking: ', uue_ranking)
  #print('witness ranking', witness_ranking)

  v = validate_rankings(uue_ranking, witness_ranking)
 
  return v

def validate_rankings(
    uue,
    witness,
    conversion_function=utils.ranking_to_clustered_dict,
    comparison_function=utils.lazy_compare_dicts):
  """
  Validates two given strings, uue_str and witness_str, by
    (1) applying conversion_function to both strings and
    (2) applying compare_function to the results of the conversions.

  uue                 The ranking as list of the unit under evaluation.
  witness             The ranking as list of the witness.
  conversion_function The conversion function to be applied on both
                      rankings/lists. Must only take the ranking/list as single
                      argument.
  comparison_function The comparison function to be applied afterwards.
                      Must only take the two converted values as arguments.

  Returns True if the converted values are equal by definition of
  comparison_function, False otherwise.
  """

  uue = conversion_function(uue)
  witness = conversion_function(witness)

  return comparison_function(uue, witness)
