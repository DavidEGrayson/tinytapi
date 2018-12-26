source $setup

FILES="$sdk/usr/lib/*.tbd"

mkdir $out
cd $out

echo appletapi
appletapi-dump $FILES > appletapi.txt
echo tinytapi
tinytapi-dump $FILES > tinytapi.txt

diff appletapi.txt tinytapi.txt > /dev/null && echo success || echo fail