cmake_minimum_required(VERSION 3.2.3)
project(LD37)
set(TARGET_NAME LD37)

cmake_policy(SET CMP0003 NEW) # Relative library link paths
cmake_policy(SET CMP0022 NEW) # INTERFACE_LINK_LIBRARIES defines the link interface
cmake_policy(SET CMP0026 OLD) # Disallow use of the LOCATION target property - so we set to OLD as we still need it
cmake_policy(SET CMP0042 NEW) # MACOSX_RPATH is enabled by default

set(URHO3D_HOME ${CMAKE_SOURCE_DIR}/Urho3D/build)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

include(Urho3D-CMake-common)

define_source_files(GLOB_CPP_PATTERNS ${CMAKE_SOURCE_DIR}/src/*.cpp)
setup_main_executable()
