csvimp - a CSV Import Utility
=============================

[![Build Status on xtuple's master](https://travis-ci.org/xtuple/csvimp.svg?branch=master)](https://travis-ci.org/xtuple/csvimp)

The following points should be considered when working with 
CSVimp source code:

* Qt must be installed and working properly to successfully compile
  CSVimp.
* CSVimp uses OpenRPT's "common" library. Therefore you need either
  the OpenRPT source code or OpenRPT development packages installed
  on your Linux box.

To build with OpenRPT sources, place the `openrpt` and `csvimp`
directories in the same parent directory and compile in `openrpt`
first. You can fork or directly clone from https://github.com/xtuple/openrpt .
You might also need to set `BUILD_SHARED_LIBS=true` in your environment before
building in `openrpt`.

To build with installed OpenRPT packages, set the following
environment variables:
- `OPENRPT_HEADERS` names the directory where the OpenRPT header files are
  installed
- `OPENRPT_LIBDIR` names the directory where the OpenRPT libraries are
  installed
