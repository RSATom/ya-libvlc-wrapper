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

#pragma once

#include <stdint.h>

#include <vector>
#include <deque>

#include "vlc_basic_player.h"
#include "vlc_audio.h"
#include "vlc_video.h"
#include "vlc_subtitles.h"

namespace vlc
{
    enum playback_mode_e
    {
        mode_single = 0, //default. playback will stop on current item end (or error).
        mode_normal, //same as mode_single by default.
        mode_loop, //have effect only to next()/prev() calls
        mode_last = mode_loop,
    };

    struct media_player_events_callback {
        virtual void media_player_event( const libvlc_event_t* e ) = 0;
    };

    class player
    {
    public:
        player();
        ~player();

        bool open( libvlc_instance_t* inst );
        void close();

        bool is_open() const { return _player.is_open(); }

        libvlc_state_t get_state() { return _player.get_state(); }
        bool is_playing() { return libvlc_Playing == get_state(); }

        int add_media( const char * mrl_or_path,
                       unsigned optc, const char **optv,
                       bool is_path = false )
            { return add_media( mrl_or_path, optc, optv, 0, 0, is_path ); }
        int add_media( const char * mrl_or_path, bool is_path = false )
            { return add_media( mrl_or_path, 0, 0, is_path ); }
        int add_media( const char * mrl_or_path,
                       unsigned optc, const char **optv,
                       unsigned trusted_optc, const char **trusted_optv,
                       bool is_path = false );
        int add_media( const vlc::media& media );

        bool delete_item( unsigned idx );
        void clear_items();
        int  item_count();

        void disable_item( unsigned idx, bool disable );
        bool is_item_disabled( unsigned idx );

        void advance_item( unsigned idx, int count );

        vlc::media get_media( unsigned idx );
        vlc::media current_media()
            { return _player.current_media(); }
        int find_media_index( const vlc::media& );

        int  current_item();
        void set_current( unsigned idx );

        void play();
        bool play( unsigned idx );
        void pause();
        void togglePause();
        void stop();
        void next();
        void prev();

        float get_rate();
        void set_rate( float );

        float get_position();
        void  set_position( float );

        libvlc_time_t get_time();
        void set_time( libvlc_time_t );

        libvlc_time_t get_length();

        float get_fps();

        playback_mode_e get_playback_mode();
        void set_playback_mode( playback_mode_e m );

        vlc::basic_player& basic_player() { return _player; }
        vlc::video& video() { return _video; }
        vlc::audio& audio() { return _audio; }
        vlc::subtitles& subtitles() { return _subtitles; }

        libvlc_media_player_t* get_mp() const
            { return _player.get_mp(); }

        //events will come from worker thread
        void register_callback( media_player_events_callback* );
        void unregister_callback( media_player_events_callback* );

        void swap( player* );

    private:
        struct playlist_item
        {
            vlc::media media;
            bool disabled;
        };

        typedef std::deque<playlist_item> playlist_t;
        typedef playlist_t::iterator playlist_it;
        typedef playlist_t::const_iterator playlist_cit;

        typedef std::vector<media_player_events_callback*> callbacks_t;
        typedef callbacks_t::iterator callbacks_it;
        typedef callbacks_t::const_iterator callbacks_cit;

    private:
        static void get_media_sub_items( const vlc::media& media, playlist_t* out );
        bool try_expand_current();
        void internal_play( int idx );
        int find_valid_item( int start_from_idx, bool forward );

        static void event_proxy( const libvlc_event_t* , void* );
        void event( const libvlc_event_t* );
        void events_attach( bool attach );

    private:
        libvlc_instance_t* _libvlc_instance;

        vlc::basic_player  _player;
        vlc::video         _video;
        vlc::audio         _audio;
        vlc::subtitles     _subtitles;

        playback_mode_e _mode;
        playlist_t _playlist;
        int        _current_idx;

        callbacks_t _callbacks;
    };
}

typedef vlc::player vlc_player;
