# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/include/expat-2.2.0"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src/expat-build"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/tmp"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src/expat-stamp"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src"
  "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src/expat-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src/expat-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/azureuser/kocher-sigmod2019-reproducibility/slim/build/include/expat-prefix/src/expat-stamp${cfgdir}") # cfgdir has leading slash
endif()
