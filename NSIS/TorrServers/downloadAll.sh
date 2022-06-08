#!/bin/bash

baseurl="https://github.com/YouROK/TorrServer/releases/download"

function dwnld {
	wget -q --show-progress -c "$1" -P "$2"
	if [[ $? -ne 0 ]]; then
		echo -e "\n!!! wget failed !!!\n"
		exit 1; 
	fi
}

function dwnldAll {
	ver=`basename "$1"`
	echo $ver
	dwnld "$baseurl/$ver/TorrServer-windows-amd64.exe" "$1"
	dwnld "$baseurl/$ver/TorrServer-windows-386.exe"   "$1"	
	echo
}

if [ -z "$1" ]
	then
		find `dirname $0` -mindepth 1 -maxdepth 1 -type d | sort -n | while read dir; do dwnldAll "$dir"; done || exit 1
	else
		mkdir -p "$1"
		cat << EOF > "$1/README.md"
#### TorrServer executables should be here.  
***
TorrServer-windows-386.exe  
TorrServer-windows-amd64.exe  
EOF
		dwnldAll "$1"
fi

echo -e "Done\n"
sleep 2



