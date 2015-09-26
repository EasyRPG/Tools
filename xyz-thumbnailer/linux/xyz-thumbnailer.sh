#!/bin/bash

# Copyright (c) 2015 xyz-thumbnailer authors
# Copyright (c) 2015 carstene1ns <dev@f4ke.de>
# This file is released under the MIT License
# http://opensource.org/licenses/MIT

# enable for debugging
#set -x

# helper functions
function error_out {
	echo $1
	exit 1
}
function cleanup {
	rm -rf "$TEMPFOLDER"
}

# check arguments
if [ $# -lt 2 -o $# -gt 3 ]; then
	error_out "Usage: xyz-thumbnailer path/to/input.xyz path/to/output.png [size in pixels]"
fi
INPUT=$1
OUTPUT=$2
SIZE=${3:-128}
[ -s "$INPUT" ] || error_out "Input file not found!"
INPUTLC=${INPUT,,}
[ "$INPUTLC" == "${INPUTLC%.xyz}" ] && echo "Input file has not XYZ extension, continuing anyway!"
[[ $SIZE == +([0-9]) ]] || error_out "Size argument is not a valid number!"
PIXELCOUNT=$(($SIZE*$SIZE))

# setup a temporary folder to work in
TEMPFOLDER=$(mktemp -q --tmpdir -d xyz-thumbnailer.XXXX)
[ -d "$TEMPFOLDER" ] || error_out "Could not create temporary folder!"
# setup cleanup hook
trap cleanup EXIT

cp "$INPUT" "$TEMPFOLDER"/image.xyz
pushd "$TEMPFOLDER" > /dev/null

# convert!
xyz2png image.xyz
[ -s image.png ] || error_out "Could not convert xyz file to png!"
convert image.png -thumbnail ${PIXELCOUNT}@ -gravity center -background transparent -extent ${SIZE}x${SIZE} output.png
[ -s output.png ] || error_out "Could not convert to thumbnail!"

popd > /dev/null
OUTPUTFOLDER=$(dirname "$OUTPUT")
if [ ! -d "$OUTPUTFOLDER" ]; then
	echo "Output folder does not exist, creating it!"
	mkdir -p "$OUTPUTFOLDER"
fi
cp "$TEMPFOLDER"/output.png "$OUTPUT"

# all good
exit 0
