/*******************************************************************************
* Copyright © 2014, Sergey Radionov <rsatom_gmail.com>
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
#include <string>

namespace vlc
{
    class media
    {
    public:
        media();
        explicit media( ::libvlc_media_t*, bool needs_retain );
        media( const media& other );
        ~media();

        media& operator= ( const media& m );

        bool operator== ( const media& m ) const
            { return m_media == m.m_media; }
        bool operator!= ( const media& m ) const
            { return m_media != m.m_media; }
        bool operator== ( ::libvlc_media_t* m ) const
            { return m_media == m; }
        bool operator!= ( ::libvlc_media_t* m ) const
            { return m_media != m; }
        operator bool() const
            { return m_media != 0; }

        ::libvlc_media_t* libvlc_media_t() const
            { return m_media; }

        bool is_parsed() const;
        void parse();

        std::string mrl() const;
        std::string meta( ::libvlc_meta_t meta_id ) const;
        void set_meta( ::libvlc_meta_t meta_id, const std::string& meta );

    private:
        void release_media();

    private:
        ::libvlc_media_t* m_media;
    };
};
