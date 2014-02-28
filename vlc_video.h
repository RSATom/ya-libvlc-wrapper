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

#pragma once

#include "vlc_basic_player.h"

namespace vlc
{
    class video
    {
    public:
        video( vlc::basic_player& player )
            : _player( player ) {};

        unsigned track_count();

        //can return -1 if there is no active video track
        int get_track();
        void set_track( unsigned );

        bool has_vout();

        float get_contrast()
            { return get_ajust_filter_var( libvlc_adjust_Contrast, 1.0f ); }
        void set_contrast( float val )
            { set_ajust_filter_var( libvlc_adjust_Contrast, val ); }

        float get_brightness()
            { return get_ajust_filter_var( libvlc_adjust_Brightness, 1.0f ); }
        void set_brightness( float val )
            { set_ajust_filter_var( libvlc_adjust_Brightness, val ); }

        float get_hue()
            { return get_ajust_filter_var( libvlc_adjust_Hue, .0f ); }
        void set_hue( float val )
            { set_ajust_filter_var( libvlc_adjust_Hue, val ); }

        float get_saturation()
            { return get_ajust_filter_var( libvlc_adjust_Saturation, 1.0f ); }
        void set_saturation( float val )
            { set_ajust_filter_var( libvlc_adjust_Saturation, val ); }

        float get_gamma()
            { return get_ajust_filter_var( libvlc_adjust_Gamma, 1.0f ); }
        void set_gamma( float val )
            { set_ajust_filter_var( libvlc_adjust_Gamma, val ); }

    private:
        float get_ajust_filter_var( libvlc_video_adjust_option_t option, float def_v );
        void set_ajust_filter_var( libvlc_video_adjust_option_t option, float val );

    private:
        vlc::basic_player& _player;
    };
};
