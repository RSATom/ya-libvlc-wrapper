/*****************************************************************************
* Copyright (c) 2013-2014, Sergey Radionov <rsatom_gmail.com>
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

#include "vlc_player.h"

#include <cassert>

using namespace vlc;

player::player()
    : _libvlc_instance( 0 ),
      _video( _player ), _audio( _player ), _subtitles( _player ), _current_media( _player ),
      _mode( mode_normal ), _current_idx( -1 )
{
}

player::~player()
{
    close();
}

void player::libvlc_event_proxy( const struct libvlc_event_t* event, void* user_data )
{
    static_cast<player*>( user_data )->libvlc_event( event );
}

void player::libvlc_event( const struct libvlc_event_t* event )
{
    //come from another thread
    if( libvlc_MediaPlayerEndReached == event->type ||
        libvlc_MediaPlayerEncounteredError == event->type )
    {
        //to avoid deadlock we should execute commands on another thread
        thread th( &player::next, this );
        th.detach();
    }
}

bool player::open( libvlc_instance_t* inst )
{
    _libvlc_instance = inst;
    if( _player.open( inst ) ) {
        libvlc_event_manager_t* em = libvlc_media_player_event_manager( _player.get_mp() );

        libvlc_event_attach( em, libvlc_MediaPlayerEndReached, libvlc_event_proxy, this );
        libvlc_event_attach( em, libvlc_MediaPlayerEncounteredError, libvlc_event_proxy, this );

        return true;
    }
    return false;
}

void player::close()
{
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

    lock_guard<mutex_t> lock( _playlist_guard );
    playlist_item item = { vlc::media( media, false ) };
    playlist_it it = _playlist.insert( _playlist.end(), item );

    return it - _playlist.begin();
}

bool player::delete_item( unsigned idx )
{
    lock_guard<mutex_t> lock( _playlist_guard );

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
    lock_guard<mutex_t> lock( _playlist_guard );

    _playlist.clear();

    _current_idx = -1;

    //we shouldn't stop/clear current playing item in _player
    //since some users want to refill playlist
    //while current media is still playing
}

int player::current_item()
{
    lock_guard<mutex_t> lock( _playlist_guard );

    return _current_idx;
}

int player::item_count()
{
    lock_guard<mutex_t> lock( _playlist_guard );

    return _playlist.size();
}

vlc::media player::get_media( unsigned idx )
{
    lock_guard<mutex_t> lock( _playlist_guard );

    if( idx >= _playlist.size() )
        return vlc::media();

    return _playlist[idx].media;
}

void player::set_current( unsigned idx )
{
    lock_guard<mutex_t> lock( _playlist_guard );

    if( idx < _playlist.size() ) {
        _current_idx = idx;
        _player.set_media( _playlist[_current_idx].media );
    }
}

void player::internalPlay( int idx )
{
    if( idx < 0 || static_cast<unsigned>( idx ) >= _playlist.size() ) {
        return;
    }

    if( _player.current_media() != _playlist[idx].media )
        set_current( idx );

    _player.play();
}

void player::play()
{
    lock_guard<mutex_t> lock( _playlist_guard );

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

        internalPlay( idx );
    }
}

bool player::play( unsigned idx )
{
    lock_guard<mutex_t> lock( _playlist_guard );

    if( idx < _playlist.size() ) {
        internalPlay( idx );
        return true;
    }

    return false;
}

void player::prev()
{
    lock_guard<mutex_t> lock( _playlist_guard );

    if( _playlist.empty() )
        return;

    const unsigned sz = _playlist.size();

    if( _current_idx <= 0 ) {
        if( mode_loop == _mode )
            internalPlay( sz - 1 );
    } else if( unsigned( _current_idx ) >= sz ) {
        internalPlay( sz - 1 );
    } else
        internalPlay( _current_idx - 1 );
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
            playlist_item item = { vlc::media( sub_item, false ) };
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
    lock_guard<mutex_t> lock( _playlist_guard );

    bool expanded = try_expand_current();

    if( _playlist.empty() )
        return;

    const unsigned sz = _playlist.size();

   if( expanded )
        internalPlay( _current_idx );
    else if( ( _current_idx < 0 && sz > 0 ) ||
             ( unsigned( _current_idx ) == ( sz - 1 ) && ( mode_loop == _mode ) ) )
    {
       //if current not set or current is last and loop mode enabled
        internalPlay( 0 );
    } else
        internalPlay( _current_idx + 1 );
}

void player::pause()
{
    _player.pause();
}

void player::togglePause()
{
    _player.togglePause();
}

void player::stop( bool async /*= false*/ )
{
    _player.stop( async );
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
