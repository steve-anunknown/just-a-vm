#!/bin/bash

# input must be binary file
binfile=$1
name=$(basename $binfile)

gzip -c $binfile | base64 > $name.b64
