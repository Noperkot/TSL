#!/bin/bash

find `dirname $0` -mindepth 1 -maxdepth 1 -type d | sort -n | while read dir; do source download.sh "$dir"; done || exit 1