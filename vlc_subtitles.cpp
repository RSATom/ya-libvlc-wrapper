/*****************************************************************************
* Copyright (c) 2013, Sergey Radionov <rsatom_gmail.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the project nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "vlc_subtitles.h"

#include "vlc_helpers.h"

using namespace vlc;

unsigned subtitles::track_count()
{
    if( _player.is_open() )
        return libvlc_video_get_spu_count( _player.get_mp() );

    return 0;
}

int subtitles::get_track()
{
    if( !_player.is_open() )
        return -1;

    int track_idx = -1;
    libvlc_track_description_t* tracks =
        libvlc_video_get_spu_description( _player.get_mp() );

    if( tracks ) {
        track_idx = track_id_2_track_idx( tracks, libvlc_video_get_spu( _player.get_mp() ) );

        libvlc_free( tracks );
    }

    return track_idx;
}

void subtitles::set_track( unsigned idx )
{
    if( !_player.is_open() )
        return;

    libvlc_track_description_t* tracks =
        libvlc_video_get_spu_description( _player.get_mp() );

    if( tracks ) {
        int id = track_idx_2_track_id( tracks, idx );
        if( id >= 0)
            libvlc_video_set_spu( _player.get_mp(), id );

        libvlc_free( tracks );
    }
}

int64_t subtitles::get_delay()
{
    if( !_player.is_open() )
        return 0;

    return libvlc_video_get_spu_delay( _player.get_mp() );
}

void subtitles::set_delay( int64_t d )
{
    if( !_player.is_open() )
        return;

    libvlc_video_set_spu_delay( _player.get_mp(), d );
}
