#! /bin/sh

pmodIA_EXE=../bin/pmodia.test.C.C
pmodIA_DAT=./datos

test -d $pmodIA_DAT || mkdir $pmodIA_DAT
cd $pmodIA_DAT
$pmodIA_EXE
