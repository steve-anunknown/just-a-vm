#!/bin/bash

# input file must be b64
compressed=$1
name=$(basename $compressed)
base64 -d $compressed | zcat > $name.b
