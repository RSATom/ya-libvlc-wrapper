#pragma once

#include <vector>

#include "vlc_basic_player.h"

namespace vlc
{
    class basic_vmem_wrapper {
    public:
        basic_vmem_wrapper( vlc::basic_player& player )
            : _player( player ) {}
        bool open();
        void close();

    private:
        //for libvlc_video_set_format_callbacks
        static unsigned video_format_proxy( void **opaque, char *chroma,
                                            unsigned *width, unsigned *height,
                                            unsigned *pitches, unsigned *lines )
            { return reinterpret_cast<basic_vmem_wrapper*>( *opaque )->video_format_cb( chroma,
                                                                                        width, height,
                                                                                        pitches, lines ); }
        static void video_cleanup_proxy( void *opaque )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_cleanup_cb(); }
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        static void* video_fb_lock_proxy( void *opaque, void **planes )
            { return reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_lock_cb(planes); }
        static void  video_fb_unlock_proxy( void *opaque, void *picture, void *const *planes )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_unlock_cb( picture, planes ); }
        static void  video_fb_display_proxy( void *opaque, void *picture )
            { reinterpret_cast<basic_vmem_wrapper*>( opaque )->video_display_cb( picture ); }
        //end (for libvlc_video_set_callbacks)

    protected:
        //for libvlc_video_set_format_callbacks
        virtual unsigned video_format_cb( char *chroma,
                                          unsigned *width, unsigned *height,
                                          unsigned *pitches, unsigned *lines ) = 0;
        virtual void video_cleanup_cb() = 0;
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        virtual void* video_lock_cb( void **planes ) = 0;
        virtual void  video_unlock_cb( void *picture, void *const *planes ) = 0;
        virtual void  video_display_cb( void *picture ) = 0;
        //end (for libvlc_video_set_callbacks)

    private:
        vlc::basic_player& _player;
    };

    const char DEF_CHROMA[] = "RV32";
    enum {
        DEF_PIXEL_BYTES = 4,

        original_media_width = 0,
        original_media_height = 0
    };

    class vmem : public basic_vmem_wrapper
    {
    public:
        vmem( vlc::basic_player& player );

        //0 - use size same as source has
        void set_desired_size( unsigned width, unsigned height );

        unsigned width() const { return _media_width; }
        unsigned height() const { return _media_height; }
        const std::vector<char>& frame_buf() { return _frame_buf; }

    protected:
        //on_format_setup/on_frame_ready/on_frame_cleanup will come from worker thread
        virtual void on_format_setup() {}
        virtual void on_frame_ready( const std::vector<char>& frame_buf ) = 0;
        virtual void on_frame_cleanup() = 0;

    private:
        //for libvlc_video_set_format_callbacks
        virtual unsigned video_format_cb( char *chroma,
                                          unsigned *width, unsigned *height,
                                          unsigned *pitches, unsigned *lines );
        virtual void video_cleanup_cb();
        //end (for libvlc_video_set_format_callbacks)

        //for libvlc_video_set_callbacks
        virtual void* video_lock_cb( void **planes );
        virtual void  video_unlock_cb( void *picture, void *const *planes );
        virtual void  video_display_cb( void *picture );
        //end (for libvlc_video_set_callbacks)

    private:
        std::vector<char>  _frame_buf;
        unsigned           _desired_width;
        unsigned           _desired_height;
        unsigned           _media_width;
        unsigned           _media_height;
    };
};
