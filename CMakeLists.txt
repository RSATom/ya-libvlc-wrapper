cmake_minimum_required( VERSION 2.8.11 )

if( NOT MSVC )
    add_definitions( -std=c++11 )
endif()

file( GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    )

add_library( libvlc_wrapper STATIC ${SOURCE_FILES} )
set_target_properties( libvlc_wrapper PROPERTIES POSITION_INDEPENDENT_CODE ON )
target_include_directories( libvlc_wrapper PUBLIC "libvlc-sdk/include" )

if( MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8 )
    find_library( LIBVLC_LIBRARY NAMES libvlc.x64
        HINTS "$ENV{LIBVLC_LIBRARY_PATH}"
              "${CMAKE_CURRENT_SOURCE_DIR}/libvlc-sdk/lib/msvc/"
        )
else()
    find_library( LIBVLC_LIBRARY NAMES vlc libvlc
        HINTS "$ENV{LIBVLC_LIBRARY_PATH}"
              "${CMAKE_CURRENT_SOURCE_DIR}/libvlc-sdk/lib/msvc/"
        PATHS  "/Applications/VLC.app/Contents/MacOS/lib"
        )
endif()

target_link_libraries( libvlc_wrapper ${LIBVLC_LIBRARY} )
