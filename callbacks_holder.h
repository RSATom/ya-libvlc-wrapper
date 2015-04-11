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

#include <cassert>
#include <deque>
#include <functional>
#include <algorithm>
#include <mutex>

template<typename callback_t>
class callbacks_holder
{
protected:
    ~callbacks_holder();

public:
    virtual void register_callback( callback_t* );
    virtual void unregister_callback( callback_t* );

protected:
    void for_each_callback( const std::function<void( callback_t* )>& ) const;
    bool has_callbacks() const
        { return !_callbacks.empty(); }
    void clear_callbacks()
        { _callbacks.clear(); }

private:
    typedef std::deque<callback_t*> callbacks_t;

private:
    mutable std::mutex _callbacks_guard;
    callbacks_t _callbacks;
};

template<typename callback_t>
callbacks_holder<callback_t>::~callbacks_holder()
{
    assert( _callbacks.empty() );
}

template<typename callback_t>
void callbacks_holder<callback_t>::register_callback( callback_t* callback )
{
    std::lock_guard<std::mutex> lock( _callbacks_guard );
    assert( _callbacks.end() == std::find( _callbacks.begin(), _callbacks.end(), callback ) );
    _callbacks.push_back( callback );
}

template<typename callback_t>
void callbacks_holder<callback_t>::unregister_callback( callback_t* callback )
{
    std::lock_guard<std::mutex> lock( _callbacks_guard );
    auto it = std::find( _callbacks.begin(), _callbacks.end(), callback );
    if( it != _callbacks.end() ) {
        _callbacks.erase( it );
    }
}

template<typename callback_t>
void callbacks_holder<callback_t>::for_each_callback( const std::function<void( callback_t* )>& f ) const
{
    std::lock_guard<std::mutex> lock( _callbacks_guard );
    std::for_each( _callbacks.begin(), _callbacks.end(), f );
}
