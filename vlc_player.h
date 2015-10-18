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

#include "callbacks_holder.h"
#include "vlc_basic_player.h"
#include "vlc_playback.h"
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

    struct media_player_events_callback
    {
        virtual void media_player_event( const libvlc_event_t* e ) = 0;
    };

    class player_core
        : protected callbacks_holder<media_player_events_callback>
    {
        typedef callbacks_holder<media_player_events_callback> callbacks_holder;

    public:
        player_core();
        ~player_core();

        virtual bool open( libvlc_instance_t* );
        virtual void close();

        bool is_open() const { return _player.is_open(); }

        libvlc_state_t get_state() { return _player.get_state(); }
        bool is_playing() { return libvlc_Playing == get_state(); }

        vlc::media current_media()
            { return _player.current_media(); }

        virtual void play() = 0;
        void pause();
        void togglePause();
        void stop();

        vlc::basic_player& basic_player() { return _player; }

        vlc::playback& playback() { return _playback; }
        vlc::video& video() { return _video; }
        vlc::audio& audio() { return _audio; }
        vlc::subtitles& subtitles() { return _subtitles; }

        libvlc_media_player_t* get_mp() const
            { return _player.get_mp(); }

        //events will come from worker thread
        void register_callback( media_player_events_callback* ) override;
        void unregister_callback( media_player_events_callback* ) override;

        void swap( player_core* );

    private:
        void event( const libvlc_event_t* );

    protected:
        static void event_proxy( const libvlc_event_t* , void* );
        virtual void events_attach( bool attach );

    protected:
        libvlc_instance_t* _libvlc_instance;
        vlc::basic_player  _player;

    private:
        vlc::playback      _playback;
        vlc::video         _video;
        vlc::audio         _audio;
        vlc::subtitles     _subtitles;
    };

    extern const unsigned PLAYLIST_MAX_SIZE;

    class playlist_player_core : public player_core
    {
    public:
        virtual playback_mode_e get_playback_mode() = 0;
        virtual void set_playback_mode( playback_mode_e m ) = 0;

        int add_media( const char* mrl_or_path,
                       unsigned optc, const char **optv,
                       bool is_path = false )
            { return add_media( mrl_or_path, optc, optv, 0, 0, is_path ); }
        int add_media( const char* mrl_or_path, bool is_path = false )
            { return add_media( mrl_or_path, 0, 0, is_path ); }
        int add_media( const char* mrl_or_path,
                       unsigned optc, const char** optv,
                       unsigned trusted_optc, const char** trusted_optv,
                       bool is_path = false );

        virtual int add_media( const vlc::media& media ) = 0;

        virtual unsigned item_count() = 0;

        virtual bool delete_item( unsigned idx ) = 0;
        virtual void clear_items() = 0;

        virtual vlc::media get_media( unsigned idx ) = 0;

        virtual int find_media_index( const vlc::media& ) = 0;

        virtual void disable_item( unsigned idx, bool disable ) = 0;
        virtual bool is_item_disabled( unsigned idx ) = 0;

        virtual void set_item_data( unsigned idx, const std::string& ) = 0;
        virtual const std::string& get_item_data( unsigned idx ) = 0;

        virtual void advance_item( unsigned idx, int count ) = 0;

        virtual int current_item() = 0;
        virtual void set_current( unsigned idx ) = 0 ;

        using player_core::play;
        virtual bool play( unsigned idx ) = 0;
        virtual void next() = 0;
        virtual void prev() = 0;
    };

    class player: public playlist_player_core
    {
    public:
        player();

        void close() override;

        void play() override;
        bool play( unsigned idx ) override;
        void next() override;
        void prev() override;

        playback_mode_e get_playback_mode() override;
        void set_playback_mode( playback_mode_e m ) override;

        using playlist_player_core::add_media;
        int add_media( const vlc::media& media ) override;

        void advance_item( unsigned idx, int count ) override;

        bool delete_item( unsigned idx ) override;
        void clear_items() override;
        unsigned item_count() override;

        vlc::media get_media( unsigned idx ) override;
        int find_media_index( const vlc::media& ) override;

        int current_item() override;
        void set_current( unsigned idx ) override;

        void disable_item( unsigned idx, bool disable ) override;
        bool is_item_disabled( unsigned idx ) override;

        void set_item_data( unsigned idx, const std::string& ) override;
        const std::string& get_item_data( unsigned idx ) override;

        void swap( player* );

    private:
        struct playlist_item
        {
            vlc::media media;
            bool disabled;
            std::string data;
        };

        typedef std::deque<playlist_item> playlist_t;
        typedef playlist_t::iterator playlist_it;
        typedef playlist_t::const_iterator playlist_cit;

    private:
        static void get_media_sub_items( const vlc::media& media, playlist_t* out );
        bool try_expand_current();
        void internal_play( int idx );
        int find_valid_item( int start_from_idx, bool forward );

    private:
        playback_mode_e _mode;
        playlist_t _playlist;
        int        _current_idx;
    };
}

typedef vlc::player vlc_player;
