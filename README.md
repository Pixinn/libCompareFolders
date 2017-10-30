#COMPARE_FOLDER
This program will compare the content of two folders and display:

 - Files that are identical
 - Files that are different
 - Files that have the same content but a different name and/or location
 - Files that are unique

##Usage
> compare_folder folder1 folder2
## How to build
### Prerequisites

*compare_folder* relies on the following **git submodules**

 - cryptopp

Those submodules are locates in the *submodules* folder.
In order to build *compare_folder* all the submodules have to have been built.

*compare_folder* also relies on **Boost**.
Export the environment variable **BOOST_DIR** to your local install of boost in order to help cmake find it.

### CMAKE
*compare_folder* is built with **cmake**.
> mkdir *BUILD_DIR* && cd *BUILD_DIR*
> cmake ..
> make

