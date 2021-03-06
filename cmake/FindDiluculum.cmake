#
# Find's Diluculum library.
#
# Input variables:
#   Diluculum_ROOT              Additional library search directory.
#   Diluculum_STATIC_LIBRARIES  Look for static libraries.
#
# Output variables:
#   Diluculum_INCLUDE_DIR   Include directory.
#   Diluculum_LIBRARY       Library.

cmake_minimum_required( VERSION 2.8 )

set( Diluculum_FOUND FALSE )

if( Diluculum_STATIC_LIBRARIES )
	set( Diluculum_SUFFIX "-s" )
	add_definitions( -DDiluculum_STATIC )
else()
	set( Diluculum_SUFFIX "" )
endif()

find_path(
	Diluculum_INCLUDE_DIR
	Diluculum/LuaState.hpp
	PATH_SUFFIXES
		include
	PATHS
		/usr
		/usr/local
		${Diluculum_ROOT}
)

find_library(
	Diluculum_LIBRARY_RELEASE
	Diluculum${Diluculum_SUFFIX}
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		/usr
		/usr/local
		${Diluculum_ROOT}
)

find_library(
	Diluculum_LIBRARY_DEBUG
	Diluculum${Diluculum_SUFFIX}-d
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		/usr
		/usr/local
		${Diluculum_ROOT}
)

if( Diluculum_LIBRARY_RELEASE AND Diluculum_LIBRARY_DEBUG )
	set( Diluculum_LIBRARY debug ${Diluculum_LIBRARY_DEBUG} optimized ${Diluculum_LIBRARY_RELEASE} )
endif()

if( Diluculum_LIBRARY_RELEASE AND NOT Diluculum_LIBRARY_DEBUG )
	set( Diluculum_LIBRARY_DEBUG ${Diluculum_LIBRARY_RELEASE} )
	set( Diluculum_LIBRARY ${Diluculum_LIBRARY_RELEASE} )
endif()

if( Diluculum_LIBRARY_DEBUG AND NOT Diluculum_LIBRARY_RELEASE )
	set( Diluculum_LIBRARY_RELEASE ${Diluculum_LIBRARY_DEBUG} )
	set( Diluculum_LIBRARY ${Diluculum_LIBRARY_DEBUG} )
endif()

if( NOT Diluculum_INCLUDE_DIR OR NOT Diluculum_LIBRARY )
	message( FATAL_ERROR "Diluculum not found." )
else()
	message( "Diluculum found: ${Diluculum_INCLUDE_DIR}" )
endif()
