TinyTAPI - Tiny library for working with MacOS TBD files.

This library is not ready to be used by anyone yet.

The goal is to eventually make a tiny version of the
[tapi library](https://github.com/DavidEGrayson/tapi)
that is easier to build and does not depend on Clang and LLVM.

The tapi library is highly coupled with Clang and LLVM.  For example,
it requires the clang-tblgen tool, which is an internal part
of Clang.  I was able to get it working with Clang and LLVM 5.0.0,
but not with 7.0.0.
