BUILDING:
  mkdir build
  cd build
  CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -G "Unix Makefiles" ../
  
  make -j9

RUNNING:
  ./build/bin/cheffe <INPUT_FILE>
