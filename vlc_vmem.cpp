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

#include "vlc_vmem.h"

#include <cstring>

using namespace vlc;

////////////////////////////////////////////////////////////////////////////////
// class vlc::basic_vmem_wrapper
////////////////////////////////////////////////////////////////////////////////
bool basic_vmem_wrapper::open( vlc::basic_player* player )
{
    if( player->is_open() && player->get_mp() == _mp )
        return true;

    close();

    if( !player->is_open() )
        return false;

    _mp = player->get_mp();
    libvlc_media_player_retain( _mp );

    libvlc_video_set_callbacks( _mp,
                                video_fb_lock_proxy,
                                video_fb_unlock_proxy,
                                video_fb_display_proxy,
                                this );

    libvlc_video_set_format_callbacks( _mp,
                                       video_format_proxy,
                                       video_cleanup_proxy );

    return true;
}

unsigned video_format_stub( void**, char*, unsigned*, unsigned*, unsigned*, unsigned* )
{
    return 0;
}

void* video_fb_lock_stub( void*, void** planes )
{
    planes[0] = planes[1] = planes[3] = 0;
    return 0;
}

void basic_vmem_wrapper::close()
{
    if( _mp ) {
        libvlc_video_set_callbacks( _mp, video_fb_lock_stub, 0, 0, 0 );
        libvlc_video_set_format_callbacks( _mp, video_format_stub, 0 );

        //libvlc will continue to use old callbacks until playback will be stopped
        libvlc_media_player_stop( _mp );
        libvlc_media_player_release( _mp );
        _mp = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// class vlc::vmem
////////////////////////////////////////////////////////////////////////////////
vmem::vmem()
    : _desired_width( 0 ), _desired_height( 0 ),
      _media_width( 0 ), _media_height( 0 )
{
}

unsigned vmem::video_format_cb( char* chroma,
                                unsigned* width, unsigned* height,
                                unsigned* pitches, unsigned* lines )
{
    if ( original_media_width != _desired_width && original_media_height != _desired_height ) {
        float src_aspect = (float) *width / *height;
        float dst_aspect = (float) _desired_width / _desired_height;
        if ( src_aspect > dst_aspect ) {
            if( _desired_width != *width ) { //don't scale if size equal
                *width  = _desired_width;
                *height = static_cast<unsigned>( *width / src_aspect + 0.5 );
            }
        }
        else {
            if( _desired_height != *height ) { //don't scale if size equal
                *height = _desired_height;
                *width  = static_cast<unsigned>( *height * src_aspect + 0.5 );
            }
        }
    }

    _media_width  = *width;
    _media_height = *height;

    memcpy( chroma, DEF_CHROMA, sizeof( DEF_CHROMA ) - 1 );
    *pitches = _media_width * DEF_PIXEL_BYTES;
    *lines   = _media_height;

    //+1 for vlc 2.0.3/2.1 bug workaround.
    //They writes after buffer ed boundary by some reason unknown to me...
    _frame_buf.resize( *pitches * ( *lines + 1 ) );

    on_format_setup();

    return 1;
}

void vmem::video_cleanup_cb()
{
    on_frame_cleanup();

    _frame_buf.resize( 0 );
    _media_width  = 0;
    _media_height = 0;
}

void* vmem::video_lock_cb( void **planes )
{
    *planes = _frame_buf.empty() ? 0 : &_frame_buf[0];
    return 0;
}

void vmem::video_unlock_cb( void* /*picture*/, void *const * /*planes*/ )
{
}

void vmem::video_display_cb( void * /*picture*/ )
{
    on_frame_ready( &_frame_buf );
}

void vmem::set_desired_size( unsigned width, unsigned height )
{
    _desired_width = width;
    _desired_height = height;
}
