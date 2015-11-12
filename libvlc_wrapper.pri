include( libvlc-sdk/libvlc-sdk.pri )

CONFIG += c++11

HEADERS += $$PWD/vlc_vmem.h \
    $$PWD/vlc_audio.h \
    $$PWD/vlc_basic_player.h \
    $$PWD/vlc_helpers.h \
    $$PWD/vlc_player.h \
    $$PWD/vlc_subtitles.h \
    $$PWD/vlc_playback.h \
    $$PWD/vlc_video.h \
    $$PWD/vlc_media.h \
    $$PWD/callbacks_holder.h

SOURCES += $$PWD/vlc_vmem.cpp \
    $$PWD/vlc_audio.cpp \
    $$PWD/vlc_basic_player.cpp \
    $$PWD/vlc_helpers.cpp \
    $$PWD/vlc_player.cpp \
    $$PWD/vlc_subtitles.cpp \
    $$PWD/vlc_playback.cpp \
    $$PWD/vlc_video.cpp\
    $$PWD/vlc_media.cpp

!android {
    HEADERS += $$PWD/vlc_media_list_player.h

    SOURCES += $$PWD/vlc_media_list_player.cpp
}

INCLUDEPATH += $$PWD
