#!/usr/bin/python3

"""
The MIT License (MIT)
Copyright (c) 2019 Manuel Kocher, Daniel Kocher

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

# TODO(DK, MK):
# * Use subprocess.call (with lists, not concat. strings) instead of os.system
# * Remove unused variables + clean up

import os
import sys
import subprocess
import argparse

parser = argparse.ArgumentParser()

parser.add_argument(
    "--fig",
    type=int,
    default=0,
    help="figure number, i.e., which experiments to execute (0 = all figures)")
parser.add_argument(
    "--tmpdir",
    default="tmp/",
    help="directory to temporarily store the C++ binary")
parser.add_argument(
    "--resultdir",
    default="results/",
    help="directory to store the results")
parser.add_argument(
    "--libmemusage",
    default="/lib/x86_64-linux-gnu/libmemusage.so",
    help="path to the memusage shared library")
parser.add_argument(
    "--runs",
    type=int,
    default=1,
    help="nnumber of runs for a single experiment (robustness parameter)")
parser.add_argument(
    "--maxprocesses",
    type=int,
    default=1,
    help="max. number of parallel processes during experiments (robustness parameter)")
parser.add_argument(
    "--datasetsdir",
    default="datasets/",
    help="directory the documents are located in (all XML files will be used)")
parser.add_argument(
    "--queriesdir",
    default="queries/",
    help="directory the queries are located in (all XML files will be used)")
parser.add_argument(
    "--default_k",
    type=int,
    default=10,
    help="a single k value that is used in all experiments with fixed k")
parser.add_argument(
    "--vary_k",
    nargs='+',
    metavar='K-VALUES',
    default=[1, 10, 100],
    help="a list of all k values for varying-k experiments (Figure 16)")
parser.add_argument(
    "--updates",
    nargs='+',
    default=[10, 100, 1000, 10000, 100000, 1000000],
    help="a list of update counts for varying-number-of-updates experiments (Figures 12e and 12f)")

args = parser.parse_args()

################################################################################

updateop = ["rename", "delete"]
updates_fig12 = [ int(x) for x in args.updates ]

fig = args.fig
tmpdir = args.tmpdir
resultdir = args.resultdir
libmemusagepath = args.libmemusage
runs = args.runs
processes = args.maxprocesses

datasetspath = args.datasetsdir
queriespath = args.queriesdir
ks = [ int(x) for x in args.vary_k ]

slim_path = "slim/build/topk_queries"
tasm_struct_path = "tasm-struct/build/tree_similarity"

tasm_struct = {
    "TASM": "--tasm-postorder",
    "STRUCT": "--structure-search"
}

slim = {
    "SLIM": "--shincone",
    "SLIMU": "--shincone-updatable",
    "CONE": "--cone",
    "MERGE": "--merge"
}

#################
#   Figure 12   #
#################

figure_12_abcd_algos = {
    "SLIM": "--indexonly --shincone",
    "SLIMU": "--indexonly --shincone-updatable",
    "CONE": "--indexonly --cone"
}

figure_12_ef_algos = {
    "SLIMU": "--indexonly --shincone-updatable"
}

figure_12_ab = {
    "algos": figure_12_abcd_algos,
    "dir": "fig12_ab/"
}

figure_12_cd = {
    "algos": figure_12_abcd_algos,
    "dir": "fig12_cd/"
}

figure_12_e = {
    "algos": figure_12_ef_algos,
    "dir": "fig12_e/"
}

figure_12_f = {
    "algos": figure_12_ef_algos,
    "dir": "fig12_f/"
}

#################
#   Figure 13   #
#################

figure_13_ab = {
    "algos": slim,
    "dir": "fig13_ab/"
}

figure_13_cd = {
    "algos": slim,
    "dir": "fig13_cd/"
}

#################
#   Figure 14   #
#################

figure_14_15_16_algos_slim = {
    "SLIM": "--shincone",
    "SLIMU": "--shincone-updatable"
}

figure_14_ab_old = {
    "algos": tasm_struct,
    "dir": "fig14_ab/"
}

figure_14_ab_slim = {
    "algos": figure_14_15_16_algos_slim,
    "dir": figure_14_ab_old["dir"]
}

figure_14_cd_old = {
    "algos": tasm_struct,
    "dir": "fig14_cd/"
}

figure_14_cd_slim = {
    "algos": figure_14_15_16_algos_slim,
    "dir": figure_14_cd_old["dir"]
}

#################
#   Figure 15   #
#################

figure_15_ab_old = {
    "algos": tasm_struct,
    "dir": "fig15_ab/"
}

figure_15_ab_slim = {
    "algos": figure_14_15_16_algos_slim,
    "dir": figure_15_ab_old["dir"]
}

figure_15_cd_old = {
    "algos": tasm_struct,
    "dir": "fig15_cd/"
}

figure_15_cd_slim = {
    "algos": { "SLIM": "--shincone" },
    "dir": figure_15_cd_old["dir"]
}

#################
#   Figure 16   #
#################

figure_16_ab_old = {
    "algos": tasm_struct,
    "dir": "fig16_ab/"
}

figure_16_ab_slim = {
    "algos": figure_14_15_16_algos_slim,
    "dir": figure_16_ab_old["dir"]
}

figure_16_cd_old = {
    "algos": tasm_struct,
    "dir": "fig16_cd/"
}

figure_16_cd_slim = {
    "algos": { "SLIM": "--shincone" },
    "dir": figure_16_cd_old["dir"]
}

#################
#   Figure 12   #
#################

if fig in [0, 12]:
    # k and qsize are irrelevant as the binary terminates after index
    # construction, thus we fix k and use queries with 16 nodes (cf. symbolic
    # links in fig12_ab/fig12_cd/fig12_e/fig12_f in queries/)
    k = args.default_k

    # a) b)

    fig12_ab_resultdir = \
        os.path.join(resultdir, figure_12_ab["dir"])
    fig12_ab_datasetsdir = \
        os.path.join(datasetspath, figure_12_ab["dir"])
    fig12_ab_queriesdir = \
        os.path.join(queriespath, figure_12_ab["dir"])

    # cone, slim, slim-dyn
    for algo, arg in figure_12_ab["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                    " --resultdir " + fig12_ab_resultdir + \
                    " --runs " + str(runs) + \
                    " --k " + str(k) + \
                    " --maxprocesses " + str(processes) + \
                    " --libmemusagepath " + libmemusagepath + \
                    " " + fig12_ab_datasetsdir + \
                    " " + fig12_ab_queriesdir + \
                    " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") \
                    + " " + "--" + updateop[0] + "\"")
    # struct
    os.system("python3 profile.py --tmpdir " + tmpdir + \
                " --resultdir " + fig12_ab_resultdir + \
                " --runs " + str(runs) + \
                " --k " + str(k) + \
                " --maxprocesses " + str(processes) + \
                " --libmemusagepath " + libmemusagepath + \
                " " + fig12_ab_datasetsdir + \
                " " + fig12_ab_queriesdir + \
                " " + "\"STRUCT: " + (tasm_struct_path + " --indexonly " + tasm_struct["STRUCT"] + "\""))

    # c) d)

    fig12_cd_resultdir = \
        os.path.join(resultdir, figure_12_cd["dir"])
    fig12_cd_datasetsdir = \
        os.path.join(datasetspath, figure_12_cd["dir"])
    fig12_cd_queriesdir = \
        os.path.join(queriespath, figure_12_cd["dir"])

    # cone, slim, slim-dyn
    for algo, arg in figure_12_cd["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                    " --resultdir " + fig12_cd_resultdir + \
                    " --runs " + str(runs) + \
                    " --k " + str(k) + \
                    " --maxprocesses " + str(processes) + \
                    " --libmemusagepath " + libmemusagepath + \
                    " " + fig12_cd_datasetsdir + \
                    " " + fig12_cd_queriesdir + \
                    " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0")\
                    + " " + "--" + updateop[0] + "\"")
    # struct
    os.system("python3 profile.py --tmpdir " + tmpdir + \
              " --resultdir " + fig12_cd_resultdir + \
              " --runs " + str(runs) + \
              " --k " + str(k) + \
              " --maxprocesses " + str(processes) + \
              " --libmemusagepath " + libmemusagepath + \
              " " + fig12_cd_datasetsdir + \
              " " + fig12_cd_queriesdir + \
              " " + "\"STRUCT: " + (tasm_struct_path + " --indexonly " + tasm_struct["STRUCT"] + "\""))

    # e)

    fig12_e_resultdir = \
        os.path.join(resultdir, figure_12_e["dir"])
    fig12_e_datasetsdir = \
        os.path.join(datasetspath, figure_12_e["dir"])
    fig12_e_queriesdir = \
        os.path.join(queriespath, figure_12_e["dir"])

    for op in updateop:
        for update_size in updates_fig12:
            os.system("python3 profile.py --tmpdir " + tmpdir + \
                        " --resultdir " + os.path.join(fig12_e_resultdir, op + "/", str(update_size) + "/") + \
                        " --runs " + str(runs) + \
                        " --k " + str(k) + \
                        " --maxprocesses " + str(processes) + \
                        " --libmemusagepath " + libmemusagepath + \
                        " " + fig12_e_datasetsdir + \
                        " " + fig12_e_queriesdir + \
                        " " + "\"SLIMU: " + (slim_path + " --indexonly " + slim["SLIMU"] + " " + str(update_size)) \
                        + " " + "--" + op + "\"")

    # f)

    fig12_f_resultdir = \
        os.path.join(resultdir, figure_12_f["dir"])
    fig12_f_datasetsdir = \
        os.path.join(datasetspath, figure_12_f["dir"])
    fig12_f_queriesdir = \
        os.path.join(queriespath, figure_12_f["dir"])

    for op in updateop:
        for update_size in updates_fig12:
            os.system("python3 profile.py --tmpdir " + tmpdir + \
                      " --resultdir " + os.path.join(fig12_f_resultdir, op + "/", str(update_size) + "/") + \
                      " --runs " + str(runs) + \
                      " --k " + str(k) + \
                      " --maxprocesses " + str(processes) + \
                      " --libmemusagepath " + libmemusagepath + \
                      " " + fig12_f_datasetsdir + \
                      " " + fig12_f_queriesdir + \
                      " " + "\"SLIMU: " + (slim_path + " --indexonly " + slim["SLIMU"] + " " + str(update_size)) + \
                      " " + "--" + op + "\"")

#################
#   Figure 13   #
#################

if fig in [0, 13]:
    # fixed k, qsize (cf. symbolic links in fig13_ab/fig13_cd in queries/)
    k = args.default_k

    # a) b)

    fig13_ab_resultdir = \
        os.path.join(resultdir, figure_13_ab["dir"])
    fig13_ab_datasetsdir = \
        os.path.join(datasetspath, figure_13_ab["dir"])
    fig13_ab_queriesdir = \
        os.path.join(queriespath, figure_13_ab["dir"])

    for algo, arg in figure_13_ab["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig13_ab_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig13_ab_datasetsdir + \
                  " " + fig13_ab_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

    # c) d)

    fig13_cd_resultdir = \
        os.path.join(resultdir, figure_13_cd["dir"])
    fig13_cd_datasetsdir = \
        os.path.join(datasetspath, figure_13_cd["dir"])
    fig13_cd_queriesdir = \
        os.path.join(queriespath + figure_13_cd["dir"])

    for algo, arg in figure_13_cd["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig13_cd_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig13_cd_datasetsdir + \
                  " " + fig13_cd_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

#################
#   Figure 14   #
#################

if fig in [0, 14]:
    # fixed k, qsize (cf. symbolic links in fig14_ab/fig14_cd in queries/)
    k = args.default_k

    # a) b)

    fig14_ab_slim_resultdir =  \
        os.path.join(resultdir, figure_14_ab_slim["dir"])
    fig14_ab_slim_datasetsdir =  \
        os.path.join(datasetspath, figure_14_ab_slim["dir"])
    fig14_ab_slim_queriesdir =  \
        os.path.join(queriespath, figure_14_ab_slim["dir"])

    # slim, slim-dyn
    for algo, arg in figure_14_ab_slim["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig14_ab_slim_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig14_ab_slim_datasetsdir + \
                  " " + fig14_ab_slim_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

    fig14_ab_old_resultdir =  \
        os.path.join(resultdir, figure_14_ab_old["dir"])
    fig14_ab_old_datasetsdir =  \
        os.path.join(datasetspath, figure_14_ab_old["dir"])
    fig14_ab_old_queriesdir =  \
        os.path.join(queriespath, figure_14_ab_old["dir"])

    # tasm, struct
    for algo, arg in figure_14_ab_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig14_ab_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig14_ab_old_datasetsdir + \
                  " " + fig14_ab_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

    # c) d)

    fig14_cd_slim_resultdir = \
        os.path.join(resultdir, figure_14_cd_slim["dir"])
    fig14_cd_slim_datasetsdir = \
        os.path.join(datasetspath, figure_14_cd_slim["dir"])
    fig14_cd_slim_queriesdir = \
        os.path.join(queriespath, figure_14_cd_slim["dir"])

    # slim, slim-dyn
    for algo, arg in figure_14_cd_slim["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig14_cd_slim_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig14_cd_slim_datasetsdir + \
                  " " + fig14_cd_slim_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

    fig14_cd_old_resultdir = \
        os.path.join(resultdir, figure_14_cd_old["dir"])
    fig14_cd_old_datasetsdir = \
        os.path.join(datasetspath, figure_14_cd_old["dir"])
    fig14_cd_old_queriesdir = \
        os.path.join(queriespath, figure_14_cd_old["dir"])

    # tasm, struct
    for algo, arg in figure_14_cd_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig14_cd_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig14_cd_old_datasetsdir + \
                  " " + fig14_cd_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

#################
#   Figure 15   #
#################

if fig in [0, 15]:
    # fixed k, data set (cf. symbolic links in fig15_ab/fig15_cd in datasets/)
    k = args.default_k

    # a) b)

    fig15_ab_slim_resultdir = \
        os.path.join(resultdir, figure_15_ab_slim["dir"])
    fig15_ab_slim_datasetsdir = \
        os.path.join(datasetspath, figure_15_ab_slim["dir"])
    fig15_ab_slim_queriesdir = \
        os.path.join(queriespath, figure_15_ab_slim["dir"])

    # slim, slim-dyn
    for algo, arg in figure_15_ab_slim["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig15_ab_slim_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig15_ab_slim_datasetsdir + \
                  " " + fig15_ab_slim_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

    fig15_ab_old_resultdir = \
        os.path.join(resultdir, figure_15_ab_old["dir"])
    fig15_ab_old_datasetsdir = \
        os.path.join(datasetspath, figure_15_ab_old["dir"])
    fig15_ab_old_queriesdir = \
        os.path.join(queriespath, figure_15_ab_old["dir"])

    # tasm, struct
    for algo, arg in figure_15_ab_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig15_ab_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig15_ab_old_datasetsdir + \
                  " " + fig15_ab_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

    # c) d)

    fig15_cd_old_resultdir =  \
        os.path.join(resultdir, figure_15_cd_old["dir"])
    fig15_cd_old_datasetsdir =  \
        os.path.join(datasetspath, figure_15_cd_old["dir"])
    fig15_cd_old_queriesdir =  \
        os.path.join(queriespath, figure_15_cd_old["dir"])

    # tasm, struct
    for algo, arg in figure_15_cd_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig15_cd_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + str(k) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig15_cd_old_datasetsdir + \
                  " " + fig15_cd_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

    fig15_cd_slim_resultdir = \
        os.path.join(resultdir, figure_15_cd_slim["dir"])
    fig15_cd_slim_datasetsdir = \
        os.path.join(datasetspath, figure_15_cd_slim["dir"])
    fig15_cd_slim_queriesdir = \
        os.path.join(queriespath, figure_15_cd_slim["dir"])

    # slim
    os.system("python3 profile.py --tmpdir " + tmpdir + \
              " --resultdir " + fig15_cd_slim_resultdir + \
              " --runs " + str(runs) + \
              " --k " + str(k) + \
              " --maxprocesses " + str(processes) + \
              " --libmemusagepath " + libmemusagepath + \
              " " + fig15_cd_slim_datasetsdir + \
              " " + fig15_cd_slim_queriesdir + \
              " " + "\"SLIM: " + (slim_path + " " + figure_15_cd_slim["algos"]["SLIM"] + " 0") + \
              " " + "--" + updateop[0] +  "\"")

#################
#   Figure 16   #
#################

if fig in [0, 16]:
    fig16_ab_ks = " ".join(str(k) for k in ks)
    if 1000 not in ks:
        fig16_ab_ks += " 1000"
    if 10000 not in ks:
        fig16_ab_ks += " 10000"

    # a) b)

    fig16_ab_slim_resultdir = \
        os.path.join(resultdir, figure_16_ab_slim["dir"])
    fig16_ab_slim_datasetsdir = \
        os.path.join(datasetspath, figure_16_ab_slim["dir"])
    fig16_ab_slim_queriesdir = \
        os.path.join(queriespath, figure_16_ab_slim["dir"])

    # slim, slim-dyn
    for algo, arg in figure_16_ab_slim["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig16_ab_slim_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + fig16_ab_ks + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig16_ab_slim_datasetsdir + \
                  " " + fig16_ab_slim_queriesdir + \
                  " " + "\"" + algo + ": " + (slim_path + " " + arg + " 0") + \
                  " " + "--" + updateop[0] + "\"")

    fig16_ab_old_resultdir = \
        os.path.join(resultdir, figure_16_ab_old["dir"])
    fig16_ab_old_datasetsdir = \
        os.path.join(datasetspath, figure_16_ab_old["dir"])
    fig16_ab_old_queriesdir = \
        os.path.join(queriespath, figure_16_ab_old["dir"])

    # tasm, struct
    for algo, arg in figure_16_ab_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig16_ab_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + fig16_ab_ks + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig16_ab_old_datasetsdir + \
                  " " + fig16_ab_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

    # c) d)

    fig16_cd_old_resultdir = \
        os.path.join(resultdir, figure_16_cd_old["dir"])
    fig16_cd_old_datasetsdir = \
        os.path.join(datasetspath, figure_16_cd_old["dir"])
    fig16_cd_old_queriesdir = \
        os.path.join(queriespath, figure_16_cd_old["dir"])

    # tasm, struct
    for algo, arg in figure_16_cd_old["algos"].items():
        os.system("python3 profile.py --tmpdir " + tmpdir + \
                  " --resultdir " + fig16_cd_old_resultdir + \
                  " --runs " + str(runs) + \
                  " --k " + " ".join(str(k) for k in ks) + \
                  " --maxprocesses " + str(processes) + \
                  " --libmemusagepath " + libmemusagepath + \
                  " " + fig16_cd_old_datasetsdir + \
                  " " + fig16_cd_old_queriesdir + \
                  " " + "\"" + algo + ": " + (tasm_struct_path + " " + arg + "\""))

    fig16_cd_slim_resultdir = \
        os.path.join(resultdir, figure_16_cd_slim["dir"])
    fig16_cd_slim_datasetsdir = \
        os.path.join(datasetspath, figure_16_cd_slim["dir"])
    fig16_cd_slim_queriesdir = \
        os.path.join(queriespath, figure_16_cd_slim["dir"])

    # slim
    os.system("python3 profile.py --tmpdir " + tmpdir + \
              " --resultdir " + fig16_cd_slim_resultdir + \
              " --runs " + str(runs) + \
              " --k " + " ".join(str(k) for k in ks) + \
              " --maxprocesses " + str(processes) + \
              " --libmemusagepath " + libmemusagepath + \
              " " + fig16_cd_slim_datasetsdir + \
              " " + fig16_cd_slim_queriesdir + \
              " " + "\"SLIM: " + (slim_path + " " + figure_16_cd_slim["algos"]["SLIM"] + " 0") + \
              " " + "--" + updateop[0] + "\"")

#######################################
#    Extract all statistics files     #
#######################################

# since *all* statistics are extracted to separate files, we specify the
# names (prefixes) of files that we want to keep and delete all other files
try:
  prefixes = ["heap", "timingfiltering", "timingdelete", "timingrename", "timingtotal", "timingindexing", "verificationstotal", "outs.dict"]

  # loop over all serialized dictionaries, extract all statistics, and keep only
  # the files with one of the previously specified prefixes
  for dir in os.walk(resultdir):
      if len(dir[2]) != 0 and "outs.dict" in dir[2][0]:
          # create statistics files
          os.system("python3 mystatistics.py --resultdir \"" + dir[0] + "/\" \"" + os.path.join(dir[0], "outs.dict") + "\"")

          # delete unwanted files
          files = os.listdir(dir[0])
          for filename in files:
              fileprefix = filename.split('-')[0]
              if fileprefix not in prefixes:
                  os.remove(os.path.join(str(dir[0]), str(filename)))
except Exception as e:
  print("Experiments terminated correctly")
  print("Error while extracting the stat files")
  print(repr(e))
  raise

##############################################
#    Create result files for paper plots     #
#    12a/b/c/d, 13a/b/c/d, 14a/b/c/d,        #
#    15a/b/c/d, and 16a/b/c/d                #
##############################################

try:
  merge_filenames = dict()

  if fig in [0, 12]:
      merge_filenames[figure_12_ab["dir"]] = [
          "heap-peak-vary-document-size-k10.stat",
          "timingindexing-vary-document-size-k10.stat" ]
      merge_filenames[figure_12_cd["dir"]] = [
          "heap-peak-vary-document-size-k10.stat",
          "timingindexing-vary-document-size-k10.stat" ]

  if fig in [0, 13]:
      merge_filenames[figure_13_ab["dir"]] = [
          "timingfiltering-vary-document-size-k10.stat",
          "verificationstotal-vary-document-size-k10.stat"]
      merge_filenames[figure_13_cd["dir"]] = [
          "timingfiltering-vary-document-size-k10.stat",
          "verificationstotal-vary-document-size-k10.stat"]

  if fig in [0, 14]:
      merge_filenames[figure_14_ab_old["dir"]] = [
          "timingfiltering-vary-document-size-k10.stat",
          "verificationstotal-vary-document-size-k10.stat"]
      merge_filenames[figure_14_cd_old["dir"]] = [
          "timingfiltering-vary-document-size-k10.stat",
          "verificationstotal-vary-document-size-k10.stat"]

  if fig in [0, 15]:
      merge_filenames[figure_15_ab_old["dir"]] = [
          "timingfiltering-vary-query-size-k10.stat",
          "verificationstotal-vary-query-size-k10.stat"]
      merge_filenames[figure_15_cd_old["dir"]] = [
          "timingfiltering-vary-query-size-k10.stat",
          "verificationstotal-vary-query-size-k10.stat"]

  if fig in [0, 16]:
      merge_filenames[figure_16_ab_old["dir"]] = [
          "timingfiltering-vary-k-q16.stat",
          "verificationstotal-vary-k-q16.stat"]
      merge_filenames[figure_16_cd_old["dir"]] = [
          "timingfiltering-vary-k-q16.stat",
          "verificationstotal-vary-k-q16.stat"]

  need_header_transform = list()

  if fig in [0, 15]:
    need_header_transform.append(figure_15_cd_old["dir"])

  if fig in [0, 16]:
    need_header_transform.append(figure_16_cd_old["dir"])

  for fig_subdir, filenames in merge_filenames.items():
      # get immediate subdirectories
      current_subdir = os.path.join(resultdir, fig_subdir)
      immediate_subdirs = [ name for name in os.listdir(current_subdir) if os.path.isdir(os.path.join(current_subdir, name)) ]

      for filename in filenames:
          cmd = "python3 merge.py "

          if "vary-document-size" in filename:
              cmd += "--hassym "

          for immediate_subdir in immediate_subdirs:
              cmd += os.path.join(current_subdir, immediate_subdir, filename)
              cmd += " "

          resultfilepath = os.path.join(current_subdir, filename)
          cmd += resultfilepath
          os.system(cmd)

          cmd = "python3 replaceheader.py "

          if "vary-document-size" in filename:
              cmd += "--hassym "

          if fig_subdir in need_header_transform:
              cmd += "--transform-documents "

          cmd += resultfilepath
          os.system(cmd)

          if fig_subdir in need_header_transform:
              cmd = "python3 suffixcolumnorder.py {}".format(resultfilepath)
              os.system(cmd)

  # enrich result files of figures 12b and 12d with estimated memory consumption
  # of original StructureSearch publication (STRUCT-DEWEY line/bar)

  if fig in [0, 12]:
      filepaths = [
          os.path.join(
              resultdir,
              figure_12_ab["dir"],
              "heap-peak-vary-document-size-k10.stat"),
          os.path.join(
              resultdir,
              figure_12_cd["dir"],
              "heap-peak-vary-document-size-k10.stat") ]

      for filepath in filepaths:
          new_lines = list()
          with open(filepath, "r") as fin:
              for lineidx, line in enumerate(fin.readlines()):
                  if lineidx == 0:
                    new_lines.append(line.strip() + "STRUCTPLAIN;\n")
                  else:
                    document_size = float(line.strip().split(";")[0])
                    estimated_index_size = 10 * document_size * 1024 * 1024
                    new_lines.append(line.strip() + "{};\n".format(str(estimated_index_size)))

          with open(filepath, "w") as fout:
              fout.writelines(new_lines)

  # enrich result files of figures 14a and 14c with runtime for SLIM without a
  # pre-built index, i.e., the time to build the index *and* answer the query
  # (SLIM-NOINDEX line/bar)

  if fig in [0, 14]:
      filepaths = [
          ( os.path.join(resultdir, figure_14_ab_old["dir"]),
            os.path.join(
              resultdir,
              figure_14_ab_old["dir"],
              "timingfiltering-vary-document-size-k10.stat") ),
          ( os.path.join(resultdir, figure_14_cd_old["dir"]),
            os.path.join(
              resultdir,
              figure_14_cd_old["dir"],
              "timingfiltering-vary-document-size-k10.stat") ) ]

      for totalpath, filteringpath  in filepaths:
          new_lines = list()
          timingtotal_lines = list()

          for file in os.walk(totalpath):
              if file[0][-4:] == "SLIM":
                  with open(os.path.join(file[0],"timingtotal-vary-document-size-k10.stat"), "r") as fin:
                      for lineidx, line in enumerate(fin.readlines()):
                          if lineidx == 0:
                              timingtotal_lines.append("SLIMT;")
                          else:
                              timingtotal_lines.append(line.strip().split(";")[2])

          with open(filteringpath, "r") as fin:
              for lineidx, line in enumerate(fin.readlines()):
                  if lineidx == 0:
                    new_lines.append(line.strip() + timingtotal_lines[0].strip() + "\n")
                  else:
                    new_lines.append(line.strip() + "{};\n".format(timingtotal_lines[lineidx].strip()))

          with open(filteringpath, "w") as fout:
              fout.writelines(new_lines)
except Exception as e:
  print("Experiments terminated correctly")
  print("Error while merging and adapting the stat files")
  print(repr(e))
  raise


####################################################
#    Create result files for paper plots 12e/f     #
####################################################

try:
  if fig in [0, 12]:
      ops = {
          "rename" : "SLIMUr;",
          "delete" : "SLIMUd;"
      }

      for fig_subdir, defaulttime in [ (figure_12_e["dir"], "33881.060000000005"), (figure_12_f["dir"], "126049.1") ]:
          slim_from_scratch = defaulttime
          data_to_write = list()
          data_to_write.append("T;SLIMnew;")

          for dir in os.walk(os.path.join(resultdir, fig_subdir, "rename/10/")):
              if len(dir[2]) >= 1:
                  with open(os.path.join(dir[0], "timingindexing-vary-document-size-k10.stat")) as fin:
                      slim_from_scratch = fin.readlines()[1].split(";")[2]

          for up in updates_fig12:
              data_to_write.append(str(up) + ";" + slim_from_scratch + ";")

          for op in updateop:
              data_to_write[0] += ops[op] # extend header

              for dir in os.walk(os.path.join(resultdir, fig_subdir, op + "/")):
                  if len(dir[2]) >= 1:
                      timingfilename = "timing" + op + "-vary-k-q16.stat"
                      with open(os.path.join(dir[0], timingfilename), "r") as fin:
                        updates = dir[0].split("/")[-2]
                        timing = fin.readlines()[1].split(";")[1]

                        # write to index where the current updates are (+1 because of header line (T;SLIMU; ...))
                        data_index = updates_fig12.index(int(updates)) + 1
                        data_to_write[data_index] += timing + ";"

          timingfilename = "timingupdate-vary-ops-k10.stat"
          timingfilepath = os.path.join(resultdir, fig_subdir, timingfilename)
          with open(timingfilepath, "w") as fout:
              fout.writelines([x + "\n" for x in data_to_write])
except Exception as e:
  print("Experiments terminated correctly")
  print("Error while creating stat files for update plots (Fig. 12e/f)")
  print(repr(e))
  raise
