#!/bin/bash

ver=`curl -sL "https://api.github.com/repos/YouROK/TorrServer/releases/latest" | jq -r ".tag_name"`
if [[ "$ver" == "null" ]]
	then
		echo -e "\n!!! curl failed !!!\n"
		exit 1; 
	else
		source downloadAll.sh $ver
fi
