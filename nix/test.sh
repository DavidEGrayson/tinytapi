source $setup

FILES="$sdk/usr/lib/*.tbd"

mkdir $out

echo appletapi
appletapi-dump $FILES > $out/appletapi.txt
echo tinytapi
tinytapi-dump $FILES > $out/tinytapi.txt
echo done