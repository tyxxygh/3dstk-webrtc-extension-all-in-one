/*

 Copyright (c) 2014, Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#pragma once

#include <ortc/services/types.h>

namespace ortc
{
  namespace services
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStream
    #pragma mark

    interaction ITransportStream
    {
      ZS_DECLARE_INTERACTION_PTR(StreamHeader)

      interaction StreamHeader
      {
        virtual ~StreamHeader() {}  // needs virtual method to ensure dynamic casting works
      };

      enum Endians
      {
        Endian_Big = true,
        Endian_Little = false,
      };

      static const char *toString(Endians endian);

      static ElementPtr toDebug(ITransportStreamPtr stream);

      static ITransportStreamPtr create(
                                        ITransportStreamWriterDelegatePtr writerDelegate = ITransportStreamWriterDelegatePtr(),
                                        ITransportStreamReaderDelegatePtr readerDelegate = ITransportStreamReaderDelegatePtr()
                                        );

      virtual PUID getID() const = 0;

      virtual ITransportStreamWriterPtr getWriter() const = 0;
      virtual ITransportStreamReaderPtr getReader() const = 0;

      virtual void cancel() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStreamWriter
    #pragma mark

    interaction ITransportStreamWriter
    {
      typedef ITransportStream::StreamHeader StreamHeader;
      typedef ITransportStream::StreamHeaderPtr StreamHeaderPtr;
      typedef ITransportStream::StreamHeaderWeakPtr StreamHeaderWeakPtr;
      typedef ITransportStream::Endians Endians;

      virtual PUID getID() const = 0; // returns the same ID as the stream

      //-----------------------------------------------------------------------
      // PURPOSE: Subscribe to receive events when the write buffer is empty
      //          and available for more data to be written
      virtual ITransportStreamWriterSubscriptionPtr subscribe(ITransportStreamWriterDelegatePtr delegate) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: obtain the associated transport stream
      virtual ITransportStreamPtr getStream() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: check if the writer has been informed the reader is ready
      virtual bool isWriterReady() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: clears out all data pending in the stream and prevents any
      //          more data being read/written
      virtual void cancel() = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Write buffer into the stream
      virtual void write(
                         const BYTE *buffer,
                         size_t bufferLengthInBytes,
                         StreamHeaderPtr header = StreamHeaderPtr()   // not always needed
                         ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Write buffer into the stream
      virtual void write(
                         SecureByteBlockPtr bufferToAdopt,
                         StreamHeaderPtr header = StreamHeaderPtr()   // not always needed
                         ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Write a WORD value into the stream
      virtual void write(
                         WORD value,
                         StreamHeaderPtr header = StreamHeaderPtr(),  // not always needed
                         Endians endian = ITransportStream::Endian_Big
                         ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Write a WORD value into the stream
      virtual void write(
                         DWORD value,
                         StreamHeaderPtr header = StreamHeaderPtr(),  // not always needed
                         Endians endian = ITransportStream::Endian_Big
                         ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Cause the send stream to backlog all the data being written
      //          and not allow it to be notified to the reader (until the
      //          block is removed).
      // NOTE:    When blocking, the blocking mode "header" will be the first
      //          header written while blocking and all other headers will
      //          be ignored. When the block is removed, all blocked data
      //          will be written as "one large buffer" combining all data
      //          written into a single written buffer.
      virtual void block(bool block = true) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStreamWriterDelegate
    #pragma mark

    interaction ITransportStreamWriterDelegate
    {
      //-----------------------------------------------------------------------
      // PURPOSE: Notification that the writer's data buffers are empty thus
      //          more data can be written at this time (without causing an
      //          overflow of data).
      virtual void onTransportStreamWriterReady(ITransportStreamWriterPtr writer) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStreamWriterSubscription
    #pragma mark

    interaction ITransportStreamWriterSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStreamReader
    #pragma mark

    interaction ITransportStreamReader
    {
      typedef ITransportStream::StreamHeader StreamHeader;
      typedef ITransportStream::StreamHeaderPtr StreamHeaderPtr;
      typedef ITransportStream::StreamHeaderWeakPtr StreamHeaderWeakPtr;
      typedef ITransportStream::Endians Endians;

      virtual PUID getID() const = 0; // returns the same ID as the stream

      //-----------------------------------------------------------------------
      // PURPOSE: Subscribe to receive events when read data is available
      virtual ITransportStreamReaderSubscriptionPtr subscribe(ITransportStreamReaderDelegatePtr delegate) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: obtain the associated transport stream
      virtual ITransportStreamPtr getStream() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: clears out all data pending in the stream and prevents any
      //          more data being read/written
      virtual void cancel() = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: call to notify the writer that it can start sending data
      //          to the reader.
      // NOTE:    By notifying that the writer that the reader is ready to
      //          read, the writer will get notified of "writer ready" for
      //          the first time.
      //
      //          The reader will not receive "on reader ready" untl this
      //          method is called (but still can choose to read regardless. If
      //          data is read then the "notifyReaderReadyToRead" is presumed
      //          to be called and read/write ready notifications will fire
      //          for the reader and the writer.
      //
      //          The writer can choose to send data before receiving this
      //          ready notification in which case the read data will be
      //          cached until the reader decided to reads the data.
      virtual void notifyReaderReadyToRead() = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Obtains the size of the next "written" buffer available to
      //          be read.
      // NOTE:    This buffer will match the next FIFO size buffer written to
      //          the ITransportWriter (and not the total amount written thus
      //          far).
      //
      //          A "0" sized buffer could have been written to the "write"
      //          stream thus this is not a reliable method to determine
      //          if there are any buffers available or not for reading.
      //          Instead use "getTotalReadBuffersAvailable" to determine if
      //          buffers are available for reading.
      virtual size_t getNextReadSizeInBytes() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Obtains the header for the next FIFO buffer written
      // NOTE:    Will return StreamHeaderPtr() if no header is available (or
      //          no buffer is available).
      virtual StreamHeaderPtr getNextReadHeader() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Obtains the total number of the buffers "written" to the
      //          FIFO write stream that are still available to read.
      virtual size_t getTotalReadBuffersAvailable() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Obtains the size of all data "written" buffer available to
      //          be read.
      // NOTE:    This buffer will match the sum of all FIFO buffered data
      //          written to the ITransportWriter (and not any individual
      //          written buffer).
      virtual size_t getTotalReadSizeAvailableInBytes() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Reads buffered data written to the stream.
      // NOTE:    If the buffer size is equal to the next FIFO buffer written
      //          this method will read the entire written buffer at once
      //          and the header value will point to that buffer's header.
      //          If the read size is less than the next written FIFO buffer,
      //          this method will partially read part of the next FIFO buffer
      //          and return the written header of that buffer. If the read
      //          size is greater than the next FIFO buffer, it will read
      //          beyond the next available FIFO buffer but return the header
      //          of only the first available FIFO buffer.
      virtual size_t read(
                          BYTE *outBuffer,
                          size_t bufferLengthInBytes,
                          StreamHeaderPtr *outHeader = NULL
                          ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Reads buffered data written to the write buffer with exactly
      //          the size written.
      virtual SecureByteBlockPtr read(
                                      StreamHeaderPtr *outHeader = NULL
                                      ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Reads a WORD of buffered data written to the stream.
      // NOTE:    returns sizeof(WORD) if successful
      virtual size_t readWORD(
                              WORD &outResult,
                              StreamHeaderPtr *outHeader = NULL,
                              Endians endian = ITransportStream::Endian_Big
                              ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Reads buffered data written to the stream.
      // NOTE:    returns sizeof(DWORD) if successful
      virtual size_t readDWORD(
                               DWORD &outResult,
                               StreamHeaderPtr *outHeader = NULL,
                               Endians endian = ITransportStream::Endian_Big
                               ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Peeks ahead at the buffered data written to the stream
      //          but still allows it to be read later.
      virtual size_t peek(
                          BYTE *outBuffer,
                          size_t bufferLengthInBytes,
                          StreamHeaderPtr *outHeader = NULL,
                          size_t offsetInBytes = 0
                          ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Peeks ahead at the buffered data written to the stream
      //          but still allows it to be read later.
      virtual SecureByteBlockPtr peek(
                                      size_t bufferLengthInBytes = 0,
                                      StreamHeaderPtr *outHeader = NULL,
                                      size_t offsetInBytes = 0
                                      ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Peeks a WORD of buffered data written to the stream.
      // NOTE:    returns sizeof(WORD) if successful
      virtual size_t peekWORD(
                              WORD &outResult,
                              StreamHeaderPtr *outHeader = NULL,
                              size_t offsetInBytes = 0,
                              Endians endian = ITransportStream::Endian_Big
                              ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Peeks buffered data written to the stream.
      // NOTE:    returns sizeof(DWORD) if successful
      virtual size_t peekDWORD(
                               DWORD &outResult,
                               StreamHeaderPtr *outHeader = NULL,
                               size_t offsetInBytes = 0,
                               Endians endian = ITransportStream::Endian_Big
                               ) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Flushes the FIFO by the data offset specified.
      virtual size_t skip(size_t offsetInBytes) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportStreamReaderDelegate
    #pragma mark

    interaction ITransportStreamReaderDelegate
    {
      //-----------------------------------------------------------------------
      // PURPOSE: Notification every time more data was either written to the
      //          stream or after ever read where data is still present.
      virtual void onTransportStreamReaderReady(ITransportStreamReaderPtr reader) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportReaderSubscription
    #pragma mark

    interaction ITransportStreamReaderSubscription
    {
      virtual PUID getID() const = 0;

      virtual void cancel() = 0;

      virtual void background() = 0;
    };
    
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::services::ITransportStreamWriterDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::ITransportStreamWriterPtr, ITransportStreamWriterPtr)
ZS_DECLARE_PROXY_METHOD_1(onTransportStreamWriterReady, ITransportStreamWriterPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::services::ITransportStreamWriterDelegate, ortc::services::ITransportStreamWriterSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::services::ITransportStreamWriterPtr, ITransportStreamWriterPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onTransportStreamWriterReady, ITransportStreamWriterPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

ZS_DECLARE_PROXY_BEGIN(ortc::services::ITransportStreamReaderDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::ITransportStreamReaderPtr, ITransportStreamReaderPtr)
ZS_DECLARE_PROXY_METHOD_1(onTransportStreamReaderReady, ITransportStreamReaderPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::services::ITransportStreamReaderDelegate, ortc::services::ITransportStreamReaderSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::services::ITransportStreamReaderPtr, ITransportStreamReaderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onTransportStreamReaderReady, ITransportStreamReaderPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
