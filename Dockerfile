FROM centos:7

# Install extra repositories needed for gcc 8, cmake3, and git-lfs
RUN yum update -y -q \
    && yum install -y -q epel-release centos-release-scl
    

# Install required packages
# Initialize git lfs
# Make cmake3 the preferred cmake
RUN yum install -y \
        gcc-c++ \
        wget \
        gmp-devel \
        mpfr-devel \
        libmpc-devel \
        libmpc-static \
        devtoolset-8-runtime \
        devtoolset-8-toolchain \
        automake \
        make 
# install recent gcc/g++ version
RUN wget https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.xz \
    && tar xf gcc-10.2.0.tar.xz
RUN rm -rf gcc-10.2.0.tar.xz
RUN cd gcc-10.2.0 && ./configure --enable-languages=c,c++ --disable-multilib \
    && make -j$(nproc)\
    && make install

RUN yum install -y \
       git \
       libpng-static \
       zlib-static \
       vim \
       autoconf \
       libjpeg-turbo-static \
       cmake3 \
       libjpeg-devel \
       libtiff-devel \
       libtiff-static \
       freeglut-devel \
       openjpeg-devel \
       libgeotiff-devel
RUN yum group install -y "Development Tools"

# create symbolic link cmake to cmake3
RUN ln -s /usr/bin/cmake3 /usr/bin/cmake

# install sqlite
RUN wget https://www.sqlite.org/2020/sqlite-autoconf-3330000.tar.gz\
    && tar xzf sqlite-autoconf-3330000.tar.gz
RUN cd sqlite-autoconf-3330000 && ./configure \
    && make -j$(nproc)\
    && make install

# install proj
RUN wget https://download.osgeo.org/proj/proj-7.1.1.tar.gz \
    && tar xzf proj-7.1.1.tar.gz
RUN rm -rf proj-7.1.1.tar.gz
RUN cd proj-7.1.1 && SQLITE3_LIBS="-L/usr/local/lib -lsqlite3" SQLITE3_CFLAGS="-I/usr/local/include" ./configure --without-curl \
    && make -j$(nproc)\
    && make install

# install gdal
RUN wget http://download.osgeo.org/gdal/3.2.0/gdal-3.2.0.tar.gz \
    && tar xzf gdal-3.2.0.tar.gz
RUN rm -rf gdal-3.2.0.tar.gz
RUN cd gdal-3.2.0 && ./configure --with-proj=/usr/local/ --with-sqlite3 --with-geotiff --without-ld-shared --disable-shared --enable-static --disable-all-optional-drivers --with-jpeg=/usr/lib64 --with-openjpeg \
    && make -j$(nproc) static-lib\
    && make install

RUN rm -f /usr/bin/gcc
RUN rm -f /usr/local/bin/gcc
RUN update-alternatives --install /usr/bin/gcc gcc /usr/local/bin/x86_64-pc-linux-gnu-gcc-10.2.0 1

RUN rm -f /usr/local/bin/g++
RUN rm -f /usr/bin/g++
RUN update-alternatives --install /usr/bin/g++ g++ /usr/local/bin/x86_64-pc-linux-gnu-g++ 1

RUN rm -f /usr/bin/c++
RUN update-alternatives --install /usr/bin/c++ c++ /usr/local/bin/x86_64-pc-linux-gnu-g++ 1

WORKDIR /var/app

ENTRYPOINT ["/bin/bash"]