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

#include "vlc_media_list_player.h"

using namespace vlc;

media_list_player::media_list_player()
    : _media_list( nullptr ), _media_list_player( nullptr )
{
}

media_list_player::~media_list_player()
{
    close();
}

void media_list_player::events_attach( bool attach )
{
     vlc::playlist_player_core::events_attach( attach );

    if( !is_open() )
        return;

    libvlc_event_manager_t* em =
        libvlc_media_list_event_manager( get_ml() );

    if( !em )
        return;

    for( int e = libvlc_MediaListItemAdded; e <= libvlc_MediaListWillDeleteItem; ++e ) {
        switch( e ){
            case libvlc_MediaListItemAdded:
            case libvlc_MediaListWillAddItem:
            case libvlc_MediaListItemDeleted:
            case libvlc_MediaListWillDeleteItem:
            case libvlc_MediaListPlayerPlayed:
            case libvlc_MediaListPlayerNextItemSet:
            case libvlc_MediaListPlayerStopped:
            if( attach )
                libvlc_event_attach( em, e, event_proxy, this );
            else
                libvlc_event_detach( em, e, event_proxy, this );
            break;
        }
    }
}
bool media_list_player::open( libvlc_instance_t* inst )
{
    if( player_core::open( inst ) ) {
        _media_list = libvlc_media_list_new( inst );
    }

    if( _media_list ) {
        _media_list_player = libvlc_media_list_player_new( inst );
    }

    if( _media_list_player ) {
        libvlc_media_list_player_set_media_list( _media_list_player, _media_list );
        libvlc_media_list_player_set_media_player( _media_list_player, get_mp() );
        return true;
    }

    close();
    return false;
}

void media_list_player::close()
{
    if( _media_list_player ) {
        libvlc_media_list_player_release( _media_list_player );
        _media_list_player = nullptr;
    }

    if( _media_list ) {
        libvlc_media_list_release( _media_list );
        _media_list = nullptr;
    }

    player_core::close();
}

void media_list_player::play()
{
    if( _media_list_player )
        libvlc_media_list_player_play( _media_list_player );
}

bool media_list_player::play( unsigned idx )
{
    if( !_media_list_player )
        return false;

    return
        libvlc_media_list_player_play_item_at_index( _media_list_player, idx ) == 0;
}

void media_list_player::next()
{
    if( _media_list_player )
        libvlc_media_list_player_next( _media_list_player );
}

void media_list_player::prev()
{
    if( _media_list_player )
        libvlc_media_list_player_previous( _media_list_player );
}

playback_mode_e media_list_player::get_playback_mode()
{
    return _mode;
}

void media_list_player::set_playback_mode( playback_mode_e m )
{
    libvlc_playback_mode_t libvlcMode = libvlc_playback_mode_default;
    switch( m ) {
    case mode_single:
        //not supported
        return;
    case mode_normal:
        libvlcMode = libvlc_playback_mode_default;
        break;
    case mode_loop:
        libvlcMode = libvlc_playback_mode_loop;
    }

    _mode = m;
    libvlc_media_list_player_set_playback_mode( _media_list_player, libvlcMode );
}

int media_list_player::add_media( const vlc::media& media )
{
    if( !_media_list || item_count() >= PLAYLIST_MAX_SIZE || !media )
        return -1;

    int idx = -1;

    libvlc_media_list_lock( _media_list );
    if( 0 == libvlc_media_list_add_media( _media_list, media.libvlc_media_t() ) ) {
        idx = libvlc_media_list_index_of_item( _media_list, media.libvlc_media_t() );
    }
    libvlc_media_list_unlock( _media_list );

    return idx;
}

void media_list_player::advance_item( unsigned /*idx*/, int /*count*/ )
{
    //not supported
}

bool media_list_player::delete_item( unsigned idx )
{
    if( !_media_list )
        return false;

    libvlc_media_list_lock( _media_list );
    bool ret = libvlc_media_list_remove_index( _media_list, idx ) == 0;
    libvlc_media_list_unlock( _media_list );

    return ret;
}

void media_list_player::clear_items()
{
    if( !_media_list )
        return;

    libvlc_media_list_lock( _media_list );
    for( int i = libvlc_media_list_count( _media_list ); i > 0; --i ) {
        libvlc_media_list_remove_index( _media_list, i - 1 );
    }
    libvlc_media_list_unlock( _media_list );
}

unsigned media_list_player::item_count()
{
    if( !_media_list )
        return 0;

    unsigned count = 0;
    libvlc_media_list_lock( _media_list );
    count = libvlc_media_list_count( _media_list );
    libvlc_media_list_unlock( _media_list );

    return count;
}

vlc::media media_list_player::get_media( unsigned idx )
{
    if( !_media_list )
        return vlc::media();

    libvlc_media_list_lock( _media_list );
    libvlc_media_t* libvlcMedia = libvlc_media_list_item_at_index( _media_list, idx );
    libvlc_media_list_unlock( _media_list );

    if( libvlcMedia )
        return vlc::media( libvlcMedia, false );
    else
        return vlc::media();
}

int media_list_player::find_media_index( const vlc::media& media )
{
    if( !_media_list )
        return -1;

    libvlc_media_list_lock( _media_list );
    int index = libvlc_media_list_index_of_item( _media_list, media.libvlc_media_t() );
    libvlc_media_list_unlock( _media_list );
    return index;
}

int media_list_player::current_item()
{
    if( !is_open() )
        return -1;

    libvlc_media_t* media =
        libvlc_media_player_get_media( get_mp() );

    if( !media )
        return -1;

    libvlc_media_list_lock( _media_list );
    int index = libvlc_media_list_index_of_item( _media_list, media );
    libvlc_media_list_unlock( _media_list );

    return index;
}

void media_list_player::set_current( unsigned /*idx*/ )
{
    //not supported
}

void media_list_player::disable_item( unsigned /*idx*/, bool /*disable*/ )
{
    //not supported
}

bool media_list_player::is_item_disabled( unsigned /*idx*/ )
{
    //not supported
    return false;
}

void media_list_player::set_item_data( unsigned /*idx*/, const std::string& )
{
    //not supported
}

const std::string& media_list_player::get_item_data( unsigned /*idx*/ )
{
    //not supported
    static std::string stub;
    return stub;
}
