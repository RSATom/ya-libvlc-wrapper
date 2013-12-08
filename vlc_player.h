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
*     * Neither the name of the Sergey Radionov aka RSATom nor the
*       names of project contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

//include stdint.h before boost to avoid conflicts
#include <stdint.h>

#include <deque>
#include <boost/thread/recursive_mutex.hpp>

#include "vlc_basic_player.h"
#include "vlc_audio.h"
#include "vlc_video.h"
#include "vlc_subtitles.h"
#include "vlc_current_media.h"

namespace vlc
{
    enum playback_mode_e
    {
        mode_normal,
        mode_loop,
    };

    class player
    {
    public:
        player();
        ~player();

        bool open(libvlc_instance_t* inst);
        void close();

        bool is_open() const { return _player.is_open(); }

        libvlc_state_t get_state() { return _player.get_state(); }
        bool is_playing() { return libvlc_Playing == get_state(); }

        int add_media( const char * mrl_or_path,
                                   unsigned optc, const char **optv,
                                   bool is_path = false )
            { return add_media(mrl_or_path, optc, optv, 0, 0, is_path); }
        int add_media( const char * mrl_or_path, bool is_path = false )
            { return add_media(mrl_or_path, 0, 0, is_path); }
        int add_media( const char * mrl_or_path,
                                   unsigned optc, const char **optv,
                                   unsigned trusted_optc, const char **trusted_optv,
                                   bool is_path = false );
        bool delete_item( unsigned idx );
        void clear_items();
        int  item_count();

        int  current_item();
        void set_current( unsigned idx );

        void play();
        bool play( unsigned idx );
        void pause();
        void stop( bool async = false );
        void next();
        void prev();

        float get_rate();
        void set_rate( float );

        float get_position();
        void  set_position( float );

        libvlc_time_t get_time();
        void set_time( libvlc_time_t );

        void set_playback_mode( playback_mode_e m )
            { _mode = m; }

        vlc::basic_player& basic_player() { return _player; }
        vlc::video& video() { return _video; }
        vlc::audio& audio() { return _audio; }
        vlc::subtitles& subtitles() { return _subtitles; }
        vlc::current_media& current_media() { return _current_media; }

        libvlc_media_player_t* get_mp() const
            { return _player.get_mp(); }

    private:
        static void libvlc_event_proxy( const struct libvlc_event_t* event, void* user_data);
        void libvlc_event( const struct libvlc_event_t* event );

        bool try_expand_current();
        void internalPlay( int idx );

    private:
        struct playlist_item
        {
            libvlc_media_t* media;
        };

        typedef std::deque<playlist_item> playlist_t;
        typedef playlist_t::iterator playlist_it;
        typedef playlist_t::const_iterator playlist_cit;

    private:
        libvlc_instance_t* _libvlc_instance;

        vlc::basic_player  _player;
        vlc::video         _video;
        vlc::audio         _audio;
        vlc::subtitles     _subtitles;
        vlc::current_media _current_media;

        playback_mode_e    _mode;

        typedef boost::recursive_mutex mutex_t;
        mutex_t    _playlist_guard;
        playlist_t _playlist;
        int        _current_idx;
    };
};

typedef vlc::player vlc_player;
