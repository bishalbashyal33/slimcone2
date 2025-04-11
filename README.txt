================================================================================
Readme for reproducibility submission of paper ID mod567
Title: A Scalable Index for Top-k Subtree Similarity Queries

A) Source code info

Repository:
  https://kitten.cosy.sbg.ac.at/index.php/s/fjT3eQ76JekgAK3/download

Programming Language:
  C++ (algorithms) and python3 (experiments)

Additional Programming Language info:
  C++11

Compiler Info:
  clang version 3.8.1-24 (tags/RELEASE_381/final)
  Target: x86_64-pc-linux-gnu
  Thread model: posix
  Optimization flags:
    -O3 -std=c++11 -Wall -Wextra -pedantic-errors -DNO_LOG -DNO_INALGO_TIMINGS

Packages/Libraries Needed:
  wget
  tar
  ansible (version >= 2.2.1.0; main file to run all experiments; will then
  install all following dependencies automatically)
    python3
    clang-3.8
    cmake
    make
    texlive-full
    latexmk

B) Datasets info
Repository:
  data sets: https://kitten.cosy.sbg.ac.at/index.php/s/jYJC2xzPCNnjJZD/download
  queries: https://kitten.cosy.sbg.ac.at/index.php/s/m4JixE8xXKG7xkM/download

Data generators:
  None

C) Hardware Info
  There are no special hardware requirements except that the machine should have
  at least 40 GiB of main memory (if experiments are executed sequentially). We
  ran our experiments on a machine with 96GB of main memory and the
  specification below is based on this machine (C1 - C5).

C1) Processor
  2x Intel(R) Xeon(R) CPUs E5-2630 v3 2.40 GHz
  8 cores per physical processor (=> 16 logical processors)

C2) Caches
  3 cache levels
  L1d: 32 KiB
  L1i: 32 KiB
  L2: 256 KiB
  L3: 20.480 KiB

C3) Memory
  96 GiB of main memory @ 2.133 MHz (1.866 MHz configured clock speed)

C4) Secondary Storage
  2x 1.8 TiB HDDs as secondary storage, theoretical performance characteristics:
    * read (cache miss/hit): 0,5/0,1ms
    * write: 0,015ms
    * seek: 0,5ms

C5) Network
  Not used, all experiments are executed locally.

D) Experimentation Info

D1) Scripts and how-tos to generate all necessary data or locate datasets:
	See D3.

D2) Scripts and how-tos to prepare the software for system
	See D3.

D3) Scripts and how-tos for all experiments executed for the paper
	After downloading the reproducibility package from

    https://kitten.cosy.sbg.ac.at/index.php/s/fjT3eQ76JekgAK3/download

  and extracting it into a directory sigmod2019-reproducibility, cd into this
  directory and run

    ansible-playbook -K run_all.yaml

  "-K" asks for sudo password to install software packages  (hit "Enter" if you
  are root). Cf. report.pdf for detailed information on the arguments that can
  be passed to run_all.yaml.

================================================================================