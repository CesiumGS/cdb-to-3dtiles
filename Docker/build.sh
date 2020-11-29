build_dir="/var/app/Build"

mkdir $build_dir
cd $build_dir
cmake .. -DM_LIB=/usr/lib64/libm.so

# Use all processors in parallel
processors=`grep -c ^processor /proc/cpuinfo`
make -j$processors $build_args
