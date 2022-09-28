/*

 Copyright (c) 2014, Robin Raymond
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

#include <zsLib/Log.h>
#include <zsLib/eventing/Log.h>
#include <zsLib/helpers.h>
#include <zsLib/Exception.h>
#include <zsLib/XML.h>
#include <zsLib/Numeric.h>
#include <zsLib/Singleton.h>
#include <zsLib/String.h>

#ifdef _WIN32
#include <Evntprov.h>
#include <WinBase.h>
#else
#include <pthread.h>
#endif //_WIN32

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark Subsystem
  #pragma mark

  //---------------------------------------------------------------------------
  Subsystem::Subsystem(
                       CSTR inName,
                       Log::Level inOutputLevel,
                       Log::Level inEventingLevel
                       ) :
    mSubsystem(inName),
    mOutputLevel(static_cast<Subsystem::LevelType>(inOutputLevel)),
    mEventingLevel(static_cast<Subsystem::LevelType>(inEventingLevel))
  {
  }

  //---------------------------------------------------------------------------
  void Subsystem::notifyNewSubsystem()
  {
    Log::notifyNewSubsystem(this);
  }

  //---------------------------------------------------------------------------
  void Subsystem::setOutputLevel(Log::Level inLevel)
  {
    mOutputLevel = inLevel;
  }

  //---------------------------------------------------------------------------
  Log::Level Subsystem::getOutputLevel() const
  {
    return mOutputLevel;
  }

  //---------------------------------------------------------------------------
  void Subsystem::setEventingLevel(Log::Level inLevel)
  {
    mEventingLevel = inLevel;
  }

  //---------------------------------------------------------------------------
  Log::Level Subsystem::getEventingLevel() const
  {
    return mEventingLevel;
  }

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Log
    #pragma mark


    //-------------------------------------------------------------------------
    Log::EventingWriter::EventingWriter()
    {      
    }
    
    //-------------------------------------------------------------------------
    Log::EventingWriter::~EventingWriter()
    {
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Log
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Log(const make_private &) :
        mOutputListeners(make_shared<OutputListenerList>()),
        mEventingListeners(make_shared<EventingListenerList>()),
        mEventingProviderListeners(make_shared<EventingProviderListenerList>())
    {
    }

    //-------------------------------------------------------------------------
    Log::~Log()
    {      
    }

    //-------------------------------------------------------------------------
    void Log::initSingleton()
    {
      zsLib::Log::singleton();
    }

    //-------------------------------------------------------------------------
    String Log::paramize(const char *name)
    {
      if (!name) return String();

      auto orignalLength = strlen(name);
      auto maxLength = orignalLength * 2;

      char buffer[256] {};
      char *useBuffer = &(buffer[0]);

      char *allocatedBuffer = NULL;

      if (maxLength >= sizeof(buffer)) {
        allocatedBuffer = new char[maxLength+1] {};
        useBuffer = allocatedBuffer;
      }

      bool lastWasSpace = true;
      bool lastWasUpper = true;

      const char *source = name;
      char *dest = useBuffer;

      while (*source) {
        char gliph = *source;

        if (!isalnum(gliph)) {
          if (lastWasSpace) {
            ++source;
            continue;
          }
          *dest = ' ';
          ++dest;
          ++source;

          lastWasSpace = true;
          continue;
        }

        if (isupper(gliph)) {
          if (!lastWasUpper) {
            if (!lastWasSpace) {
              *dest = ' ';
              ++dest;
            }
          }

          *dest = tolower(gliph);
          ++dest;
          ++source;

          lastWasSpace = false;
          lastWasUpper = true;
          continue;
        }

        lastWasUpper = false;
        lastWasSpace = false;

        *dest = gliph;
        ++dest;
        ++source;
      }

      String result((const char *)useBuffer);

      delete [] allocatedBuffer;
      allocatedBuffer = NULL;

      return result;
    }
  }

  //---------------------------------------------------------------------------
  const char *Log::toString(Severity severity) noexcept
  {
    switch (severity) {
      case Informational: return "Informational";
      case Warning:       return "Warning";
      case Error:         return "Error";
      case Fatal:         return "Fatal";
    }
    return "undefined";
  }

  //---------------------------------------------------------------------------
  Log::Severity Log::toSeverity(const char *inSeverityStr) noexcept(false)
  {
    String severityStr(inSeverityStr);

    if (0 == (severityStr.compareNoCase("Info"))) {
      severityStr = "Informational";
    }

    for (Log::Severity index = Log::Severity_First; index <= Log::Severity_Last; index = static_cast<Log::Severity>(static_cast<std::underlying_type<Log::Severity>::type>(index) + 1)) {
      if (0 == severityStr.compareNoCase(toString(index))) {
        return index;
      }
    }

    ZS_THROW_INVALID_ARGUMENT(String("Invalid severity: ") + severityStr);
    return Informational;
  }
  
  //---------------------------------------------------------------------------
  const char *Log::toString(Level level) noexcept
  {
    switch (level) {
      case None:          return "None";
      case Basic:         return "Basic";
      case Detail:        return "Detail";
      case Debug:         return "Debug";
      case Trace:         return "Trace";
      case Insane:        return "Insane";
    }
    return "undefined";
  }

  //---------------------------------------------------------------------------
  Log::Level Log::toLevel(const char *inLevelStr) noexcept(false)
  {
    String levelStr(inLevelStr);

    for (Log::Level index = Log::Level_First; index <= Log::Level_Last; index = static_cast<Log::Level>(static_cast<std::underlying_type<Log::Level>::type>(index) + 1)) {
      if (0 == levelStr.compareNoCase(toString(index))) {
        return index;
      }
    }

    ZS_THROW_INVALID_ARGUMENT(String("Invalid level: ") + levelStr);
    return None;
  }

  //---------------------------------------------------------------------------
  Log::Log(const make_private &ignore) :
    internal::Log(ignore)
  {
  }

  //---------------------------------------------------------------------------
  Log::~Log()
  {
    for (auto iter = mSubsystems.begin(); iter != mSubsystems.end(); ++iter)
    {
      auto *subsystem = (*iter);
      subsystem->setEventingLevel(None);
    }

    for (auto iter = mCleanUpWriters.begin(); iter != mCleanUpWriters.end(); ++iter)
    {
      auto *writer = (*iter);
      writer->mInitValue = 0;
      delete writer;
    }
  }

  //---------------------------------------------------------------------------
  LogPtr Log::singleton()
  {
    static SingletonLazySharedPtr<Log> singleton(create());
    return singleton.singleton();
  }

  //---------------------------------------------------------------------------
  LogPtr Log::create()
  {
    return make_shared<Log>(make_private{});
  }

  //---------------------------------------------------------------------------
  QWORD Log::getCurrentTimestampMS() noexcept
  {
    return (QWORD)(toMilliseconds(zsLib::now()).count());
  }

  //---------------------------------------------------------------------------
  QWORD Log::getCurrentThreadID() noexcept
  {
#ifdef _WIN32
    return (GetCurrentThreadId());
#else
#ifdef __APPLE__
    return (QWORD)(((PTRNUMBER)pthread_mach_thread_np(pthread_self())));
#else
    return (QWORD)(((PTRNUMBER)pthread_self()));
#endif //APPLE
#endif //_WIN32
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (output methods)
  #pragma mark

  //---------------------------------------------------------------------------
  void Log::addOutputListener(ILogOutputDelegatePtr delegate)
  {
    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    SubsystemList notifyList;
    // copy the list but notify without the lock
    {
      AutoRecursiveLock lock(refThis.mLock);

      OutputListenerListPtr replaceList(make_shared<OutputListenerList>());

      size_t originalSize = refThis.mOutputListeners->size();

      (*replaceList) = (*refThis.mOutputListeners);

      replaceList->push_back(delegate);

      refThis.mOutputListeners = replaceList;

      notifyList = refThis.mSubsystems;

      if (0 == originalSize) {
        Optional<Level> defaultLevel;

        {
          auto found = refThis.mDefaultOutputSubsystemLevels.find(String());
          if (found != refThis.mDefaultOutputSubsystemLevels.end()) defaultLevel = static_cast<Level>((*found).second);
        }

        for (auto iter = refThis.mSubsystems.begin(); iter != refThis.mSubsystems.end(); ++iter)
        {
          auto *subsystem = (*iter);
          auto *name = subsystem->getName();
          auto found = refThis.mDefaultOutputSubsystemLevels.find(String(name));
          if (found != refThis.mDefaultOutputSubsystemLevels.end()) {
            auto level = static_cast<Level>((*found).second);
            subsystem->setOutputLevel(level);
          } else if (defaultLevel.hasValue()) {
            subsystem->setOutputLevel(defaultLevel.value());
          }
        }
      }
    }

    for (SubsystemList::iterator iter = notifyList.begin(); iter != notifyList.end(); ++iter)
    {
      delegate->notifyNewSubsystem(*(*iter));
    }
  }

  //---------------------------------------------------------------------------
  void Log::removeOutputListener(ILogOutputDelegatePtr delegate)
  {
    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    AutoRecursiveLock lock(refThis.mLock);

    OutputListenerListPtr replaceList(make_shared<OutputListenerList>());

    (*replaceList) = (*refThis.mOutputListeners);

    for (OutputListenerList::iterator iter = replaceList->begin(); iter != replaceList->end(); ++iter)
    {
      if (delegate.get() == (*iter).get())
      {
        replaceList->erase(iter);

        if (0 == replaceList->size()) {
          if (refThis.mDefaultOutputSubsystemLevels.size() > 0) {
            for (auto innerIter = refThis.mSubsystems.begin(); innerIter != refThis.mSubsystems.end(); ++innerIter) {
              auto *subsystem = (*innerIter);
              subsystem->setOutputLevel(None);
            }
          }
        }

        refThis.mOutputListeners = replaceList;
        return;
      }
    }

    ZS_THROW_INVALID_ARGUMENT("cound not remove log listener as it was not found");
  }

  //---------------------------------------------------------------------------
  void Log::notifyNewSubsystem(Subsystem *inSubsystem)
  {
    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    OutputListenerListPtr notify1List;
    EventingListenerListPtr notify2List;
    EventingProviderListenerListPtr notify3List;

    // scope: remember the subsystem
    {
      AutoRecursiveLock lock(refThis.mLock);
      refThis.mSubsystems.push_back(inSubsystem);
      notify1List = refThis.mOutputListeners;
      notify2List = refThis.mEventingListeners;
      notify3List = refThis.mEventingProviderListeners;

      {
        auto found = refThis.mDefaultOutputSubsystemLevels.find(String(inSubsystem->getName()));
        if (found != refThis.mDefaultOutputSubsystemLevels.end()) {
          inSubsystem->setOutputLevel(static_cast<Level>((*found).second));
        } else {
          auto foundDefault = refThis.mDefaultOutputSubsystemLevels.find(String());
          if (foundDefault != refThis.mDefaultOutputSubsystemLevels.end()) {
            inSubsystem->setOutputLevel(static_cast<Level>((*foundDefault).second));
          }
        }
      }

      // only if listeners attached then there's a reason to adjust the subsystem's current level
      if (refThis.mEventingListeners->size() > 0)
      {
        String subsystemName(inSubsystem->getName());

        // set the eventing level for the new subsystem in order or priority
        {
          auto found = refThis.mEventingSubsystemLevels.find(subsystemName);
          if (found != refThis.mEventingSubsystemLevels.end()) {
            inSubsystem->setEventingLevel(static_cast<Level>((*found).second));
            goto adjusted_level;
          }
        }
        {
          auto found = refThis.mEventingSubsystemLevels.find(String());
          if (found != refThis.mEventingSubsystemLevels.end()) {
            inSubsystem->setEventingLevel(static_cast<Level>((*found).second));
            goto adjusted_level;
          }
        }
        {
          auto found = refThis.mDefaultEventingSubsystemLevels.find(subsystemName);
          if (found != refThis.mDefaultEventingSubsystemLevels.end()) {
            inSubsystem->setEventingLevel(static_cast<Level>((*found).second));
            goto adjusted_level;
          }
        }
        {
          auto found = refThis.mDefaultEventingSubsystemLevels.find(String());
          if (found != refThis.mDefaultEventingSubsystemLevels.end()) {
            inSubsystem->setEventingLevel(static_cast<Level>((*found).second));
            goto adjusted_level;
          }
        }
        {
          inSubsystem->setEventingLevel(Level::None);
          goto adjusted_level;
        }
      adjusted_level:
        {}
      }
    }

    for (auto iter = notify1List->begin(); iter != notify1List->end(); ++iter)
    {
      (*iter)->notifyNewSubsystem(*inSubsystem);
    }
    for (auto iter = notify2List->begin(); iter != notify2List->end(); ++iter)
    {
      (*iter)->notifyNewSubsystem(*inSubsystem);
    }
    for (auto iter = notify3List->begin(); iter != notify3List->end(); ++iter)
    {
      (*iter)->notifyNewSubsystem(*inSubsystem);
    }
  }

  //---------------------------------------------------------------------------
  void Log::setOutputLevelByName(
                                 const char *subsystemName,
                                 Level level
                                 )
  {
    String subsystemNameStr(subsystemName);
    if (subsystemNameStr.isEmpty()) {
      subsystemNameStr = String();
    }

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    bool defaultAll = subsystemNameStr.isEmpty();

    {
      AutoRecursiveLock lock(refThis.mLock);
      refThis.mDefaultOutputSubsystemLevels[subsystemNameStr] = static_cast<decltype(level)>(static_cast<std::underlying_type<decltype(level)>::type>(level));

      // if no listeners attached then there's no reason to adjust the subsystem's current level
      if (refThis.mOutputListeners->size() < 1) return;

      for (auto iter = refThis.mSubsystems.begin(); iter != refThis.mSubsystems.end(); ++iter)
      {
        auto &subsystem = *(*iter);

        if (defaultAll) {
          subsystem.setOutputLevel(level);
          continue;
        }

        if (subsystemNameStr == subsystem.getName()) {
          subsystem.setOutputLevel(level);
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  void Log::log(
                const Subsystem &inSubsystem,
                Severity inSeverity,
                Level inLevel,
                const String &inMessage,
                CSTR inFunction,
                CSTR inFilePath,
                ULONG inLineNumber
                )
  {
    log(inSubsystem, inSeverity, inLevel, Log::Params(inMessage), inFunction, inFilePath, inLineNumber);
  }

  //-------------------------------------------------------------------------
  void Log::log(
                const Subsystem &inSubsystem,
                Severity inSeverity,
                Level inLevel,
                const Log::Params &inParams,
                CSTR inFunction,
                CSTR inFilePath,
                ULONG inLineNumber
                )
  {
    if (inLevel > inSubsystem.getOutputLevel())
      return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    OutputListenerListPtr notifyList;

    {
      AutoRecursiveLock lock(refThis.mLock);
      notifyList = refThis.mOutputListeners;
    }

    for (auto iter = notifyList->begin(); iter != notifyList->end(); ++iter)
    {
      (*iter)->notifyLog(
                         inSubsystem,
                         inSeverity,
                         inLevel,
                         inFunction,
                         inFilePath,
                         inLineNumber,
                         inParams
                         );
    }
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (eventing methods)
  #pragma mark

  //---------------------------------------------------------------------------
  void Log::addEventingListener(ILogEventingDelegatePtr delegate)
  {
    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    SubsystemList notifyList;

    // copy the list but notify without the lock
    {
      AutoRecursiveLock lock(refThis.mLock);

      EventingListenerListPtr replaceList(make_shared<EventingListenerList>());

      size_t originalSize = refThis.mEventingListeners->size();

      (*replaceList) = (*refThis.mEventingListeners);

      replaceList->push_back(delegate);

      refThis.mEventingListeners = replaceList;

      notifyList = refThis.mSubsystems;

      if (0 == originalSize) {

        auto foundAll = refThis.mEventingSubsystemLevels.find(String());
        auto foundDefault = refThis.mDefaultEventingSubsystemLevels.find(String());

        for (auto iter = refThis.mSubsystems.begin(); iter != refThis.mSubsystems.end(); ++iter)
        {
          auto *subsystem = (*iter);
          auto *name = subsystem->getName();

          String subsystemName(subsystem->getName());

          // set the individual subsystem level by order of priority
          {
            auto found = refThis.mEventingSubsystemLevels.find(subsystemName);
            if (found != refThis.mEventingSubsystemLevels.end()) {
              subsystem->setEventingLevel(static_cast<Level>((*found).second));
              goto adjusted_level;
            }
          }
          {
            if (foundAll != refThis.mEventingSubsystemLevels.end()) {
              subsystem->setEventingLevel(static_cast<Level>((*foundAll).second));
              goto adjusted_level;
            }
          }
          {
            auto found = refThis.mDefaultEventingSubsystemLevels.find(subsystemName);
            if (found != refThis.mDefaultEventingSubsystemLevels.end()) {
              subsystem->setEventingLevel(static_cast<Level>((*found).second));
              goto adjusted_level;
            }
          }
          {
            if (foundDefault != refThis.mDefaultEventingSubsystemLevels.end()) {
              subsystem->setEventingLevel(static_cast<Level>((*foundDefault).second));
              goto adjusted_level;
            }
          }
          {
            subsystem->setEventingLevel(Level::None);
            goto adjusted_level;
          }
        adjusted_level:
          {}
        }
      }
    }

    for (auto iter = notifyList.begin(); iter != notifyList.end(); ++iter)
    {
      delegate->notifyNewSubsystem(*(*iter));
    }
  }

  //---------------------------------------------------------------------------
  void Log::removeEventingListener(ILogEventingDelegatePtr delegate)
  {
    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    AutoRecursiveLock lock(refThis.mLock);

    EventingListenerListPtr replaceList(make_shared<EventingListenerList>());

    (*replaceList) = (*refThis.mEventingListeners);

    auto originalSize = replaceList->size();

    for (auto iter_doNotUse = replaceList->begin(); iter_doNotUse != replaceList->end(); )
    {
      auto current = iter_doNotUse;
      ++iter_doNotUse;

      if (delegate.get() == (*current).get()) {
        replaceList->erase(current);
      }
    }

    if (originalSize != replaceList->size()) {
      refThis.mEventingListeners = replaceList;

      if (0 == replaceList->size()) {
        for (auto innerIter = refThis.mSubsystems.begin(); innerIter != refThis.mSubsystems.end(); ++innerIter) {
          auto *subsystem = (*innerIter);
          subsystem->setEventingLevel(None);
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  void Log::addEventingProviderListener(ILogEventingProviderDelegatePtr delegate)
  {
    typedef std::list<EventingWriter *> EventWriterList;

    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    SubsystemList notifyList;
    EventWriterList eventWriterList;

    // copy the list but notify without the lock
    {
      AutoRecursiveLock lock(refThis.mLock);

      EventingProviderListenerListPtr replaceList(make_shared<EventingProviderListenerList>());

      (*replaceList) = (*refThis.mEventingProviderListeners);

      replaceList->push_back(delegate);

      refThis.mEventingProviderListeners = replaceList;

      notifyList = refThis.mSubsystems;

      for (auto iter = refThis.mEventWriters.begin(); iter != refThis.mEventWriters.end(); ++iter)
      {
        eventWriterList.push_back((*iter).second);
      }
    }

    for (auto iter = notifyList.begin(); iter != notifyList.end(); ++iter)
    {
      delegate->notifyNewSubsystem(*(*iter));
    }

    for (auto iter = eventWriterList.begin(); iter != eventWriterList.end(); ++iter)
    {
      auto *writer = (*iter);
      delegate->notifyEventingProviderRegistered(reinterpret_cast<uintptr_t>(writer), &(writer->mAtomInfo[0]));
    }

    for (auto iter = eventWriterList.begin(); iter != eventWriterList.end(); ++iter)
    {
      auto *writer = (*iter);
      if (0 != writer->mKeywordsBitmask) {
        delegate->notifyEventingProviderLoggingStateChanged(reinterpret_cast<uintptr_t>(writer), &(writer->mAtomInfo[0]), writer->mKeywordsBitmask);
      }
    }
  }

  //---------------------------------------------------------------------------
  void Log::removeEventingProviderListener(ILogEventingProviderDelegatePtr delegate)
  {
    if (!delegate) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    AutoRecursiveLock lock(refThis.mLock);

    EventingProviderListenerListPtr replaceList(make_shared<EventingProviderListenerList>());

    (*replaceList) = (*refThis.mEventingProviderListeners);

    for (auto iter = replaceList->begin(); iter != replaceList->end(); ++iter)
    {
      if (delegate.get() == (*iter).get())
      {
        replaceList->erase(iter);
        refThis.mEventingProviderListeners = replaceList;
        return;
      }
    }
  }

  //---------------------------------------------------------------------------
  Log::EventingAtomIndex Log::registerEventingAtom(const char *atomNamespace)
  {
    String atomNamespaceStr(atomNamespace);

    LogPtr log = Log::singleton();
    if (!log) return 0;

    Log &refThis = (*log);

    AutoRecursiveLock lock(refThis.mLock);

    auto found = refThis.mConsumedEventingAtoms.find(atomNamespaceStr);
    if (found != refThis.mConsumedEventingAtoms.end()) return (*found).second;

    auto nextIndex = static_cast<EventingAtomIndex>(refThis.mConsumedEventingAtoms.size() + 1);

    if (nextIndex >= ZSLIB_LOG_PROVIDER_ATOM_MAXIMUM) return 0;

    refThis.mConsumedEventingAtoms[atomNamespaceStr] = nextIndex;
    return nextIndex;
  }

  //---------------------------------------------------------------------------
  Log::ProviderHandle Log::registerEventingWriter(
                                                  const char *providerID,
                                                  const char *providerName,
                                                  const char *uniqueProviderHash,
                                                  const char *providerJMAN
                                                  )
  {
    try {
      UUID tempProviderID = Numeric<UUID>(providerID);
      return registerEventingWriter(tempProviderID, providerName, uniqueProviderHash, providerJMAN);
    } catch (const Numeric<UUID>::ValueOutOfRange &) {
    }

    return 0;
  }

  //---------------------------------------------------------------------------
  Log::ProviderHandle Log::registerEventingWriter(
                                                  const UUID &providerID,
                                                  const char *providerName,
                                                  const char *uniqueProviderHash,
                                                  const char *providerJMAN
                                                  )
  {
    LogPtr log = Log::singleton();
    if (!log) return 0;

    Log &refThis = (*log);

    EventingWriter *writer = NULL;

    EventingProviderListenerListPtr notifyList;

    {
      AutoRecursiveLock lock(refThis.mLock);

      {
        auto found = refThis.mEventWriters.find(providerID);
        if (found != refThis.mEventWriters.end()) {
          auto *existingWriter = (*found).second;
          return reinterpret_cast<uintptr_t>(existingWriter);
        }
      }

      writer = new EventingWriter;
      writer->mProviderID = providerID;
      writer->mProviderName = String(providerName);
      writer->mUniqueProviderHash = String(uniqueProviderHash);
      writer->mLog = log.get();
      writer->mJMAN = providerJMAN ? providerJMAN : "";
      refThis.mEventWriters[providerID] = writer;

      notifyList = refThis.mEventingProviderListeners;
    }

    uintptr_t result(reinterpret_cast<uintptr_t>(writer));

    KeywordBitmaskType keywords = writer->mKeywordsBitmask;

    for (auto iter = notifyList->begin(); iter != notifyList->end(); ++iter)
    {
      auto delegate = (*iter);
      delegate->notifyEventingProviderRegistered(result, &(writer->mAtomInfo[0]));
    }

    if (writer->mKeywordsBitmask != keywords) {
      for (auto iter = notifyList->begin(); iter != notifyList->end(); ++iter)
      {
        auto delegate = (*iter);
        delegate->notifyEventingProviderLoggingStateChanged(result, &(writer->mAtomInfo[0]), writer->mKeywordsBitmask);
      }
    }

    return reinterpret_cast<uintptr_t>(writer);
  }

  //---------------------------------------------------------------------------
  void Log::unregisterEventingWriter(ProviderHandle handle)
  {
    if (0 == handle) return;

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    EventingWriter *writer = reinterpret_cast<EventingWriter *>(handle);

    EventingProviderListenerListPtr notifyList;

    {
      AutoRecursiveLock lock(refThis.mLock);

      for (auto iter = refThis.mEventWriters.begin(); iter != refThis.mEventWriters.end(); ++iter)
      {
        uintptr_t found = reinterpret_cast<uintptr_t>((*iter).second);
        if (found != handle) continue;

        refThis.mEventWriters.erase(iter);
        break;
      }

      refThis.mCleanUpWriters.insert(writer);
      notifyList = refThis.mEventingProviderListeners;
    }

    for (auto iter = notifyList->begin(); iter != notifyList->end(); ++iter)
    {
      auto delegate = (*iter);
      delegate->notifyEventingProviderUnregistered(handle, &(writer->mAtomInfo[0]));
    }
  }

  //---------------------------------------------------------------------------
  bool Log::getEventingWriterInfo(
                                  ProviderHandle handle,
                                  GetEventingWriterInfoResult &result
                                  )
  {
    if (0 == handle) return false;

    EventingWriter *writer = reinterpret_cast<EventingWriter *>(handle);
    if (ZSLIB_INTERNAL_LOG_EVENT_WRITER_INIT_VALUE != writer->mInitValue) return false;

    result.providerID_ = writer->mProviderID;
    result.providerName_ = writer->mProviderName;
    result.uniqueProviderHash_ = writer->mUniqueProviderHash;
    if (result.includeJMAN_) {
      result.jman_ = writer->mJMAN;
    }
    result.atomArray_ = &(writer->mAtomInfo[0]);
    return true;
  }

  //---------------------------------------------------------------------------
  void Log::setDefaultEventingLevelByName(
                                          const char *subsystemName,
                                          Level level
                                          )
  {
    String subsystemNameStr(subsystemName);
    if (subsystemNameStr.isEmpty()) {
      subsystemNameStr = String();
    }

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    bool defaultAll = subsystemNameStr.isEmpty();

    {
      AutoRecursiveLock lock(refThis.mLock);
      refThis.mDefaultEventingSubsystemLevels[subsystemNameStr] = static_cast<decltype(level)>(static_cast<std::underlying_type<decltype(level)>::type>(level));

      // if no listeners attached then there's no reason to adjust the subsystem's current level
      if (refThis.mEventingListeners->size() < 1) return;

      // default level will never take affect while an eventing level for all compoments is in affect
      if (refThis.mEventingSubsystemLevels.end() != refThis.mEventingSubsystemLevels.find(String())) return;

      for (auto iter = refThis.mSubsystems.begin(); iter != refThis.mSubsystems.end(); ++iter)
      {
        auto &subsystem = *(*iter);

        String checkSubsystemName = String(subsystem.getName());

        // default level will not take affect if the component has an existing event level set for the specific componenet
        if (refThis.mEventingSubsystemLevels.end() != refThis.mEventingSubsystemLevels.find(checkSubsystemName)) continue;

        if (defaultAll) {
          // default for all will not take affect if there is a default for the specific component
          if (refThis.mDefaultEventingSubsystemLevels.end() != refThis.mDefaultEventingSubsystemLevels.find(checkSubsystemName)) continue;

          subsystem.setEventingLevel(level);
          continue;
        }

        if (subsystemNameStr == checkSubsystemName) {
          subsystem.setEventingLevel(level);
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  void Log::setEventingLevelByName(
                                   const char *subsystemName,
                                   Level level
                                   )
  {
    String subsystemNameStr(subsystemName);
    if (subsystemNameStr.isEmpty()) {
      subsystemNameStr = String();
    }

    LogPtr log = Log::singleton();
    if (!log) return;

    Log &refThis = (*log);

    bool defaultAll = subsystemNameStr.isEmpty();

    {
      AutoRecursiveLock lock(refThis.mLock);
      refThis.mEventingSubsystemLevels[subsystemNameStr] = static_cast<decltype(level)>(static_cast<std::underlying_type<decltype(level)>::type>(level));

      // if no listeners attached then there's no reason to adjust the subsystem's current level
      if (refThis.mEventingListeners->size() < 1) return;

      for (auto iter = refThis.mSubsystems.begin(); iter != refThis.mSubsystems.end(); ++iter)
      {
        auto &subsystem = *(*iter);
        String checkSubsystemName = String(subsystem.getName());

        if (defaultAll) {
          // set all level will not take affect if there is a value for the specific component
          if (refThis.mEventingSubsystemLevels.end() != refThis.mEventingSubsystemLevels.find(checkSubsystemName)) continue;
          subsystem.setEventingLevel(level);
          continue;
        }

        if (subsystemNameStr == checkSubsystemName) {
          subsystem.setEventingLevel(level);
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  void Log::writeEvent(
                       ProviderHandle handle,
                       Severity severity,
                       Level level,
                       EVENT_DESCRIPTOR_HANDLE descriptor,
                       EVENT_PARAMETER_DESCRIPTOR_HANDLE paramDescriptor,
                       EVENT_DATA_DESCRIPTOR_HANDLE dataDescriptor,
                       size_t dataDescriptorCount
                       )
  {
    if (0 == handle) return;

    EventingWriter *writer = reinterpret_cast<EventingWriter *>(handle);
    if (ZSLIB_INTERNAL_LOG_EVENT_WRITER_INIT_VALUE != writer->mInitValue) return;

    Log *log = writer->mLog;

    EventingListenerListPtr notifyList;

    {
      AutoRecursiveLock lock(log->mLock);
      notifyList = log->mEventingListeners;
    }

    EventingListenerList &useList = *notifyList;

    for (auto iter = useList.begin(); iter != useList.end(); ++iter)
    {
      (*iter)->notifyWriteEvent(handle, &(writer->mAtomInfo[0]), severity, level, descriptor, paramDescriptor, dataDescriptor, dataDescriptorCount);
    }
  }

  //---------------------------------------------------------------------------
  void Log::setEventingLogging(
                               ProviderHandle handle,
                               PUID enablingObjectID,
                               bool enabled,
                               KeywordBitmaskType bitmask
                               )
  {
    if (0 == handle) return;

    EventingWriter *writer = reinterpret_cast<EventingWriter *>(handle);
    if (ZSLIB_INTERNAL_LOG_EVENT_WRITER_INIT_VALUE != writer->mInitValue) return;

    Log *log = writer->mLog;

    EventingProviderListenerListPtr notifyList;

    KeywordBitmaskType keywords = 0;
    
    {
      AutoRecursiveLock lock(log->mLock);
      if (!enabled) {
        auto found = writer->mEnabledObjects.find(enablingObjectID);
        if (found != writer->mEnabledObjects.end()) writer->mEnabledObjects.erase(found);
      } else {
        writer->mEnabledObjects[enablingObjectID] = bitmask;
      }

      // re-assemble the enabled keyword bitmask
      for (auto iter = writer->mEnabledObjects.begin(); iter != writer->mEnabledObjects.end(); ++iter) {
        auto enabledBitmask = (*iter).second;
        keywords = keywords | enabledBitmask;
      }
      writer->mKeywordsBitmask = keywords;
      
      notifyList = log->mEventingProviderListeners;
    }

    auto &useList = *notifyList;

    for (auto iter = useList.begin(); iter != useList.end(); ++iter)
    {
      (*iter)->notifyEventingProviderLoggingStateChanged(handle, &(writer->mAtomInfo[0]), keywords);
    }
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark Log::Params
  #pragma mark

  //---------------------------------------------------------------------------
  Log::Param::Param(const Param &param) :
    mParam(param.mParam)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const char *value, bool isNumber)
  {
    if (!value) return;
    if ('\0' == *value) return;

    mParam = XML::Element::create(name);

    XML::TextPtr tmpTxt = XML::Text::create();
    if (isNumber) {
      tmpTxt->setValue(value, XML::Text::Format_JSONNumberEncoded);
    } else {
      tmpTxt->setValueAndJSONEncode(value);
    }
    mParam->adoptAsFirstChild(tmpTxt);
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const String &value, bool isNumber)
  {
    if (value.isEmpty()) return;

    mParam = XML::Element::create(name);

    XML::TextPtr tmpTxt = XML::Text::create();
    if (isNumber) {
      tmpTxt->setValue(value, XML::Text::Format_JSONNumberEncoded);
    } else {
      tmpTxt->setValueAndJSONEncode(value);
    }
    mParam->adoptAsFirstChild(tmpTxt);
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, XML::ElementPtr value)
  {
    if (!value) return;

    mParam = XML::Element::create(name);

    mParam->adoptAsLastChild(value);
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, bool value) :
    Param(name, value ? "true" : "false", true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, CHAR value) :
    Param(name, (INT)value)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, UCHAR value) :
    Param(name, (UINT)value)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, SHORT value) :
    Param(name, (INT)value)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, USHORT value) :
    Param(name, (UINT)value)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, INT value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, UINT value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, LONG value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, ULONG value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, LONGLONG value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, ULONGLONG value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, FLOAT value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, DOUBLE value) :
    Param(name, string(value), true)
  {
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Time &value)
  {
    if (Time() == value) return;

    Param tmp(name, string(value), true);

    mParam = tmp.mParam;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Hours &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Minutes &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Seconds &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Milliseconds &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Microseconds &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::Param(const char *name, const Nanoseconds &value)
  {
    if (Hours() == value) return;
    Param temp(name, string(value));
    mParam = temp.mParam;
    return;
  }

  //---------------------------------------------------------------------------
  Log::Param::~Param()
  {    
  }

  //---------------------------------------------------------------------------
  const XML::ElementPtr &Log::Param::param() const
  {
    return mParam;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark Log::Params
  #pragma mark

  //---------------------------------------------------------------------------
  Log::Params::Params()
  {
  }

  //---------------------------------------------------------------------------
  Log::Params::Params(const char *message, XML::ElementPtr object) :
    mObject(object),
    mMessage(message)
  {
  }

  //---------------------------------------------------------------------------
  Log::Params::Params(const String &message, XML::ElementPtr object) :
    mObject(object),
    mMessage(message)
  {
  }

  //---------------------------------------------------------------------------
  Log::Params::Params(const char *message, const char *staticObjectName) :
    mObject(XML::Element::create(staticObjectName)),
    mMessage(message)
  {
    ZS_THROW_INVALID_ARGUMENT_IF(!staticObjectName)
  }

  //---------------------------------------------------------------------------
  Log::Params::Params(const String &message, const char *staticObjectName) :
    mObject(XML::Element::create(staticObjectName)),
    mMessage(message)
  {
    ZS_THROW_INVALID_ARGUMENT_IF(!staticObjectName)
  }

  //---------------------------------------------------------------------------
  Log::Params::Params(const Params &params) :
    mObject(params.mObject),
    mParams(params.mParams),
    mMessage(params.mMessage)
  {
  }

  //---------------------------------------------------------------------------
  Log::Params::~Params()
  {
  }

  //---------------------------------------------------------------------------
  Log::Params &Log::Params::operator<<(const XML::ElementPtr &param)
  {
    if (!param) return *this;

    if (!mParams) {
      mParams = XML::Element::create("params");
    }

    mParams->adoptAsLastChild(param);

    return *this;
  }

  //---------------------------------------------------------------------------
  Log::Params &Log::Params::operator<<(const Param &param)
  {
    return (*this) << (param.param());
  }

  //---------------------------------------------------------------------------
  Log::Params &Log::Params::operator<<(const Params &params)
  {
    if (params.mMessage.hasData()) {
      mMessage = params.mMessage;
    }
    if (params.mObject) {
      mObject = params.mObject;
    }
    if (params.mParams) {
      mParams = params.mParams;
    }
    return (*this);
  }

  //---------------------------------------------------------------------------
  const String &Log::Params::message() const
  {
    return mMessage;
  }

  //---------------------------------------------------------------------------
  const XML::ElementPtr &Log::Params::object() const
  {
    return mObject;
  }

  //---------------------------------------------------------------------------
  const XML::ElementPtr &Log::Params::params() const
  {
    return mParams;
  }

} // namespace zsLib
