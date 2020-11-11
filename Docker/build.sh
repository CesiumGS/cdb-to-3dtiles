build_dir="/var/app/docker-build"

mkdir $build_dir
cd $build_dir
cmake .. -DM_LIB=/usr/lib64/libm.so -DJPEG_LIBRARY=/usr/lib64/libjpeg.a -DJPEG_INCLUDE_DIR=/usr/include -DPNG_LIBRARY=/usr/lib64/libpng.a -DPNG_INCLUDE_DIR=/usr/include

# Use all processors in parallel
processors=`grep -c ^processor /proc/cpuinfo`
make -j$processors $build_args
