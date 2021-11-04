#!/bin/sh
set -x
export CXX="g++"
# comment following line to get optimized code. (-O2 is default).
# No -O2 is for compilation speed and good symbolic debugger.
# You'll get better error checking without the following line.
#export CXXFLAGS="-g -Wall"
#export CFLAGS="-g -Wall"
export CXXFLAGS="-g -O2 -Wall -Weffc++"
#export CXXFLAGS="-g -O2 -Wall"
export CFLAGS="-g -O2 -Wall"
#export CXXFLAGS="-g -O0 -Wall"
#export CFLAGS="-g -O0 -Wall"
#export CXXFLAGS="-g -O3 -Wall"
#export CFLAGS="-g -O3 -Wall"
#export CXXFLAGS="-O2"
#export CFLAGS="-O2"
rm -f config.cache
aclocal -I config                                  && \
autoheader                                         && \
automake --gnits --add-missing --copy              && \
autoconf                                           && \
./configure --enable-maintainer-mode
set +x
