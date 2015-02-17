/*******************************************************************************
* Copyright © 2013-2015, Sergey Radionov <rsatom_gmail.com>
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

#include "vlc_audio.h"

#include "vlc_helpers.h"

using namespace vlc;

bool audio::is_muted()
{
    if( !_player.is_open() )
        return false;

    return libvlc_audio_get_mute( _player.get_mp() ) != 0;
}

void audio::toggle_mute()
{
    if( _player.is_open() )
        libvlc_audio_toggle_mute( _player.get_mp() );
}

void audio::set_mute( bool mute )
{
    if( _player.is_open() )
        libvlc_audio_set_mute( _player.get_mp(), mute );
}

unsigned audio::get_volume()
{
    if( !_player.is_open() )
        return 0;

    int v = libvlc_audio_get_volume( _player.get_mp() );

    return v < 0 ? 0 : v;
}

void audio::set_volume( unsigned volume )
{
    if( _player.is_open() )
        libvlc_audio_set_volume( _player.get_mp() , volume );
}

unsigned audio::track_count()
{
    if( !_player.is_open() )
        return 0;

    int tc = libvlc_audio_get_track_count( _player.get_mp() );

    return tc < 0 ? 0 : tc ;
}

int audio::get_track()
{
    if( !_player.is_open() )
        return -1;

    int track_idx = -1;
    libvlc_track_description_t* tracks =
        libvlc_audio_get_track_description( _player.get_mp() );

    if( tracks ) {
        track_idx = track_id_2_track_idx( tracks, libvlc_audio_get_track( _player.get_mp() ) );

        libvlc_free( tracks );
    }

    return track_idx;
}

void audio::set_track( unsigned idx )
{
    if( !_player.is_open() )
        return;

    libvlc_track_description_t* tracks =
        libvlc_audio_get_track_description( _player.get_mp() );

    if( tracks ) {
        int id = track_idx_2_track_id( tracks, idx );
        libvlc_audio_set_track( _player.get_mp(), id );

        libvlc_free( tracks );
    }
}

libvlc_audio_output_channel_t audio::get_channel()
{
    if( !_player.is_open() )
        return libvlc_AudioChannel_Error;

    return (libvlc_audio_output_channel_t) libvlc_audio_get_channel( _player.get_mp() );
}

void audio::set_channel( libvlc_audio_output_channel_t channel )
{
    if( _player.is_open() )
        libvlc_audio_set_channel( _player.get_mp(), channel );
}

int64_t audio::get_delay()
{
    if( !_player.is_open() )
        return 0;

    return libvlc_audio_get_delay( _player.get_mp() ) / 1000;
}

void audio::set_delay( int64_t d )
{
    if( !_player.is_open() )
        return;

    libvlc_audio_set_delay( _player.get_mp(), d * 1000 );
}
