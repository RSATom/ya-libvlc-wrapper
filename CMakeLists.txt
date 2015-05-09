cmake_minimum_required( VERSION 2.8 )

file( GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    )

add_library( libvlc_wrapper STATIC ${SOURCE_FILES} )
