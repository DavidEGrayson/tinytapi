source $setup

mkdir -p $out

tar -xf $src
srcd=$out/src
mv tapi-* $srcd

mkdir build
cd build

mkdir -p include/tapi/{Core,Driver}
cat > include/tapi/Core/ArchitectureConfig.h <<EOF
#pragma once
#define SUPPORT_ARCH_I386 1
#define SUPPORT_ARCH_X86_64 1
#define SUPPORT_ARCH_X86_64H 1
EOF

cat > include/tapi/Version.inc <<EOF
#pragma once
#define TAPI_VERSION $version
#define TAPI_VERSION_MAJOR $version0
#define TAPI_VERSION_MINOR $version1
#define TAPI_VERSION_PATCH $version2
EOF

clang-tblgen -I$clang/include -gen-clang-diags-defs \
  -o include/tapi/Driver/DiagnosticTAPIKinds.inc \
  $srcd/include/tapi/Driver/DiagnosticTAPIKinds.td

llvm-tblgen -I$clang/include -gen-opt-parser-defs \
  -o include/tapi/Driver/TAPIOptions.inc \
  $srcd/include/tapi/Driver/TAPIOptions.td

function build-lib() {
  libsrc=$srcd/$1
  lib=$2
  mkdir $lib.o
  for f in $libsrc/*.cpp; do
    echo "compiling ${f/$srcd\/}"
    g++ -c $CFLAGS $f -o $lib.o/$(basename $f).o
  done
  echo "archiving $lib"
  ar cr $lib $lib.o/*.o
}

CFLAGS="-g -O0 -std=gnu++11 -Iinclude -I$srcd/include -I$clang/include"

build-lib tools/libtapi libtapi.a
build-lib lib/Config libtapiConfig.a
build-lib lib/Core libtapiCore.a
build-lib lib/Driver libtapiDriver.a
build-lib lib/Scanner libtapiScanner.a
build-lib lib/SDKDB libtapiSDKDB.a

CLANG_LIBS="-lclangTooling -lclangFrontend -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangAST -lclangAnalysis -lclangEdit -lclangLex -lclangBasic -lLLVMDemangle -lLLVMObject -lLLVMBitReader -lLLVMMC -lLLVMMCParser -lLLVMCore -lLLVMBinaryFormat -lLLVMOption -lLLVMProfileData -lLLVMSupport"

LDFLAGS="-L$clang/lib -L. -ltapi -ltapiDriver -ltapiCore -ltapiDriver -ltapiScanner -ltapiSDKDB -ltapiConfig $CLANG_LIBS -lpthread"

echo "building tapi"
g++ $CFLAGS $srcd/tools/tapi/tapi.cpp $LDFLAGS -o tapi

echo "building tapi-run"
g++ $CFLAGS $srcd/tools/tapi-run/tapi-run.cpp $LDFLAGS -o tapi-run

mkdir -p $out/lib/pkgconfig $out/bin
cp *.a $out/lib
cp tapi tapi-run $out/bin

cp -r $srcd/include $out/

cp include/tapi/Version.inc $out/include/tapi/

cat > $out/lib/pkgconfig/libtapi.pc <<EOF
prefix=$out
libdir=\${prefix}/lib
includedir=\${prefix}/include

Version: $version
Libs: -L\${libdir} -L$clang/lib -ltapi -ltapiCore $CLANG_LIBS -lpthread
Cflags: -I\${includedir}
EOF
