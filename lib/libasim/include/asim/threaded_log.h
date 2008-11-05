/*
 *Copyright (C) 2004-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef THREADED_LOG_H
#define THREADED_LOG_H

#include "message_handler_log.h"
#include <queue>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sched.h>
#include <stdlib.h>

typedef  int LONG;
typedef  int DWORD; 
typedef void * LPVOID;
#define WINAPI 

// the object to be started - on a given thread
struct win32_thread_obj
{
    virtual ~win32_thread_obj() {}
    virtual void operator()() = 0;
};

 

 

struct win32_thread_manager
{
    typedef win32_thread_obj thread_obj_base;
    static void sleep( int nMillisecs) 
    { 
//        usleep( nMillisecs); 
        sched_yield();
    }
    static void create_thread( win32_thread_obj & obj)
    {
        pthread_t dwThreadID; //warning, this goes out of scope.
        pthread_create(&dwThreadID, NULL, win32_thread_manager::ThreadProc,  &obj);
    }

private:
    static void * ThreadProc( LPVOID lpData)
    {
        win32_thread_obj * pThread = ( win32_thread_obj *)lpData;
        ( *pThread)();
      std::cout << "thread proc returning" << std::endl;
        return NULL;
    }
};

template< class char_type, class traits_type = std::char_traits< char_type> >
    class thread_safe_log_writer
{
    typedef thread_safe_log_writer< char_type, traits_type> this_class;
    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    typedef std::basic_string< char_type, traits_type> string_type;

    // forward declaration
    struct thread_info;
    friend struct thread_info;

    // thread-related definitions
    typedef win32_thread_manager thread_manager;
    typedef typename thread_manager::thread_obj_base thread_obj_base;

    // so that from our thread we know the object we're manipulating
    struct thread_info : public thread_obj_base
    {
        thread_info()
            :               m_pThis( NULL), m_bHasFinished( false)

        {}

        /* virtual */ void operator()()
        {
            while ( true)
            {
                assert(m_pThis != NULL);
                std::string * pstr = NULL;
                {
                    pthread_mutex_lock( m_pThis->m_cs);
                    // get the string
                    if ( m_pThis->m_astrMessages.size() > 0)
                    {
                        pstr = m_pThis->m_astrMessages.front();
                        m_pThis->m_astrMessages.pop();
                    }

                    // ... only when there are no more messages,
                    //    will we ask if we should be destructed

                    else if ( m_pThis->m_bShouldBeDestructed)
                    {
                        // signal to the other thread we've finished
                        m_bHasFinished = true;
                        pthread_mutex_unlock( m_pThis->m_cs);                                                       
                        return;
                    }
                   
                }

                // write the string
                if ( pstr)
                {
                    cout << *pstr;
//                    m_pThis->m_underlyingLog << *pstr;
                    delete pstr;
                }

                else
                    // nothing to write - wait
                    thread_manager::sleep(1);              

                pthread_mutex_unlock( m_pThis->m_cs);   //this may
                                                            //have to
                                                            //go at
                                                            //end of function
            }
        }

        this_class * m_pThis;
        volatile bool m_bHasFinished;
    };

public:

    void add_message( const string_type & str)
    {
        pthread_mutex_lock(m_cs);
        m_astrMessages.push( new string_type( str));
        pthread_mutex_unlock(m_cs);
    }

    thread_safe_log_writer( ostream_type & underlyingLog)
        :           m_bShouldBeDestructed( false), m_underlyingLog( underlyingLog)

    {
        m_info.m_pThis = this;
        m_cs =  new pthread_mutex_t;
        pthread_mutex_init(m_cs, NULL);
        thread_manager::create_thread( m_info);
    }

    ~thread_safe_log_writer()
    {
        // signal to the other thread we're about to be
        // destructed
        {        
            std::cout << "trying to delete writer thread" << std::endl;
            pthread_mutex_lock(m_cs);
            m_bShouldBeDestructed = true;
            pthread_mutex_unlock(m_cs);
        }

        // wait while the other thread writes all messages   
        while ( true)
        {
            pthread_mutex_lock(m_cs);
            if ( m_info.m_bHasFinished)
                // the other thread has finished
                break;
            pthread_mutex_unlock(m_cs);
        }       
        pthread_mutex_unlock(m_cs);
        std::cout << "threadsafelogwriter deleted" << std::endl;
    }

    pthread_mutex_t * cs() const { return m_cs; }

private:

    // the critical section used for thread-safe locking
//    mutable CCriticalSection m_cs;
    pthread_mutex_t * m_cs;

    // needed to create the other thread
    thread_info m_info;
    volatile bool m_bShouldBeDestructed;

    ostream_type & m_underlyingLog;

    std::queue< string_type*> m_astrMessages;

};

// forward declaration

template< class char_type, class traits_type = std::char_traits< char_type> >
    class basic_thread_safe_log;

template< class char_type, class traits_type = std::char_traits< char_type> >
    class basic_internal_thread_safe_log
{
    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    friend class basic_thread_safe_log< char_type, traits_type>;

    // non-copyiable
    typedef basic_internal_thread_safe_log< char_type, traits_type> this_class;
    basic_internal_thread_safe_log( const this_class &);
    this_class & operator=( this_class &);

public:
    basic_internal_thread_safe_log( ostream_type & underlyingLog)
    :           m_underlyingLog( underlyingLog), m_writer( underlyingLog)

    {}

    ~basic_internal_thread_safe_log()

    {}

    void write_message( const std::basic_string< char_type, traits_type> & str)
    { m_writer.add_message( str); }

    void copy_state_to( ostream_type & dest) const
    {
        pthread_mutex_lock(m_writer.cs());
       dest.copyfmt( m_underlyingLog);
        dest.setstate( m_underlyingLog.rdstate());
        pthread_mutex_unlock(m_writer.cs());
    }

    void copy_state_from( const ostream_type & src)
    {
        pthread_mutex_lock(m_writer.cs());
        m_underlyingLog.copyfmt( src);
        m_underlyingLog.setstate( m_underlyingLog.rdstate());
        pthread_mutex_unlock(m_writer.cs());    
    }

private:
    ostream_type & m_underlyingLog;
    thread_safe_log_writer< char_type, traits_type> m_writer;

};

typedef basic_internal_thread_safe_log< char> internal_thread_safe_log;
typedef basic_internal_thread_safe_log< wchar_t> winternal_thread_safe_log;

template< class char_type, class traits_type>
    class basic_thread_safe_log

    // *** protected, not public !!!
    : protected basic_message_handler_log< char_type, traits_type>
{

    typedef std::basic_ostream< char_type, traits_type> ostream_type;
    typedef basic_internal_thread_safe_log< char_type, traits_type> internal_type;

public:
    basic_thread_safe_log( internal_type & tsLog)
        : m_tsLog( tsLog)
    {
        // get underlying stream state
        tsLog.copy_state_to( ts() );
    }

    basic_thread_safe_log( const basic_thread_safe_log< char_type, traits_type> & from)
        : std::basic_ios<char_type, traits_type>(), basic_message_handler_log< char_type, traits_type>(), m_tsLog( from.m_tsLog)

          // ... on some platforms, a std::ostream base copy-constructor
          //    might be defined as private...

    {
        // get underlying stream state
        m_tsLog.copy_state_to( ts() );
    }

    ~basic_thread_safe_log()
    {
        // copy state to underlying stream
        m_tsLog.copy_state_from( ts() );
    }

    // get base class - to which we can write
    ostream_type & ts() { return *this; }

protected:
    virtual void on_new_message( const std::string & str)
    { m_tsLog.write_message( str); }

private:
    internal_type  & m_tsLog;
};

typedef basic_thread_safe_log< char> thread_safe_log;
typedef basic_thread_safe_log< wchar_t> wthread_safe_log;

inline thread_safe_log get_thread_safe_log(){
#if MAX_PTHREADS > 1
    static std::ofstream out( "out.txt");
//    static std::ostringstream out(ostringstream::out);   
    static internal_thread_safe_log log( out);
    return thread_safe_log(log);
#else
std::cerr << "trying to use wrong log scheme" << std::endl;
exit(1);
#endif
}



#endif
