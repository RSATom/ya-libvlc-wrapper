#pragma once

#if __cplusplus >= 201103L
#define LW_USE_STD_THREAD 1
#endif

#ifdef LW_USE_STD_THREAD
#include <thread>
#include <mutex>
#else
#include <boost/thread.hpp>
#endif

namespace vlc
{
#ifdef LW_USE_STD_THREAD

    typedef std::mutex mutex;
    typedef std::recursive_mutex recursive_mutex;
    typedef std::thread thread;

    template<typename Mutex>
    struct lock_guard {
        explicit lock_guard( Mutex& m )
            : _guard( m ) {}
        std::lock_guard<Mutex> _guard;
    };

#else

    typedef boost::mutex mutex;
    typedef boost::recursive_mutex recursive_mutex;
    typedef boost::thread thread;

    template<typename Mutex>
    struct lock_guard {
        explicit lock_guard( Mutex& m )
            : _guard( m ) {}
        boost::lock_guard<Mutex> _guard;
    };

#endif
}
