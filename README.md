# spatial_partitioning_study
Testing hierarchical grids for spatial partitioning

# Build (Windows - MSVC)
Visual Studio with C++ compiler msvc is required!

First you must run vs script to set environ variables. In my machine for 2019 is here:
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

Then clone the git and move to src folder:

```
mkdir spatial_partitioning_study
git clone https://github.com/4Bruno/spatial_partitioning_study.git
cd spatial_partitioning_study\src
..\debug_build.bat
```
I debug in VS by openning executable in output folder (debug/)
or
Simply run:
```
debug\main.exe
```
