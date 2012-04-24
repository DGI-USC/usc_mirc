#!/bin/bash

SCRIPT=$0
FILE=$1

function imagemagick_identify() {
  DIRNAME=`dirname "$SCRIPT"`
  TEMPLATE=`cat "$DIRNAME/pbcore_from_dpx_via_imagemagick.xml"`
  identify -format "$TEMPLATE" "$FILE"
}

function add_size() {
  DIR=`dirname "$1"`
  SIZE=`du --apparent-size --exclude="!(*.dpx)" "$DIR"`
  SIZE=`cut -f1 <<< "$SIZE"`
  sed "s/\%\[total_size\]/$SIZE/"
}

PBCORE=`imagemagick_identify "$FILE"`
#echo "$PBCORE"
add_size "$FILE" <<< "$PBCORE"
