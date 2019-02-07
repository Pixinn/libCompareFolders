# What is it?

**CompareFolders** is a **library** which purpose is to scan and compare the content of folders. It can detect:

 - Files that are **identical**
 - Files that are **different**
 - Files that have the **same content but a different name and/or location**
 - Files that are **unique**

The project builds the library plus two **applications** using it:

 - **scan_folder** scans the content of a folder and outputs the state of its files into a JSON file.
 - **compare_folders** compares the content of two folders and outputs a summary on screen or in a JSON file.
	 - Those folders can be either "real" folders on the disk, or a JSON file resulting from *scan_folder*.
 
# How to build
## Prerequisites
### Submodules

*CompareFolders* relies on the following **git submodules**, located in the *submodules* folder.

 - cryptopp
 - rlutil

In order to build *compare_folder* the submodules have to have been previoulsy built and **installed**.

### Boost

*CompareFolders* also relies on **Boost**. Version 1_6_5 is the minimum recommended version.

You may set the environment variable **BOOST_DIR** to your local install of boost in order to help cmake find it.

### Cmake

The Built system uses *cmake*. Version 3.9 is the minimum recommended version. If you want to use a newer version of *Boost* than the minimum recommended, you may have to also use a newer version of *cmake*.

> mkdir [*BUILD_DIR*] && cd [*BUILD_DIR*]
> 
> cmake ..
> 
> cmake --build .

# Documentation

The code is commented for **Doxygen** and a *Doxyfile* is provided.

The two provided applications, plus the tests are also good examples.
