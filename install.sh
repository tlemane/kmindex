#!/bin/bash

function kmindex_build ()
{
  mkdir kmbuild
  cd kmbuild

  cmake .. -DCMAKE_BUILD_TYPE=${1} \
           -DWITH_TESTS=${2} \
           -DPORTABLE_BUILD=${4} \
           -DCMAKE_INSTALL_PREFIX=$(realpath ${6}) \
           -DMAX_KMER_SIZE=${7}

  make -j${5}

  if [[ ${3} == 1 ]]; then
    ctest --verbose
  fi

  make install
}

function usage ()
{
  echo "kmindex build script - v0.5.3."
  echo "Usage: "
  echo "  ./install.sh [-r str] [-t int] [-j int] [-p str] [-k int] [-n] [-h]"
  echo "Options: "
  echo "  -r <Release|Debug> -> build type {Release}."
  echo "  -t <0|1|2>         -> tests: 0 = disabled, 1 = compile, 2 = compile and run {0}."
  echo "  -j <INT>           -> nb threads {8}."
  echo "  -k <INT>           -> Max k-mer size (should be a multiple of 32) {256}"
  echo "  -n                 -> portable x86-64 build {disabled}."
  echo "  -p                 -> install path {./kmindex_install}"
  echo "  -h                 -> show help."
  exit 1
}

rel="Release"
deb="Debug"

mode="Release"
static="OFF"
tests=0
tests_str="OFF"
tests_run=1
jopt=8
native="OFF"
installp=$(realpath ./kmindex_install)
max_ksize=256

while getopts "k:r:t:j:p:nh" option; do
  case "$option" in
    r)
      mode=${OPTARG}
      [[ ${mode} != ${rel} && ${mode} != ${deb} ]] && usage
      ;;
    t)
      tests=${OPTARG}
      [[ ${tests} == 0 ]] || [[ ${tests} == 1 ]] || [[ ${tests} == 2 ]] || usage
      [[ ${tests} == 0 ]] && tests_run=0
      [[ ${tests} == 0 ]] && tests_str="OFF"
      [[ ${tests} == 1 ]] && tests_run=0
      [[ ${tests} == 1 ]] && tests_str="ON"
      [[ ${tests} == 2 ]] && tests_run=1
      [[ ${tests} == 2 ]] && tests_str="ON"
      ;;
    j)
      jopt=${OPTARG}
      ;;
    n)
      native="ON"
      ;;
    p)
      installp=${OPTARG}
      ;;
    k)
      max_ksize=${OPTARG}
      ;;
    h)
      usage
      ;;
    *)
      usage
      ;;
  esac
done

kmindex_build ${mode} ${tests_str} ${tests_run} ${native} ${jopt} ${installp} ${max_ksize}
