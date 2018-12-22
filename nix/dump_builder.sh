source $setup

CFLAGS="-Wall -Wextra -Wno-comment -O2 -std=c++14"
g++ $CFLAGS $src_file $(pkg-config --cflags --libs libtapi)

mkdir -p $out/bin
cp a.out $out/bin/$name