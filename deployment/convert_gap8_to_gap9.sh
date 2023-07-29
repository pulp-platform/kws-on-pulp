#!/bin/bash

export NETWORK_DIR=$1

export CUR_DIR=$PWD

cd gap8_to_gap9

mkdir -p $NETWORK_DIR
cp -r $CUR_DIR/application/hex $NETWORK_DIR
cp -r $CUR_DIR/application/src $NETWORK_DIR
cp -r $CUR_DIR/application/inc $NETWORK_DIR

python convert_gap9.py gap8to9_templates $NETWORK_DIR 

# We use our own main.c file
rm $NETWORK_DIR/src/main.c