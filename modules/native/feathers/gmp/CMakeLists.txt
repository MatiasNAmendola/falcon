####################################################################
# The Falcon Programming Language
#
# CMake configuration file for the GNU Multiple Precision Arithmetic Library
####################################################################

# Creates the proper module file name from the project name.
falcon_define_module( CURRENT_MODULE gmp )

#find_package(GMP REQUIRED)

# Inclusion settings
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Sources files the module is built from.
set(SRC_FILES 
  gmp.cpp
  gmp_mod.cpp
  gmp_ext.cpp
)

# These are actually not needed by cmake to build. But if omitted they won't be
# listed in the virtual file tree of Visual Studio.
set(HDR_FILES
  gmp_ext.h
  gmp_mod.h
)

# Builds and links from the source files
add_library( ${CURRENT_MODULE} MODULE
   ${SRC_FILES}
   ${HDR_FILES} # optional, see comment above.
)

# If your module needs some lib more, use the followings:
target_link_libraries( ${CURRENT_MODULE} falcon_engine)

falcon_install_module( ${CURRENT_MODULE} )

