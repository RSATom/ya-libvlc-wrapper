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

#include <vlc/vlc.h>

#include "vlc_media.h"

namespace vlc
{
    class basic_player
    {
    public:
        basic_player();
        ~basic_player();

        bool open( libvlc_instance_t* inst );
        void close();

        void swap( basic_player* );

        bool is_open() const { return _mp != 0; }

        void play();
        void pause();
        void togglePause();
        void stop();

        void set_media( const vlc::media& );

        vlc::media current_media();

        libvlc_state_t get_state();

        bool is_playing() { return libvlc_Playing == get_state(); }
        bool is_paused() { return libvlc_Paused == get_state(); }
        bool is_stopped() { return libvlc_Stopped == get_state(); }

        libvlc_media_player_t* get_mp() const
            { return _mp; }

    private:
        libvlc_instance_t*     _libvlc_instance;
        libvlc_media_player_t* _mp;
    };
};
