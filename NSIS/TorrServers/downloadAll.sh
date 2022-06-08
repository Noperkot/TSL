#!/bin/bash

baseurl="https://github.com/YouROK/TorrServer/releases/download"

function dwnld {
	ver=`basename "$1"`
	echo $ver
	wget -q --show-progress -c "$baseurl/$ver/TorrServer-windows-amd64.exe" -P "$1"
	wget -q --show-progress -c "$baseurl/$ver/TorrServer-windows-386.exe"   -P "$1"
	echo
}

if [ -z "$1" ]
	then
		find `dirname $0` -mindepth 1 -maxdepth 1 -type d | sort -n | while read dir; do dwnld "$dir"; done
	else
		dwnld "$1"
fi

echo -e "Done\n"
sleep 2



