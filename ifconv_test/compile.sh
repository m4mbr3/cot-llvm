#!/bin/bash
FILECPP=$1".c"
FILEBC=$1".bc"
FILEBC1=$1".bc1"
FILEBC2=$1".bc2"
FILEBC3=$1".bc3"
FILEBC4=$1".bc4"
FILEOBJ=$1".o"
FILEOBJ1=$1".o1"
FILEEXE=$1
$LLVM_ROOT/bin/clang -emit-llvm -c $FILECPP -o $FILEBC
$LLVM_ROOT/bin/llvm-link  $FILEBC -o $FILEBC1 
$LLVM_ROOT/bin/opt -mem2reg $FILEBC1 -o $FILEBC2 
$LLVM_ROOT/bin/llc -march=hexagon -debug -disable-if-converter -enable-if-convertion-pre-reg-allocation -enable-psi-elimination -print-after-all $FILEBC2
