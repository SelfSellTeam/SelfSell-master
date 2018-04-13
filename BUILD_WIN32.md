Windows - Visual Studio 2013
============================
#### Prerequisites ####
* Microsoft Visual C++ 2013 Update 1 (the free Express edition will work)
* If you have multiple MSVS installation use MSVS Developer console from target version.
* You can build either Windows 64bit binaries.

#### Set up the directory structure ####
* Create a base directory for all projects.  I'm putting everything in
  `D:\SelfSell`, you can use whatever you like.  In several of the batch files
  and makefiles, this directory will be referred to as `SelfSell_ROOT`: ```
  
* Clone the SelfSell repository:
  ```
   $ mkdir D:\SelfSell
   $ cd D:\SelfSell
   $ git clone https://github.com/SelfSell-Dev/SelfSell.git
   $ cd SelfSell
   $ git submodule update --init --recursive
  ```

#### SelfSell depends on the following third party libraries - Skip if you downloaded the prebuilt binary package(s) ####

 * Boost
   SelfSell depends on the Boost libraries version 1.55 or later (I assume
   you're using 1.55, the latest as of this writing). 
   * download the latest boost source from http://www.boost.org/users/download/
   * unzip it to the base directory `D:\SelfSell`.
   * This will create a directory like `D:\SelfSell\boost_1_55_0`.

 * OpenSSL
   SelfSell depends on OpenSSL, and you must build this from source.
    * download the latest OpenSSL source from http://www.openssl.org/source/
    * Untar it to the base directory `D:\SelfSell`
    * this will create a directory like `D:\SelfSell\openssl-1.0.1g`.

 At the end of this, your base directory should look like this:
  ```
  D:\SelfSell\include
  +- LevelDB
  +- SelfSell
  +- boost_1.55
  +- CMake
  +- OpenSSL
  +- miniupnpc
  +- fc
  ```
#### Build the library dependencies - Skip if you downloaded the prebuilt binary package(s) ####
* Build boost libraries (required for 64bit builds only):
  ```
  cd D:\SelfSell\boost
  bootstrap.bat
  b2.exe toolset=msvc-11.0 variant=debug,release link=static threading=multi runtime-link=shared address-model=32
  ```
    The file `D:\SelfSell\SelfSell\libraries\fc\CMakeLists.txt` has the
    `FIND_PACKAGE(Boost ...)`
    command that makes CMake link in Boost.  That file contains the line:
    ```
    set(Boost_USE_DEBUG_PYTHON ON)
    ```
    Edit this file and comment this line out (with a `#`).
    This line  tells CMake to look for a boost library that was built with
    `b2.exe link=shared python-debugging=on`.  That would cause debug builds to
    have `-gyd` mangled into their filename.  We don't need python debugging here,
    so we didn't give the `python-debugging` argument to `b2.exe`, and
    that causes our boost debug libraries to have `-gd` mangled into the filename
    instead.  If this option in `fc\CMakeLists.txt` doesn't match the way you
    compiled boost, CMake won't be able to find the debug version of the boost
    libraries, and you'll get some strange errors when you try to run the
    debug version of SelfSell.

* Build OpenSSL DLLs
  ```
  cd D:\SelfSell\openssl-1.0.1g
  perl Configure --openssldir=D:\SelfSell\OpenSSL VC-WIN32
  ms\do_ms.bat
  nmake -f ms\ntdll.mak
  nmake -f ms\ntdll.mak install
  ```
  This will create the directory `D:\SelfSell\OpenSSL` with the libraries, DLLs,
  and header files.

#### Build SelfSell ####
* Launch *Visual Studio* and load `D:\SelfSell\SelfSell\SelfSell.sln` for 64 bit builds.

* *Build Solution*

