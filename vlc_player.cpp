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

#include "vlc_player.h"

#include <cassert>

#include <algorithm>

using namespace vlc;

player::player()
    : _libvlc_instance( 0 ),
      _video( _player ), _audio( _player ), _subtitles( _player ),
      _mode( mode_single ), _current_idx( -1 )
{
}

player::~player()
{
    close();
}

bool player::open( libvlc_instance_t* inst )
{
    _libvlc_instance = inst;

    if( _player.open( inst ) ) {
        if( !_callbacks.empty() )
            events_attach( true );

        return true;
    }

    return false;
}

void player::close()
{
    if( !_callbacks.empty() )
        events_attach( false );

    _callbacks.clear();

    _player.close();
    clear_items();
    _libvlc_instance = 0;
}

int player::add_media( const char * mrl_or_path,
                       unsigned optc, const char **optv,
                       unsigned trusted_optc, const char **trusted_optv,
                       bool is_path /*= false*/ )
{
    if( !is_open() )
        return -1;

    libvlc_media_t* media = is_path ?
                            libvlc_media_new_path( _libvlc_instance, mrl_or_path ) :
                            libvlc_media_new_location( _libvlc_instance, mrl_or_path );

    if( !media )
        return -1;

    unsigned i;
    for( i = 0; i < optc; ++i )
        libvlc_media_add_option_flag( media, optv[i], libvlc_media_option_unique );

    for( i = 0; i < trusted_optc; ++i )
        libvlc_media_add_option_flag( media, trusted_optv[i],
                                      libvlc_media_option_unique | libvlc_media_option_trusted );

    playlist_item item = { vlc::media( media, false ), false };
    playlist_it it = _playlist.insert( _playlist.end(), item );

    return it - _playlist.begin();
}

int player::add_media( const vlc::media& media )
{
    if( !is_open() )
        return -1;

    playlist_item item = { media, false };
    playlist_it it = _playlist.insert( _playlist.end(), item );

    return it - _playlist.begin();
}

bool player::delete_item( unsigned idx )
{
    unsigned sz = _playlist.size();
    assert( _current_idx >= 0 && unsigned( _current_idx ) < sz );

    if( sz && idx < sz ) {
        if( _current_idx >= 0 &&
            ( unsigned( _current_idx ) > idx ||
              ( unsigned( _current_idx ) == idx &&
                unsigned( _current_idx ) == ( sz - 1 ) ) ) )
        {
            //if current is after idx or is idx and is last
            --_current_idx;
        }

        playlist_it it = ( _playlist.begin() + idx );

        _playlist.erase( it );
        assert( _current_idx < 0 || unsigned( _current_idx ) < _playlist.size() );

        return true;
    }

    return false;
}

void player::clear_items()
{
    _playlist.clear();

    _current_idx = -1;

    //we shouldn't stop/clear current playing item in _player
    //since some users want to refill playlist
    //while current media is still playing
}

int player::item_count()
{
    return _playlist.size();
}

void player::disable_item( unsigned idx, bool disable )
{
    if( idx >= _playlist.size() )
        return;

    _playlist[idx].disabled = disable;
}

bool player::is_item_disabled( unsigned idx )
{
    if( idx >= _playlist.size() )
        return false;

    return _playlist[idx].disabled;
}

void player::advance_item( unsigned idx, int count )
{
    if( idx >= _playlist.size() ||
        int( idx ) + count < 0 ||
        idx + count >= _playlist.size() ||
        0 == count )
    {
        return;
    }

    playlist_item save_item = _playlist[idx];
    _playlist.erase( _playlist.begin() + idx );
    _playlist.insert( _playlist.begin() + idx + count, save_item );

    if( _current_idx < 0 )
        return;

    if( unsigned( _current_idx ) ==  idx )
        _current_idx = idx + count;
    else if( count > 0 && idx < unsigned( _current_idx ) && unsigned( _current_idx ) <= idx + count )
        --_current_idx;
    else if( count < 0 && idx + count <= unsigned( _current_idx ) && idx > unsigned( _current_idx ) )
        ++_current_idx;
}

vlc::media player::get_media( unsigned idx )
{
    if( idx >= _playlist.size() )
        return vlc::media();

    return _playlist[idx].media;
}

int player::find_media_index( const vlc::media& media )
{
    playlist_cit it =
        std::find_if( _playlist.begin(), _playlist.end(),
            [&media] ( const playlist_item& item ) -> bool {
                return media == item.media;
            }
        );

    return ( _playlist.end() == it ) ? -1 : ( it - _playlist.begin() );
}

int player::current_item()
{
    return _current_idx;
}

void player::set_current( unsigned idx )
{
    if( idx < _playlist.size() ) {
        _current_idx = idx;
        _player.set_media( _playlist[_current_idx].media );
    }
}

void player::internal_play( int idx )
{
    if( idx < 0 || static_cast<unsigned>( idx ) >= _playlist.size() ) {
        return;
    }

    if( _player.current_media() != _playlist[idx].media ||
        libvlc_Ended == get_state() )
    {
        set_current( idx );
    }

    _player.play();
}

void player::play()
{
    if( _playlist.empty() ) {
        //special case for empty playlist ( usually after clear_items() )
        _player.play();
    } else {
        const unsigned sz = _playlist.size();
        int idx = _current_idx;
        if( idx < 0 )
            idx = 0;
        else if( unsigned( idx ) >= sz )
            idx = ( sz - 1 );

        internal_play( idx );
    }
}

bool player::play( unsigned idx )
{
    if( idx < _playlist.size() ) {
        internal_play( idx );
        return true;
    }

    return false;
}

int player::find_valid_item( int start_from_idx, bool forward )
{
    const int sz = _playlist.size();

    if( !sz )
        return -1;

    if( forward ) {
        if( start_from_idx < 0 )
            start_from_idx = 0;
        else if( start_from_idx > sz )
            start_from_idx = sz;

        for( int i = start_from_idx; i < sz; ++i ) {
            if( !_playlist[i].disabled )
                return i;
        }

        if( mode_loop == _mode ) {
            for( int i = 0; i < start_from_idx; ++i ) {
                if( !_playlist[i].disabled )
                    return i;
            }
        }
    } else {
        if( start_from_idx > sz - 1 )
            start_from_idx = sz - 1;

        if( start_from_idx < -1 )
            start_from_idx = -1;

        for( int i = start_from_idx; i >= 0; --i ) {
            if( !_playlist[i].disabled )
                return i;
        }

        if( mode_loop == _mode ) {
            for( int i = sz - 1; i > start_from_idx; --i ) {
                if( !_playlist[i].disabled )
                    return i;
            }
        }
    }
    return -1;
}

void player::prev()
{
    if( _playlist.empty() )
        return;

    internal_play( find_valid_item( _current_idx - 1, false ) );
}

void player::get_media_sub_items( const vlc::media& media, playlist_t* out )
{
    assert( out );

    if( !media || !out )
        return;

    libvlc_media_list_t* sub_items = libvlc_media_subitems( media.libvlc_media_t() );
    if( !sub_items )
        return;

    libvlc_media_list_lock( sub_items );

    int sub_items_count = libvlc_media_list_count( sub_items );

    for( int i = 0; i < sub_items_count; ++i ) {
        libvlc_media_t* sub_item = libvlc_media_list_item_at_index( sub_items, i );
        if( sub_item ) {
            playlist_item item = { vlc::media( sub_item, false ), false };
            out->push_back( item );
        }
    }

    libvlc_media_list_unlock( sub_items );

    libvlc_media_list_release( sub_items );
}

bool player::try_expand_current()
{
    vlc::media current_media =
        ( _current_idx < 0 || unsigned( _current_idx ) >= _playlist.size() ) ?
            _player.current_media() : _playlist[_current_idx].media;

    playlist_t sub_items;
    get_media_sub_items( current_media, &sub_items );

    if( !sub_items.empty() ) {
        playlist_it insert_it;
        if( _current_idx < 0 ) {
            insert_it = _playlist.begin();
        } else if( unsigned( _current_idx ) >= _playlist.size() ) {
            insert_it = _playlist.end();
        } else {
            insert_it = _playlist.erase( _playlist.begin() + _current_idx );
        }
        _current_idx = insert_it - _playlist.begin();
        _playlist.insert( insert_it, sub_items.begin(), sub_items.end() );
        return true;
    }

    return false;
}

void player::next()
{
    bool expanded = try_expand_current();

    if( _playlist.empty() )
        return;

    internal_play(
        find_valid_item(
            expanded ? _current_idx : _current_idx + 1, true ) );
}

void player::pause()
{
    _player.pause();
}

void player::togglePause()
{
    _player.togglePause();
}

void player::stop()
{
    _player.stop();
}

float player::get_rate()
{
    if( !_player.is_open() )
        return 1.f;

    return libvlc_media_player_get_rate( _player.get_mp() );
}

void player::set_rate( float rate )
{
    if( !_player.is_open() )
        return;

    libvlc_media_player_set_rate( _player.get_mp(), rate );
}

float player::get_position()
{
    if( !is_open() )
        return 0.f;

    float p = libvlc_media_player_get_position( _player.get_mp() );

    return p < 0 ? 0 : p;
}

void player::set_position( float p )
{
    if( !is_open() )
        return;

    libvlc_media_player_set_position( _player.get_mp(), p );
}

libvlc_time_t player::get_time()
{
    if( !is_open() )
        return 0;

    libvlc_time_t t = libvlc_media_player_get_time( _player.get_mp() );

    return t < 0 ? 0 : t ;
}

void player::set_time( libvlc_time_t t )
{
    if( !is_open() )
        return;

    libvlc_media_player_set_time( _player.get_mp(), t );
}

libvlc_time_t player::get_length()
{
    if( !_player.is_open() )
        return 0;

    libvlc_time_t t = libvlc_media_player_get_length( _player.get_mp() );

    return t < 0 ? 0 : t ;
}

float player::get_fps()
{
    if( !_player.is_open() )
        return 0;

    return libvlc_media_player_get_fps( _player.get_mp() );
}

playback_mode_e player::get_playback_mode()
{
    return _mode;
}

void player::set_playback_mode( playback_mode_e m )
{
    _mode = m;
}

void player::event_proxy( const libvlc_event_t* e, void* param )
{
    if( !param )
        return;

    static_cast<player*>( param )->event( e );
}

void player::event( const libvlc_event_t* e )
{
    for( auto* callback : _callbacks )
        callback->media_player_event( e );
}

void player::events_attach( bool attach )
{
    if( !is_open() )
        return;

   libvlc_event_manager_t* em =
        libvlc_media_player_event_manager( get_mp() );
    if( !em )
        return;

    for( int e = libvlc_MediaPlayerMediaChanged; e <= libvlc_MediaPlayerVout; ++e ) {
        switch( e ){
        case libvlc_MediaPlayerMediaChanged:
        case libvlc_MediaPlayerNothingSpecial:
        case libvlc_MediaPlayerOpening:
        case libvlc_MediaPlayerBuffering:
        case libvlc_MediaPlayerPlaying:
        case libvlc_MediaPlayerPaused:
        case libvlc_MediaPlayerStopped:
        case libvlc_MediaPlayerForward:
        case libvlc_MediaPlayerBackward:
        case libvlc_MediaPlayerEndReached:
        case libvlc_MediaPlayerEncounteredError:
        case libvlc_MediaPlayerTimeChanged:
        case libvlc_MediaPlayerPositionChanged:
        case libvlc_MediaPlayerSeekableChanged:
        case libvlc_MediaPlayerPausableChanged:
        case libvlc_MediaPlayerTitleChanged:
        //case libvlc_MediaPlayerSnapshotTaken:
        case libvlc_MediaPlayerLengthChanged:
        //case libvlc_MediaPlayerVout:
            if( attach )
                libvlc_event_attach( em, e, event_proxy, this );
            else
                libvlc_event_detach( em, e, event_proxy, this );
            break;
        }
    }
}

void player::register_callback( media_player_events_callback* callback )
{
    if( _callbacks.empty() )
        events_attach( true );

    _callbacks.push_back( callback );
}

void player::unregister_callback( media_player_events_callback* callback )
{
    auto it = std::find( _callbacks.begin(), _callbacks.end(), callback );
    if( it != _callbacks.end() ) {
        _callbacks.erase( it );

        if( _callbacks.empty() )
            events_attach( false );
    }
}

void player::swap( player* p )
{
    if( this == p )
        return;

    libvlc_instance_t *const tmp_libvlc = p->_libvlc_instance;
    p->_libvlc_instance = _libvlc_instance;
    _libvlc_instance = tmp_libvlc;

    if( !p->_callbacks.empty() )
        p->events_attach( false );

    if( !_callbacks.empty() )
        events_attach( false );

    _player.swap( &( p->_player ) );

    _playlist.swap( p->_playlist );

    const playback_mode_e tmp_mode = p->_mode;
    p->_mode = _mode;
    _mode = tmp_mode;

    const int tmp_current_idx = p->_current_idx;
    p->_current_idx = _current_idx;
    _current_idx = tmp_current_idx;

    if( !p->_callbacks.empty() )
        p->events_attach( true );

    if( !_callbacks.empty() )
        events_attach( true );
}
