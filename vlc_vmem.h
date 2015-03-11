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

#pragma once

#include <vector>

#include "vlc_basic_player.h"

namespace vlc
{
    class basic_vmem_wrapper {
    public:
        basic_vmem_wrapper()
            : _mp( 0 ) {}
        ~basic_vmem_wrapper() { close(); }

        bool open( vlc::basic_player* player );
        void close();

    private:
        //for libvlc_video_set_format_callbacks
        static unsigned video_format_proxy( void **opaque, char *chroma,
                                            unsigned *width, unsigned *height,
                                            unsigned *pitches, unsigned *lines )
            { return reinterpret_cast<basic_vmem_wrapper*>( *opaque )->video_format_cb( chroma,
                                                                                        width, height,
                                                                                        pitches, lines ); }
        static void video_cleanup_proxy( void *opaque )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_cleanup_cb(); }
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        static void* video_fb_lock_proxy( void *opaque, void **planes )
            { return reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_lock_cb( planes ); }
        static void  video_fb_unlock_proxy( void *opaque, void *picture, void *const *planes )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_unlock_cb( picture, planes ); }
        static void  video_fb_display_proxy( void *opaque, void *picture )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_display_cb( picture ); }
        //end (for libvlc_video_set_callbacks)

    protected:
        //for libvlc_video_set_format_callbacks
        virtual unsigned video_format_cb( char *chroma,
                                          unsigned *width, unsigned *height,
                                          unsigned *pitches, unsigned *lines ) = 0;
        virtual void video_cleanup_cb() = 0;
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        virtual void* video_lock_cb( void **planes ) = 0;
        virtual void  video_unlock_cb( void *picture, void *const *planes ) = 0;
        virtual void  video_display_cb( void *picture ) = 0;
        //end (for libvlc_video_set_callbacks)

    private:
        libvlc_media_player_t* _mp;
    };

    const char DEF_CHROMA[] = "RV32";
    enum {
        DEF_PIXEL_BYTES = 4,

        original_media_width = 0,
        original_media_height = 0
    };

    class vmem : public basic_vmem_wrapper
    {
    public:
        vmem();

        //0 - use size same as source has
        void set_desired_size( unsigned width, unsigned height );

        unsigned width() const { return _media_width; }
        unsigned height() const { return _media_height; }

    protected:
        //on_format_setup/on_frame_ready/on_frame_cleanup will come from worker thread
        virtual void on_format_setup() {}
        //frame_buf will be valid until return from
        //next call of on_frame_ready or on_frame_cleanup
        virtual void on_frame_ready( const std::vector<char>* frame_buf ) = 0;
        virtual void on_frame_cleanup() = 0;

    private:
        //for libvlc_video_set_format_callbacks
        virtual unsigned video_format_cb( char *chroma,
                                          unsigned *width, unsigned *height,
                                          unsigned *pitches, unsigned *lines );
        virtual void video_cleanup_cb();
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        virtual void* video_lock_cb( void **planes );
        virtual void  video_unlock_cb( void *picture, void *const *planes );
        virtual void  video_display_cb( void *picture );
        //end (for libvlc_video_set_callbacks)

    private:
        std::vector<char>  _frame_buf;
        unsigned           _desired_width;
        unsigned           _desired_height;
        unsigned           _media_width;
        unsigned           _media_height;
    };
};
