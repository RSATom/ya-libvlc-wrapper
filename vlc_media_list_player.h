/*******************************************************************************
* Copyright © 2015, Sergey Radionov <rsatom_gmail.com>
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

#include "vlc_player.h"

namespace vlc
{
    class media_list_player : public playlist_player_core
    {
    public:
        media_list_player();
        ~media_list_player();

        bool open( libvlc_instance_t* ) override;
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

    protected:
        void events_attach( bool attach ) override;

        libvlc_media_list_t* get_ml() const
            { return _media_list; }
        libvlc_media_list_player_t* get_mlp() const
            { return _media_list_player; }

    private:
        libvlc_media_list_t* _media_list;
        libvlc_media_list_player_t* _media_list_player;
        playback_mode_e _mode;
    };
}

typedef vlc::player vlc_player;
