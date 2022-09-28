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

#include <ortc/services/ITransportStream.h>
#include <ortc/services/internal/types.h>

#include <list>
#include <map>

namespace ortc
{
  namespace services
  {
    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark TransportStream
      #pragma mark

      class TransportStream : public Noop,
                              public zsLib::MessageQueueAssociator,
                              public ITransportStream,
                              public ITransportStreamWriter,
                              public ITransportStreamReader
      {
      protected:
        struct make_private {};

      public:
        friend interaction ITransportStreamFactory;
        friend interaction ITransportStream;

        typedef ITransportStream::StreamHeader StreamHeader;
        typedef ITransportStream::StreamHeaderPtr StreamHeaderPtr;
        typedef ITransportStream::StreamHeaderWeakPtr StreamHeaderWeakPtr;
        typedef ITransportStream::Endians Endians;

        struct Buffer
        {
          Buffer() : mRead(0) {}

          SecureByteBlockPtr mBuffer;
          size_t mRead;
          StreamHeaderPtr mHeader;
        };

        typedef std::list<Buffer> BufferList;

      public:
        TransportStream(
                        const make_private &,
                        IMessageQueuePtr queue,
                        ITransportStreamWriterDelegatePtr writerDelegate = ITransportStreamWriterDelegatePtr(),
                        ITransportStreamReaderDelegatePtr readerDelegate = ITransportStreamReaderDelegatePtr()
                        );

      protected:
        TransportStream(Noop) :
          Noop(true),
          zsLib::MessageQueueAssociator(IMessageQueuePtr()) {}

        void init();

      public:
        ~TransportStream();

        static TransportStreamPtr convert(ITransportStreamPtr stream);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark TransportStream => ITransportStream
        #pragma mark

        static ElementPtr toDebug(ITransportStreamPtr stream);

        static TransportStreamPtr create(
                                         ITransportStreamWriterDelegatePtr writerDelegate = ITransportStreamWriterDelegatePtr(),
                                         ITransportStreamReaderDelegatePtr readerDelegate = ITransportStreamReaderDelegatePtr()
                                         );

        virtual PUID getID() const {return mID;}

        virtual ITransportStreamWriterPtr getWriter() const;
        virtual ITransportStreamReaderPtr getReader() const;

        virtual void cancel();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark TransportStream => ITransportStreamWriter
        #pragma mark

        // (duplicate) virtual PUID getID() const;

        virtual ITransportStreamWriterSubscriptionPtr subscribe(ITransportStreamWriterDelegatePtr delegate);

        virtual ITransportStreamPtr getStream() const;

        virtual bool isWriterReady() const;

        virtual void write(
                           const BYTE *buffer,
                           size_t bufferLengthInBytes,
                           StreamHeaderPtr header = StreamHeaderPtr()   // not always needed
                           );

        virtual void write(
                           SecureByteBlockPtr bufferToAdopt,
                           StreamHeaderPtr header = StreamHeaderPtr()   // not always needed
                           );

        virtual void write(
                           WORD value,
                           StreamHeaderPtr header = StreamHeaderPtr(),  // not always needed
                           Endians endian = ITransportStream::Endian_Big
                           );

        virtual void write(
                           DWORD value,
                           StreamHeaderPtr header = StreamHeaderPtr(),  // not always needed
                           Endians endian = ITransportStream::Endian_Big
                           );

        virtual void block(bool block = true);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark TransportStream => ITransportStreamReader
        #pragma mark

        // (duplicate) virtual PUID getID() const;

        virtual ITransportStreamReaderSubscriptionPtr subscribe(ITransportStreamReaderDelegatePtr delegate);

        // (duplicate) virtual ITransportStreamPtr getStream() const;

        virtual void notifyReaderReadyToRead();

        virtual size_t getNextReadSizeInBytes() const;

        virtual StreamHeaderPtr getNextReadHeader() const;

        virtual size_t getTotalReadBuffersAvailable() const;

        virtual size_t getTotalReadSizeAvailableInBytes() const;

        virtual size_t read(
                            BYTE *outBuffer,
                            size_t bufferLengthInBytes,
                            StreamHeaderPtr *outHeader = NULL
                            );

        virtual SecureByteBlockPtr read(StreamHeaderPtr *outHeader = NULL);

        virtual size_t readWORD(
                                WORD &outResult,
                                StreamHeaderPtr *outHeader = NULL,
                                Endians endian = ITransportStream::Endian_Big
                                );

        virtual size_t readDWORD(
                                 DWORD &outResult,
                                 StreamHeaderPtr *outHeader = NULL,
                                 Endians endian = ITransportStream::Endian_Big
                                 );

        virtual size_t peek(
                            BYTE *outBuffer,
                            size_t bufferLengthInBytes,
                            StreamHeaderPtr *outHeader = NULL,
                            size_t offsetInBytes = 0
                            );

        virtual SecureByteBlockPtr peek(
                                        size_t bufferLengthInBytes = 0,
                                        StreamHeaderPtr *outHeader = NULL,
                                        size_t offsetInBytes = 0
                                        );

        virtual size_t peekWORD(
                                WORD &outResult,
                                StreamHeaderPtr *outHeader = NULL,
                                size_t offsetInBytes = 0,
                                Endians endian = ITransportStream::Endian_Big
                                );

        virtual size_t peekDWORD(
                                 DWORD &outResult,
                                 StreamHeaderPtr *outHeader = NULL,
                                 size_t offsetInBytes = 0,
                                 Endians endian = ITransportStream::Endian_Big
                                 );

        virtual size_t skip(size_t offsetInBytes);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark StreamTransport => (internal)
        #pragma mark

        RecursiveLock &getLock() const;
        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;

        virtual ElementPtr toDebug() const;

        bool isShutdown() const {return mShutdown;}

        void notifySubscribers(
                               bool afterRead,
                               bool afterWrite
                               );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark StreamTransport => (data)
        #pragma mark

        AutoPUID mID;
        mutable RecursiveLock mLock;
        TransportStreamWeakPtr mThisWeak;

        bool mShutdown {};
        bool mReaderReady {};

        bool mReadReadyNotified {};
        bool mWriteReadyNotified {};

        ITransportStreamWriterDelegateSubscriptions mWriterSubscriptions;
        ITransportStreamWriterSubscriptionPtr mDefaultWriterSubscription;

        ITransportStreamReaderDelegateSubscriptions mReaderSubscriptions;
        ITransportStreamReaderSubscriptionPtr mDefaultReaderSubscription;

        BufferList mBuffers;

        ByteQueuePtr mBlockQueue;
        StreamHeaderPtr mBlockHeader;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ITransportStreamFactor
      #pragma mark

      interaction ITransportStreamFactory
      {
        static ITransportStreamFactory &singleton();

        virtual TransportStreamPtr create(
                                          ITransportStreamWriterDelegatePtr writerDelegate = ITransportStreamWriterDelegatePtr(),
                                          ITransportStreamReaderDelegatePtr readerDelegate = ITransportStreamReaderDelegatePtr()
                                          );
      };

      class TransportStreamFactory : public IFactory<ITransportStreamFactory> {};
    }
  }
}
