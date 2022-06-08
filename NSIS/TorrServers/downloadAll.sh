function dwnld {
	echo $1
	wget -N -q --show-progress "https://github.com/YouROK/TorrServer/releases/download/$1/TorrServer-windows-amd64.exe" -P "./$1"
	wget -N -q --show-progress "https://github.com/YouROK/TorrServer/releases/download/$1/TorrServer-windows-386.exe"   -P "./$1"
	echo
}

if [ -z "$1" ]
then
	find . -mindepth 1 -maxdepth 1 -type d -printf "%f\n" | while read dir; do dwnld "$dir"; done
else
	dwnld "$1"
fi



