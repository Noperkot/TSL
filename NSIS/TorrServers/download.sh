#!/bin/bash

baseurl="https://github.com/YouROK/TorrServer/releases/download"

function dwnld {
	wget -q --show-progress -c "$1" -P "$2"
	if [[ $? -ne 0 ]]; then
		echo -e "\n!!! wget failed !!!\n"
		exit 1; 
	fi
}

function dwnldAllBinary {
	ver=`basename "$1"`
	echo $ver
	mkdir -p "$1"	
	if [ ! -f "$1/README.md" ]; then
		echo -e "#### TorrServer executables should be here.  \n***\nTorrServer-windows-386.exe  \nTorrServer-windows-amd64.exe  " > "$1/README.md"
	fi
	dwnld "$baseurl/$ver/TorrServer-windows-amd64.exe" "$1"
	dwnld "$baseurl/$ver/TorrServer-windows-386.exe"   "$1"	
	echo
}

if [ -z "$1" ]
	then # download last
		ver=`curl -sL "https://api.github.com/repos/YouROK/TorrServer/releases/latest" | jq -r ".tag_name"`
		if [[ "$ver" == "null" ]]
			then
				echo -e "\n!!! curl failed !!!\n"
				exit 1; 
			else
				dwnldAllBinary $ver
		fi
	else
		dwnldAllBinary "$1"
fi

echo -e "Done\n"
sleep 2



