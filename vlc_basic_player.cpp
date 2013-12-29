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

#include "vlc_basic_player.h"

#include <boost/thread/thread.hpp>

using namespace vlc;

basic_player::basic_player()
    : _libvlc_instance( 0 ), _mp( 0 )
{
}

basic_player::~basic_player()
{
    close();
}

libvlc_state_t basic_player::get_state()
{
    if( !is_open() )
        return libvlc_NothingSpecial;

    return libvlc_media_player_get_state( _mp );
}

bool basic_player::open( libvlc_instance_t* inst )
{
    if( !inst )
        return false;

    if( is_open() )
        close();

    _libvlc_instance = inst;
    _mp = libvlc_media_player_new( inst );

    return 0 != _mp;
}

void basic_player::close()
{
    if( _mp ) {
        libvlc_media_player_release( _mp );
        _mp = 0;
    }

    _libvlc_instance = 0;
}

void basic_player::play()
{
    if( is_open() )
        libvlc_media_player_play( _mp );
}

void basic_player::pause()
{
    if( is_open() )
        libvlc_media_player_set_pause( _mp, true );
}

void basic_player::togglePause()
{
    if( is_open() )
        libvlc_media_player_pause( _mp );
}

void basic_player::stop( bool async /*= false*/ )
{
    if( !is_open() )
        return;

    if( async )
        boost::thread( boost::bind( libvlc_media_player_stop, _mp ) );
    else
        libvlc_media_player_stop( _mp );
}

libvlc_media_t* basic_player::current_media()
{
    if( is_open() )
        return libvlc_media_player_get_media( _mp );
    else
        return 0;
}

void basic_player::set_media( libvlc_media_t* media )
{
    if( is_open() )
        return libvlc_media_player_set_media( _mp, media );
}
