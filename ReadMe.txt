## Windows construction
All relevant libraries can be found in /libs and all DLLs in /dlls (pre)compiled for Windows. 
The Github link https://github.com/IsmaelSO2023/Virtual-Reality-Project
The CMake script knows where to find the libraries, so all you have to do is run the CMake script and generate this virtual reality project.
3D data, textures, etc. can be found in /resources
Bear in mind that the libraries supplied have been generated with a specific version of the compiler which may or may not work on your system (generating a large number of linking errors). In this case, we recommend that you compile the libraries yourself from source.


## Building Linux
First make sure you have CMake, Git and GCC by typing as root (sudo) `apt-get install g++ cmake git` and then retrieve the necessary packages:
Using root (sudo) and typing `apt-get install libsoil-dev libglm-dev libassimp-dev libglew-dev libglfw3-dev libxinerama-dev libxcursor-dev libxi-dev libfreetype-dev libgl1-mesa-dev xorg-dev` .

**Build with CMake-gui:** The source directory is Project and the build directory is Project/build. Creating the build directory in Projet is important for linking resource files (it will also be ignored by Git). Click configure and specify your build files (Unix Makefiles are recommended), resolve any missing directories or libraries, then click generate. Navigate to the build directory (`cd Project/build`) and type `make` in the terminal. This should generate the executables in the respective chapter folders.

**Cmake:** command line build
```
cd /path/to/Projet
mkdir build && cd build
cmake ...
cmake --build .
```

Note that CodeBlocks or other IDEs may have difficulty running programs due to problems finding shader and resource files, but they should still be able to generate executables. To get around this problem, it is possible to define an environment variable to tell tutorials where the resource files are located. The environment variable is named LOGL_ROOT_PATH and can be defined as the path to the root of the LearnOpenGL tree. For example, `export LOGL_ROOT_PATH

     `export LOGL_ROOT_PATH=home\Users\Administrateur\Desktop\Tisga\Projet VR`


Running `ls $LOGL_ROOT_PATH` should display, among other things, this README file and the resource directory.

## Building on Mac OS X
Construction on Mac OS X is fairly straightforward:
```
brew install cmake assimp glm glfw freetype
cmake -S . -B build
cmake --build build -j$(sysctl -n hw.logicalcpu)
```
## Create an Xcode project on the Mac platform
Thanks [@caochao](https://github.com/caochao):
After cloning the repo, go to the root path of the repo, and run the command below:
```
mkdir xcode
cd xcode
cmake -G Xcode ...

Once you've generated the CMakeList, you can select the Project and launch it.

Thanks



