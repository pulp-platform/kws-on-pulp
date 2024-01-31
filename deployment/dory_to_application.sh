#!/bin/bash

export NETWORK_DIR=$1
export SRC_DIR=$2

export CUR_DIR=$PWD

mkdir -p $NETWORK_DIR
cp -r $CUR_DIR/$SRC_DIR/hex $NETWORK_DIR
cp -r $CUR_DIR/$SRC_DIR/src $NETWORK_DIR
cp -r $CUR_DIR/$SRC_DIR/inc $NETWORK_DIR

# We use our own main.c file
rm $NETWORK_DIR/src/main.c

cp -r $NETWORK_DIR $CUR_DIR/../
