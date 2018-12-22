nixcrpkgs:
rec {
  native = nixcrpkgs.native;

  nixpkgs = nixcrpkgs.nixpkgs;

  sdk = nixcrpkgs.macos.sdk;

  clang5 = native.make_derivation rec {
    name = "clang";

    version = "5.0.0";

    src = nixpkgs.fetchurl {
      url = "https://llvm.org/releases/${version}/cfe-${version}.src.tar.xz";
      sha256 = "0w09s8fn3lkn6i04nj0cisgp821r815fk5b5fjn97xrd371277q1";
    };

    llvm_src = nixpkgs.fetchurl {
      url = "https://llvm.org/releases/${version}/llvm-${version}.src.tar.xz";
      sha256 = "1nin64vz21hyng6jr19knxipvggaqlkl2l9jpd5czbc4c2pcnpg3";
    };

    # Note: We aren't actually using lld for anything yet.
    lld_src = nixpkgs.fetchurl {
      url = "http://releases.llvm.org/${version}/lld-${version}.src.tar.xz";
      sha256 = "15rqsmfw0jlsri7hszbs8l0j7v1030cy9xvvdb245397llh7k6ir";
    };

    patches = [ ./clang_megapatch.patch ];

    builder = ./clang_builder.sh;

    native_inputs = [ nixpkgs.python2 ];

    cmake_flags =
      "-DCMAKE_BUILD_TYPE=Release " +
      "-DLLVM_TARGETS_TO_BUILD=X86\;ARM " +
      "-DLLVM_ENABLE_RTTI=ON " +
      "-DLLVM_ENABLE_ASSERTIONS=OFF";
  };

  apple_tapi = native.make_derivation rec {
    name = "tapi";
    version = "${version0}.${version1}.${version2}";
    version0 = "2";
    version1 = "0";
    version2 = "0";
    src = nixpkgs.fetchurl {
      url = "https://github.com/DavidEGrayson/tapi/archive/f98d0c3.tar.gz";
      sha256 = "0jibz0fsyh47q8y3w6f0qspjh6fhs164rkhjg7x6k7qhlawcdy6g";
    };
    builder = ./apple_tapi_builder.sh;
    clang = clang5;
    native_inputs = [ clang ];
  };

  tinytapi = native.make_derivation rec {
    name = "tinytapi";
    include_dir = ../include;
    src_dir = ../src;
    builder = ./tinytapi_builder.sh;
    libyaml = nixpkgs.libyaml;
    native_inputs = [ libyaml ];
  };

  apple_tapi_dump = native.make_derivation rec {
    name = "apple-tapi-dump";
    builder = ./dump_builder.sh;
    src_file = ../dump/dump.cpp;
    native_inputs = [ apple_tapi ];
  };

  tinytapi_dump = native.make_derivation rec {
    name = "tinytapi-dump";
    builder = ./dump_builder.sh;
    src_file = ../dump/dump.cpp;
    native_inputs = [ tinytapi ];
  };
}