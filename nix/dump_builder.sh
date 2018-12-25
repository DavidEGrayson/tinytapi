source $setup

CFLAGS="-g -O0 -std=c++14 -Wall -Wextra -Wno-comment"
g++ $CFLAGS $src/dump.cpp $(pkg-config --cflags --libs libtapi)

mkdir -p $out/bin
cp a.out $out/bin/$name