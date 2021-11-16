# $Id: CMakeLists.txt 1059 2014-07-09 08:26:09Z RS $
# This is the CMake project file for configuration of the RiVLib examples.
#
#           Copyright (c) 2015 Riegl Laser Measurement Systems.

# The authors hereby  grant permission  to use,  and copy this software  for the
# purpose  of enhancing the useability  of  Riegl Laser Measurement Systems GmbH
# instruments you own. You may NOT distribute or modify the software for the use
# in commercial applications without the written consent of RLMS.
#
# In any case copyright notices and  this notice must be  retained in all copies
# verbatim in any distributions.

# IN  NO EVENT SHALL  THE AUTHORS OR  DISTRIBUTORS BE  LIABLE TO  ANY PARTY  FOR
# DIRECT, INDIRECT, SPECIAL,  INCIDENTAL, OR  CONSEQUENTIAL DAMAGES  ARISING OUT
# OF THE USE OF THIS SOFTWARE, ITS  DOCUMENTATION,  OR ANY DERIVATIVES  THEREOF,
# EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM  ANY WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  OF MERCHANTABILITY,  FITNESS FOR A
# PARTICULAR PURPOSE,  AND  NON-INFRINGEMENT. THIS SOFTWARE  IS  PROVIDED  ON AN
# "AS IS" BASIS, AND THE AUTHORS  AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
# MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

# This file is the input description for CMake, a tool which is able to
# generate build environments for various compilers.
# You can obtain a copy of CMake at http://www.cmake.org
# It is recommended to use the graphical tool from CMake to do the
# configuration. First you have to specify the source directory i.e. where
# the RiVlib examples are. Next you specify a directory where you want your
# build to take place. Typically this is the directory where the project files
# of your IDE will be created.
# Example:
# Source dir: ...../examples
# Build dir:  ...../examples-build
# The next step is to press the configure button. CMake will prompt you
# with a message and a variable in red, requesting you to specify the
# root directory of your variant of rivlib you will use. Use the ... navigate
# button to specify the subdirectory within your downloaded copy of RiVLib that
# fits your compiler.
# Example:
# RiVLib_DIR = .../rivlib-1_30-x86-windows-vc120 
# Repeat pressing the configuration button and resolve any more red lines,
# unitil there are no unsatisfied constraints remaining.
# Press generate, and the project files will be created.
# Now you can launch your favorite IDE or invoke the genrated Makefile.
#
# NOTE:
# RiVLib needs a compiler with C++11 support. Most recent compilers
# already do have such support. If using GCC you might need to add the
# flag -std=gnu++0x to the CMAKE_CXX_FLAGS variable.
#

cmake_minimum_required(VERSION 3.4.1)

include( InstallRequiredSystemLibraries )

project(RiVLib_examples)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

find_package(RiVLib
    COMPONENTS scanlib scanifc ctrllib ctrlifc
)

include_directories(
    ${RiVLib_INCLUDE_DIRS}
)

#
# Example programs
#

add_executable( ctrlifc_demo1
    ctrlifc_demo1.c
)
target_link_libraries( ctrlifc_demo1
    ${RiVLib_CTRLIFC_LIBRARY}
)
if (UNIX)
set_target_properties( ctrlifc_demo1
    PROPERTIES
        LINK_FLAGS "-z origin"
        INSTALL_RPATH "\\\$ORIGIN"
)
endif (UNIX)
install(
    TARGETS ctrlifc_demo1
    RUNTIME DESTINATION bin
)
install(
    FILES ${RiVLib_CTRLIFC_RUNTIME_LIBRARIES}
    DESTINATION bin
)

add_executable( ctrllib_demo2
    ctrllib_demo2.cpp
)
target_link_libraries( ctrllib_demo2
    ${RiVLib_CTRLLIB_LIBRARY}
    ${RiVLib_SCANLIB_LIBRARY}
)
install(
    TARGETS ctrllib_demo2
    RUNTIME DESTINATION bin
)

add_executable( ctrllib_demo3
    ctrllib_demo3.cpp
)
target_link_libraries( ctrllib_demo3
    ${RiVLib_CTRLLIB_LIBRARY}
    ${RiVLib_SCANLIB_LIBRARY}
)
install(
    TARGETS ctrllib_demo3
    RUNTIME DESTINATION bin
)

add_executable( pointcloudcpp
    pointcloudcpp.cpp
)
target_link_libraries( pointcloudcpp
    ${RiVLib_SCANLIB_LIBRARY}
)
install(
    TARGETS pointcloudcpp
    RUNTIME DESTINATION bin
)

add_executable( rxp_record
    rxp_record.cpp
)
target_link_libraries( rxp_record
    ${RiVLib_SCANLIB_LIBRARY}
)
install(
    TARGETS rxp_record
    RUNTIME DESTINATION bin
)

add_executable( pointclouddll
    pointclouddll.c
)
target_link_libraries( pointclouddll
    ${RiVLib_SCANIFC_LIBRARY}
)
if (UNIX)
set_target_properties( pointclouddll
    PROPERTIES
        LINK_FLAGS "-z origin"
        INSTALL_RPATH "\\\$ORIGIN"
)
endif (UNIX)
install(
    TARGETS pointclouddll
    RUNTIME DESTINATION bin
)
install(
    FILES ${RiVLib_SCANIFC_RUNTIME_LIBRARIES}
    DESTINATION bin
)

install( 
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} 
    DESTINATION bin
    COMPONENT System 
)
