/*******************************************************************************
* Copyright © 2013-2014, Sergey Radionov <rsatom_gmail.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.

* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "vlc_video.h"

#include "vlc_helpers.h"

using namespace vlc;

video::video( vlc::basic_player& player )
    : _player( player )
{
}

bool video::has_vout()
{
    if( !_player.is_open() )
        return false;

    return libvlc_media_player_has_vout( _player.get_mp() ) > 0;
}

std::string vlc::video::get_aspect()
{
    if( !_player.is_open() )
        return std::string();

    std::string aspect;
    char* c = libvlc_video_get_aspect_ratio( _player.get_mp() );
    if ( c )
        aspect = c;
    libvlc_free( c );

    return aspect;
}

void vlc::video::set_aspect( const std::string& aspect )
{
    if( !_player.is_open() )
        return;

    libvlc_video_set_aspect_ratio( _player.get_mp(), aspect.c_str() );
}

std::string vlc::video::get_crop()
{
    if( !_player.is_open() )
        return std::string();

    std::string crop;
    char* c = libvlc_video_get_crop_geometry( _player.get_mp() );
    if ( c )
        crop = c;
    libvlc_free( c );

    return crop;
}

void vlc::video::set_crop( const std::string& crop )
{
    if( !_player.is_open() )
        return;

    libvlc_video_set_crop_geometry( _player.get_mp(), crop.c_str() );
}

float video::get_ajust_filter_var( libvlc_video_adjust_option_t option,
                                   float def_v )
{
    if( _player.is_open() ) {
        return libvlc_video_get_adjust_float( _player.get_mp(), option );
    }
    else return def_v;
}

void video::set_ajust_filter_var( libvlc_video_adjust_option_t option, float val )
{
    if( _player.is_open() ) {
        libvlc_video_set_adjust_float( _player.get_mp(), option, val );
    }
}

int video::get_ajust_filter_var( libvlc_video_adjust_option_t option,
                                 int def_v )
{
    if( _player.is_open() ) {
        return libvlc_video_get_adjust_int( _player.get_mp(), option );
    }
    else return def_v;
}

void video::set_ajust_filter_var( libvlc_video_adjust_option_t option, int val )
{
    if( _player.is_open() ) {
        libvlc_video_set_adjust_int( _player.get_mp(), option, val );
    }
}

unsigned video::track_count()
{
    if( !_player.is_open() )
        return 0;

    int count = libvlc_video_get_track_count( _player.get_mp() );
    return count < 0 ? 0 : count;
}

int video::get_track()
{
    if( !_player.is_open() )
        return -1;

    int track_idx = -1;
    libvlc_track_description_t* tracks =
        libvlc_video_get_track_description( _player.get_mp() );

    if( tracks ) {
        track_idx = track_id_2_track_idx( tracks, libvlc_video_get_track( _player.get_mp() ) );

        libvlc_free( tracks );
    }

    return track_idx;
}

void video::set_track( unsigned idx )
{
    if( !_player.is_open() )
        return;

    libvlc_track_description_t* tracks =
        libvlc_video_get_track_description( _player.get_mp() );

    if( tracks ) {
        int id = track_idx_2_track_id( tracks, idx );
        libvlc_video_set_track( _player.get_mp(), id );

        libvlc_free( tracks );
    }
}
