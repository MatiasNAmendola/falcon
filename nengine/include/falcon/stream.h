/*
   FALCON - The Falcon Programming Language.
   FILE: stream.h

   Base class for I/O operations.
   -------------------------------------------------------------------
   Author: Giancarlo Niccolai
   Begin: Sat, 12 Mar 2011 13:00:57 +0100

   -------------------------------------------------------------------
   (C) Copyright 2011: the FALCON developers (see list in AUTHORS file)

   See LICENSE file for licensing details.
*/

/** \file
   The Falcon Stream API.

   Falcon Streams are the basic I/O of every Falcon subsystem, including
   VM, compiler, assembler, generators and modules.
*/

#ifndef _FALCON_STREAM_H_
#define _FALCON_STREAM_H_

#include <falcon/setup.h>
#include <falcon/types.h>

#include <falcon/refpointer.h>
#include <falcon/interrupt.h>

namespace Falcon {


/** Base class for file and filelike services.

   This class is used by all the I/O in Falcon libraries and modules.

   Subclassess to store or read data from standard streams, files and
   memory buffers are already provided; the implementors may extend
   this class or the derived classes to support more systems and/or
   special I/O devices.

   This is a purely abstract class that serves as a base class for
   system-specific or system independent implementations.
*/
class FALCON_DYN_CLASS Stream
{
public:

   /** Enumeration representing the current status of the stream. */
   typedef enum {
      /** Stream is unopened and unready. */
      t_none = 0,
      /** The stream has been opened. */
      t_open = 0x1,
      /** A read operation hit the end of file. */
      t_eof = 0x2,
      /** An I/O error condition has been met during a read, write, flush, open or close operation. */
      t_error = 0x4,
      /** Last required operation is unsupported on this stream. */
      t_unsupported = 0x8,
      /** The stream has been invalidated. */
      t_invalid = 0x10,
      /** Last I/O operation has been interrupted. */
      t_interrupted = 0x20
   } t_status ;

   /** Enumeration representing the current status of the stream. */
      typedef enum {
      ew_begin,
      ew_cur,
      ew_end
   } e_whence;

   /** Copy constructor.
    The base class copies the status and the value of the last error.
   */
   Stream( const Stream &other );
   virtual ~Stream();

   /** Returns the current stream status.
    The status may be one of the t_status enumeration,
    or a bit combination.

    The following functions should be used to determine the current
    stream status.
   */
   virtual t_status status() const { return m_status; }

   /** Changes the status of the stream.

    It's suggested to use just reset() to alter the status of the stream.
    */
   virtual void status( t_status s ) { m_status = s; }

   /** Resets the stream status after an error, or after eof has been hit. */
   virtual inline void reset();
   /** Returns true if the stream is open and not in error state or eof. */
   inline bool good() const;
   /** Returns true if the stream is in error state or at eof (ignoring open state). */
   inline bool bad() const;
   /** Returns true if the stream refers to an open resource. */
   inline bool open() const;
   /** Returns true if the caller tried to read past of end-of-file. */
   inline bool eof() const;
   /** Returns true if the caller tried to perform an unsupported operation.
    For example, a write  on a read-only stream.
    Please, notice that unsupported operations always throw a Falcon::IOError.
   */
   inline bool unsupported() const;

   /** Returns true if this stream has a permanent error and is considered invalid. */
   inline bool invalid() const;

   /** Returns true if this stream has met an I/O Error.
    In this case, the system error ID can be retreived through lastError().
   */
   inline bool error() const;

   /** Returns true if this stream has been interrupted during a blocking operation. */
   inline bool interrupted() const;


   /** Reads from target stream.

    At end of streams, returns 0 and sets the eof() status. If trying to read
    when the eof() status is set, it might throw an error.

    The returned read size may be smaller than the required size if there aren't
    enough bytes available on the stream. If there isn't any data currently
    available, the call blocks (should respect VM interruption protocol, if
    possible and efficient, but this is not required).

    Use readAvailable() to determine if the next read on this stream might block.
    
      \param buffer the buffer where read data will be stored.
      \param size the amount of bytes to read.
      \return Count of read data; 0 on stream end, -1 on error.
   */
   virtual size_t read( void *buffer, size_t size )=0;

   /** Write to the target stream.

     \param buffer the buffer where the output data is stored.
     \param size the maximum amount of bytes to write.
    */
   virtual size_t write( const void *buffer, size_t size )=0;

   /** Close target stream. */
   virtual bool close() = 0;
   
   /** Returns the current position in a file. */
   virtual int64 tell() = 0;

   /** Truncates the stream at a given position, or at current position if pos < 0 */
   virtual bool truncate( off_t pos=-1 ) = 0;

   /** Determines if the stream can be read, possibly with a given timeout.

    \param msecs_timeout Wait for available data for no more than the required milliseconds.
    \param intr An interrupter that might be remotely excited to stop the wait.
    \return 0 if no data is available, -1 on error, a positive value (possibly an hint of how
    much data can be read) on success.
   */
   virtual size_t readAvailable( int32 msecs_timeout=0 ) = 0;

   /** Determines if the stream can be written, possibly with a given timeout.

    \param msecs_timeout Wait for available data for no more than the required milliseconds.
    \param intr An interrupter that might be remotely excited to stop the wait.
    \return 0 if no data can be written, -1 on error, a positive value (possibly an hint of how
    much data can be written) on success.
    */
   virtual size_t writeAvailable( int32 msecs_timeout=0 ) = 0;

   /** Seks from the beginning of a file. */
   inline off_t seekBegin( off_t pos ) { return seek( pos, ew_begin ); }

   /** Seks from the curent position of a file. */
   inline off_t seekCurrent( off_t pos ) { return seek( pos, ew_cur ); }

   /** Seks from the end of a file. */
   inline off_t seekEnd( off_t pos ) { return seek( pos, ew_end ); }

   /** Seks from a given position in a file. */
   virtual off_t seek( off_t pos, e_whence w ) = 0;

   /** Returns the system error ID from the last I/O operation. */
   virtual size_t lastError() const { return m_lastError; }

   /** Clones the stream.
    The clone semantic is the same as the unix dup() operation; it should create
    a new stream referring to the same underlying resource.
    
    If the stream cannot be cloned, an unsupported error should be thrown.
   */
   virtual Stream *clone() const = 0;

   /** Return true if this stream is required to throw an IOError. */
   inline bool shouldThrow() const { return m_bShouldThrow; }

   /** Changes the status of throw-on-error operations. */
   inline void shouldThrow( bool bMode ) { m_bShouldThrow = bMode; }

   /** Utility to throw an unsupported error when an operation is unsupported. */
   void throwUnsupported();

   inline void setInterrupter( const ref_ptr<Interrupt> &ptr )
   {
      m_ptrIntr = ptr;
   }

protected:
   t_status m_status;
   size_t m_lastError;
   bool m_bShouldThrow;

   ref_ptr<Interrupt> m_ptrIntr;

   /** Initializes the base stream class. */
   Stream();

   friend class Transcoder;
};


/** Or operator on status bitfiled.
   This is to allow integer oinline processing on enum fields in Stream class.
*/
inline Stream::t_status operator|( const Stream::t_status &elem1, const Stream::t_status &elem2)
{
   return static_cast<Stream::t_status>(
      static_cast<unsigned int>(elem1) | static_cast<unsigned int>(elem2) );
}

/** And operator on status bitfiled.
   This is to allow integer oinline processing on enum fields in Stream class.
*/
inline Stream::t_status operator&( const Stream::t_status &elem1, const Stream::t_status &elem2)
{
   return static_cast<Stream::t_status>(
      static_cast<unsigned int>(elem1) & static_cast<unsigned int>(elem2) );
}

/** Xor operator on status bitfiled.
   This is to allow integer oinline processing on enum fields in Stream class.
*/

inline Stream::t_status operator^( const Stream::t_status &elem1, const Stream::t_status &elem2)
{
   return static_cast<Stream::t_status>(
      static_cast<unsigned int>(elem1) ^ static_cast<unsigned int>(elem2) );
}

/** Not operator on status bitfiled.
   This is to allow integer oinline processing on enum fields in Stream class.
*/

inline Stream::t_status operator~( const Stream::t_status &elem1 )
{
   return static_cast<Stream::t_status>( ~ static_cast<unsigned int>(elem1) );
}

inline void Stream::reset()
{
   status( status() &
         static_cast<t_status>(~static_cast<unsigned int>(t_error|t_unsupported|t_invalid)) );
   m_lastError = 0;
}

inline bool Stream::good() const
      { return (status() &( t_error | t_unsupported | t_invalid | t_eof | t_open )) == t_open; }
inline bool Stream::bad() const
      { return (status() &( t_error | t_unsupported | t_invalid | t_eof )) != 0; }

inline bool Stream::open() const
   { return (status() & t_open ) != 0; }
inline bool Stream::eof() const
   { return (status() & t_eof ) != 0; }
inline bool Stream::unsupported() const
   { return (status() & t_unsupported ) != 0; }
inline bool Stream::invalid() const
   { return (status() & t_invalid ) != 0; }
inline bool Stream::error() const
   { return ( status() & t_error ) != 0; }
inline bool Stream::interrupted() const
   { return ( status() & t_interrupted ) != 0; }

} //end of Falcon namespace

#endif

/* end of stream.h */