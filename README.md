# cdb-to-3dtiles

# Install dependencies
Dependencies used:
- nlohmann json
- boost
- gdal
- glm
- libflt

To install the above third party libraries, you can use Microsoft's `vcpkg`. The instruction to install the package manager is in this link https://github.com/microsoft/vcpkg#quick-start-unix. All of the libraries except `libflt` can be installed by `vcpkg` and stored in the `vcpkg` directory. You can remove them completely by removing the `vcpkg` directory. 

For `libflt`, you can download the source code from this link http://fltlib.sourceforge.net/. There is an `INSTALL` file in the source code that instructs how to build the library. After that, there will be a file name `libflt.so` that we will use for the next step  

# How to build the project
- Create Build directory within the source project: `mkdir Build && cd Build`
- Create a `Shared\libflt\` directory in the Build directory and copy `libflt.so` in there  
- Run CMake from the Build directory: `cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg directory]/scripts/buildsystems/vcpkg.cmake`
- Run make command to build: `make`
- The Main Executable is in `Build/Main/Main`: `./Main -i [CDB directory] -o [Output directory]`
