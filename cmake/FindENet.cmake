# - Find ENet
# Find the ENet libraries
#
#  This module defines the following variables:
#     ENET_FOUND        - True if ENET_INCLUDE_DIR & ENET_LIBRARY are found
#     ENET_INCLUDE_DIRS - where to find enet.h, etc.
#     ENET_LIBRARIES    - the ENet library
#
#============================================================================

find_path(ENET_INCLUDE_DIR NAMES enet/enet.h
        DOC "The ENet include directory")

find_library(ENET_LIBRARY NAMES enet enet64
        DOC "The ENet library")

# Handle the QUIETLY and REQUIRED arguments and set ENET_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENET REQUIRED_VARS ENET_LIBRARY ENET_INCLUDE_DIR)

if(ENET_FOUND)
    set(ENET_LIBRARIES ${ENET_LIBRARY})
    set(ENET_INCLUDE_DIRS ${ENET_INCLUDE_DIR})
endif()

mark_as_advanced(ENET_INCLUDE_DIR ENET_LIBRARY)
