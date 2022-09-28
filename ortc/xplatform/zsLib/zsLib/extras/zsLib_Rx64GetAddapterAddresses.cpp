/*

 Copyright (c) 2017, Optical Tone Ltd.
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

#include <zsLib/types.h>
#include <zsLib/IPAddress.h>

#ifdef WIN32_RX64

#include <iptypes.h>

using namespace Windows::Networking::Connectivity;


namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark helpers
    #pragma mark
    
    //-------------------------------------------------------------------------
    static size_t Rx64AlignedPadding(size_t sourceSizeInBytes)
    {
      if (0 == sourceSizeInBytes % sizeof(uintptr_t)) return 0;
      return sizeof(uintptr_t) - (sourceSizeInBytes % sizeof(uintptr_t));
    }

    //-------------------------------------------------------------------------
    static size_t Rx64AlignedSize(size_t sourceSizeInBytes)
    {
      return sourceSizeInBytes + Rx64AlignedPadding(sourceSizeInBytes);
    }

    //-------------------------------------------------------------------------
    static PIP_ADAPTER_ADDRESSES Rx64GetAdapterPointer(BYTE *buffer, size_t index, size_t maxAdapters)
    {
      if (NULL == buffer) return NULL;
      if (index + 1 >= maxAdapters) return NULL;

      size_t alignedSize = Rx64AlignedSize(sizeof(IP_ADAPTER_ADDRESSES));
      buffer += static_cast<uintptr_t>(static_cast<uintptr_t>(alignedSize) * static_cast<uintptr_t>(index));
      return reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer);
    }

    //-------------------------------------------------------------------------
    static PIP_ADAPTER_UNICAST_ADDRESS Rx64GetIPAddressPointer(
                                                               BYTE *buffer,
                                                               size_t index,
                                                               size_t maxIPs,
                                                               size_t totalAdapters
                                                               )
    {
      if (NULL == buffer) return NULL;
      if (index + 1 >= maxIPs) return NULL;

      size_t alignedAdapterSize = Rx64AlignedSize(sizeof(IP_ADAPTER_ADDRESSES));
      buffer += static_cast<uintptr_t>(static_cast<uintptr_t>(alignedAdapterSize) * static_cast<uintptr_t>(totalAdapters));
      size_t unicastSize = Rx64AlignedSize(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
      buffer += static_cast<uintptr_t>(static_cast<uintptr_t>(unicastSize) * static_cast<uintptr_t>(index));
      return reinterpret_cast<PIP_ADAPTER_UNICAST_ADDRESS>(buffer);
    }

    //-------------------------------------------------------------------------
    static BYTE *Rx64GetStringBufferPos(
                                        BYTE *buffer,
                                        size_t totalAdapters,
                                        size_t totalIPs
                                        )
    {
      if (NULL == buffer) return NULL;

      size_t alignedAdapterSize = Rx64AlignedSize(sizeof(IP_ADAPTER_ADDRESSES));
      buffer += static_cast<uintptr_t>(static_cast<uintptr_t>(alignedAdapterSize) * static_cast<uintptr_t>(totalAdapters));
      size_t unicastSize = Rx64AlignedSize(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
      buffer += static_cast<uintptr_t>(static_cast<uintptr_t>(unicastSize) * static_cast<uintptr_t>(totalIPs));

      return buffer;
    }

    //-------------------------------------------------------------------------
    static void Rx64AddSpaceForString(size_t &ioSizeNeeded, const String &str)
    {
      if (!str.hasData()) return;

      ioSizeNeeded += (str.length() + 1) * sizeof(char);
    }

    //-------------------------------------------------------------------------
    static void Rx64AddSpaceForString(size_t &ioSizeNeeded, const std::wstring &str)
    {
      if (0 == str.length()) return;
      ioSizeNeeded += (str.length() + 1) * sizeof(wchar_t);
    }

    //-------------------------------------------------------------------------
    static void Rx64AddSpaceForSocketAddress(size_t &ioSizeNeeded, const IPAddress &ip)
    {
      if (ip.isIPv4()) {
        ioSizeNeeded += Rx64AlignedSize(sizeof(sockaddr_in));
      } else if (ip.isIPv6()) {
        ioSizeNeeded += Rx64AlignedSize(sizeof(sockaddr_in6));
      }
    }

    //-------------------------------------------------------------------------
    static char *Rx64PrepareString(
                                   BYTE * &ioBufferPos,
                                   const String &str
                                   )
    {
      if (NULL == ioBufferPos) return NULL;

      char *result = reinterpret_cast<char *>(ioBufferPos);
      memset(result, 0, (str.length() + 1) * sizeof(char));
      memcpy(result, str.c_str(), str.length() * sizeof(char));
      ioBufferPos += static_cast<uintptr_t>((str.length() + 1) * sizeof(char));
      return result;
    }

    //-------------------------------------------------------------------------
    static sockaddr *Rx64PrepareIP(
                                   BYTE * &ioBufferPos,
                                   const IPAddress &ip
                                   )
    {
      if (NULL == ioBufferPos) return NULL;

      sockaddr *result = NULL;

      if (ip.isIPv4()) {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        ip.getIPv4(addr);
        result = reinterpret_cast<sockaddr *>(ioBufferPos);
        memcpy(ioBufferPos, &addr, sizeof(addr));
        ioBufferPos += Rx64AlignedSize(sizeof(sockaddr_in));
      }
      else if (ip.isIPv6()) {
        sockaddr_in6 addr;
        memset(&addr, 0, sizeof(addr));
        ip.getIPv6(addr);
        result = reinterpret_cast<sockaddr *>(ioBufferPos);
        memcpy(ioBufferPos, &addr, sizeof(addr));
        ioBufferPos += Rx64AlignedSize(sizeof(sockaddr_in6));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    static wchar_t *Rx64PrepareString(
                                      BYTE * &ioBufferPos,
                                      const std::wstring &str
                                      )
    {
      if (NULL == ioBufferPos) return NULL;

      wchar_t *result = reinterpret_cast<wchar_t *>(ioBufferPos);

      memset(result, 0, (str.length() + 1) * sizeof(wchar_t));
      memcpy(result, str.c_str(), str.length() * sizeof(wchar_t));

      ioBufferPos += static_cast<uintptr_t>((str.length() + 1) * sizeof(wchar_t));
      return result;
    }

    //-------------------------------------------------------------------------
    static void Rx64MemCopyIfPossible(
      BYTE *dest,
      const BYTE *source,
      size_t sourceSizeInBytes
    )
    {
      if (NULL == source) return;
      if (0 == sourceSizeInBytes) return;

      if (NULL == dest) return;

      memcpy(dest, source, sourceSizeInBytes);
    }

    //-------------------------------------------------------------------------
    ULONG WINAPI Rx64GetAdaptersAddresses(
      _In_    ULONG                 Family,
      _In_    ULONG                 Flags,
      _In_    PVOID                 Reserved,
      _Inout_ PIP_ADAPTER_ADDRESSES AdapterAddresses,
      _Inout_ PULONG                SizePointer
    )
    {
      if ((AF_INET != Family) &&
          (AF_INET6 != Family) &&
          (AF_UNSPEC != Family)) return ERROR_INVALID_PARAMETER;
      if (NULL == SizePointer) return ERROR_INVALID_PARAMETER;

      size_t totalIPs = 0;
      size_t totalAdapters = 0;
      size_t stringSizeNeeded = 0;
      size_t ipSizeNeeded = 0;

      typedef size_t ConnectionProfileIndex;
      struct IPEntry
      {
        ConnectionProfileIndex connectionProfileIndex_ {};
        ConnectionProfile^ profile_ {};
        String useName_;
        IPAddress ip_;
        size_t entryIndex_ {};
        PIP_ADAPTER_UNICAST_ADDRESS pos_ {};
        PIP_ADAPTER_UNICAST_ADDRESS next_ {};
      };
      typedef std::list<IPEntry> IPEntryList;

      struct AdapterInfo
      {
        String adapterName_;
        std::wstring description_;
        std::wstring friendlyName_;
        IPEntryList entries_;
        DWORD ifType_ {IF_TYPE_ETHERNET_CSMACD};
      };

      typedef ULONG FamilyType;
      typedef std::pair<FamilyType, ConnectionProfileIndex> FamilyProfilePair;

      typedef std::map<FamilyProfilePair, AdapterInfo> ProfileIPListMap;

      ProfileIPListMap results;

      // http://stackoverflow.com/questions/10336521/query-local-ip-address

      // Use WinRT GetHostNames to search for IP addresses
      {
        typedef std::map<String, bool> HostNameMap;
        typedef std::list<ConnectionProfile ^> ConnectionProfileList;

        HostNameMap previousFound;
        ConnectionProfileList profiles;

        // discover connection profiles
        {
          auto connectionProfiles = NetworkInformation::GetConnectionProfiles();
          for (auto iter = connectionProfiles->First(); iter->HasCurrent; iter->MoveNext()) {
            auto profile = iter->Current;
            if (nullptr == profile) continue;
            profiles.push_back(profile);
          }

          ConnectionProfile ^current = NetworkInformation::GetInternetConnectionProfile();
          if (current) profiles.push_back(current);
        }

        // search connection profiles with host names found
        {
          auto hostnames = NetworkInformation::GetHostNames();
          for (auto iter = hostnames->First(); iter->HasCurrent; iter->MoveNext()) {
            auto hostname = iter->Current;
            if (nullptr == hostname) continue;

            String canonicalName;
            if (hostname->CanonicalName) {
              canonicalName = String(hostname->CanonicalName->Data());
            }

            String displayName;
            if (hostname->DisplayName) {
              displayName = String(hostname->DisplayName->Data());
            }

            String rawName;
            if (hostname->RawName) {
              rawName = String(hostname->RawName->Data());
            }

            String useName = rawName;

            auto found = previousFound.find(useName);
            if (found != previousFound.end()) continue;

            ConnectionProfileIndex foundIndex {0};
            ConnectionProfile ^hostProfile = nullptr;
            if (hostname->IPInformation) {
              if (hostname->IPInformation->NetworkAdapter) {
                auto hostNetworkAdapter = hostname->IPInformation->NetworkAdapter;
                ConnectionProfileIndex searchIndex {1};
                for (auto profileIter = profiles.begin(); profileIter != profiles.end(); ++profileIter, ++searchIndex) {
                  auto profile = (*profileIter);
                  auto adapter = profile->NetworkAdapter;
                  if (nullptr == adapter) continue;
                  if (adapter->NetworkAdapterId != hostNetworkAdapter->NetworkAdapterId) continue;

                  // match found
                  hostProfile = profile;
                  foundIndex = searchIndex;
                  break;
                }
              }
            }

            previousFound[useName] = true;

            IPAddress ip;

            if (IPAddress::isConvertable(useName)) {
              try {
                IPAddress temp(useName);
                ip = temp;
              } catch (IPAddress::Exceptions::ParseError &) {
              }

              if (ip.isAddressEmpty()) continue;
              if (ip.isAddrAny()) continue;
            }

            FamilyProfilePair profilePair;
            profilePair.first = AF_UNSPEC;
            profilePair.second = foundIndex;
            if (ip.isIPv6()) {
              profilePair.first = AF_INET;
            }
            if (ip.isIPv6()) {
              profilePair.first = AF_INET6;
            }
            if (AF_UNSPEC == profilePair.first) continue;

            // filter out wrong IPs
            if ((AF_INET != Family) && (AF_UNSPEC != Family)) {
              if (!ip.isIPv4()) continue;
            }
            if ((AF_INET6 != Family) && (AF_UNSPEC != Family)) {
              if (!ip.isIPv6()) continue;
            }

            bool created {};

            auto foundEntry = results.find(profilePair);
            if (foundEntry == results.end()) {
              ++totalAdapters;
              results[profilePair] = AdapterInfo();
              created = true;
              foundEntry = results.find(profilePair);
            }

            auto &adapterInfo = (*foundEntry).second;
            if (created) {
              if (hostProfile) {
                if (hostProfile->ProfileName) {
                  adapterInfo.adapterName_ = String(hostProfile->ProfileName->Data());
                }
                if (hostname->DisplayName) {
                  adapterInfo.friendlyName_ = displayName.wstring();
                }
                if (hostname->CanonicalName) {
                  adapterInfo.description_ = canonicalName.wstring();
                }
              }

              Rx64AddSpaceForString(stringSizeNeeded, adapterInfo.adapterName_);
              Rx64AddSpaceForString(stringSizeNeeded, adapterInfo.description_);
              Rx64AddSpaceForString(stringSizeNeeded, adapterInfo.friendlyName_);

              if (ip.isLoopback()) {
                adapterInfo.ifType_ = IF_TYPE_SOFTWARE_LOOPBACK;
              }
            }

            IPEntry newIP;
            newIP.profile_ = hostProfile;
            newIP.useName_ = useName;
            newIP.ip_ = ip;
            newIP.entryIndex_ = totalIPs;
            adapterInfo.entries_.push_back(newIP);

            Rx64AddSpaceForSocketAddress(ipSizeNeeded, ip);

            ++totalIPs;
          }
        }
      }

      uintptr_t outputSizeNeeded = static_cast<uintptr_t>(Rx64AlignedSize(sizeof(IP_ADAPTER_ADDRESSES))) * static_cast<uintptr_t>(totalAdapters);
      outputSizeNeeded += static_cast<uintptr_t>(Rx64AlignedSize(sizeof(IP_ADAPTER_UNICAST_ADDRESS))) * static_cast<uintptr_t>(totalIPs);
      outputSizeNeeded += Rx64AlignedSize(stringSizeNeeded);
      outputSizeNeeded += static_cast<uintptr_t>(ipSizeNeeded);

      uintptr_t currentSize = static_cast<uintptr_t>(*SizePointer);
      *SizePointer = static_cast<ULONG>(outputSizeNeeded); // actual size is filled in

      if (currentSize < outputSizeNeeded) return ERROR_BUFFER_OVERFLOW;

      BYTE *source = reinterpret_cast<BYTE *>(AdapterAddresses);
      if (NULL == source) return ERROR_BUFFER_OVERFLOW;

      BYTE *strPos = Rx64GetStringBufferPos(source, totalAdapters, totalIPs);
      BYTE *addrPos = strPos + static_cast<uintptr_t>(Rx64AlignedSize(stringSizeNeeded));

      size_t ip4Index = 0;
      size_t ip6Index = 0;

      size_t adapterIndex = 0;
      for (auto iter = results.begin(); iter != results.end(); ++iter, ++adapterIndex)
      {
        auto &pairInfo = (*iter).first;
        auto &family = pairInfo.first;
        auto &profile = pairInfo.second;
        auto &adapterInfo = (*iter).second;
        auto &ipEntries = adapterInfo.entries_;

        bool isIpV4 = (AF_INET == family);
        bool isIpV6 = (AF_INET6 == family);

        ip4Index = ip4Index + (isIpV4 ? 1 : 0);
        ip6Index = ip6Index + (isIpV6 ? 1 : 0);

        IP_ADAPTER_ADDRESSES info {};
        memset(&info, 0, sizeof(info));

        info.Length = sizeof(info);
        info.IfIndex = isIpV4 ? static_cast<decltype(info.IfIndex)>(ip4Index) : 0;
        info.Ipv6IfIndex = isIpV6 ? static_cast<decltype(info.Ipv6IfIndex)>(ip6Index) : 0;
        info.Next = Rx64GetAdapterPointer(source, adapterIndex+1, results.size());

        info.AdapterName = Rx64PrepareString(strPos, adapterInfo.adapterName_);
        info.Description = Rx64PrepareString(strPos, adapterInfo.description_);
        info.FriendlyName = Rx64PrepareString(strPos, adapterInfo.friendlyName_);

        info.IfType = adapterInfo.ifType_;
        info.OperStatus = IfOperStatusUp;

        // fill in pointer to first UNICAST address
        {
          auto found = ipEntries.begin();
          if (found != ipEntries.end()) {
            auto &entry = (*found);
            info.FirstUnicastAddress = Rx64GetIPAddressPointer(source, entry.entryIndex_, totalIPs,  results.size());
          }
        }

        Rx64MemCopyIfPossible(reinterpret_cast<BYTE *>(Rx64GetAdapterPointer(source, adapterIndex, results.size())), reinterpret_cast<const BYTE *>(&info), sizeof(info));

        // calculate UNICAST pointer linked list
        {
          IPEntry *lastEntry{};
          for (auto iterEntry = ipEntries.begin(); iterEntry != ipEntries.end(); ++iterEntry)
          {
            auto &entry = (*iterEntry);
            entry.pos_ = Rx64GetIPAddressPointer(source, entry.entryIndex_, totalIPs, results.size());
            if (lastEntry) lastEntry->next_ = entry.pos_;
            lastEntry = &entry;
          }
        }

        // fill in UNICAST addresses
        for (auto iterEntry = ipEntries.begin(); iterEntry != ipEntries.end(); ++iterEntry)
        {
          auto &entry = (*iterEntry);

          IP_ADAPTER_UNICAST_ADDRESS unicast;
          memset(&unicast, 0, sizeof(unicast));

          unicast.Length = sizeof(IP_ADAPTER_UNICAST_ADDRESS);
          unicast.Next = entry.next_;

          if (isIpV4) {
            Rx64PrepareIP(addrPos, entry.ip_);
            unicast.Address.iSockaddrLength = static_cast<decltype(unicast.Address.iSockaddrLength)>(sizeof(sockaddr_in));
          } else if (isIpV6) {
            Rx64PrepareIP(addrPos, entry.ip_);
            unicast.Address.iSockaddrLength = static_cast<decltype(unicast.Address.iSockaddrLength)>(sizeof(sockaddr_in6));
          }

          Rx64MemCopyIfPossible(reinterpret_cast<BYTE *>(entry.pos_), reinterpret_cast<const BYTE *>(&unicast), sizeof(unicast));
        }
      }

      return NOERROR;
    }

  } // namespace internal
} // namespace zsLib

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
ULONG WINAPI Rx64GetAdaptersAddresses(
  _In_    ULONG                 Family,
  _In_    ULONG                 Flags,
  _In_    PVOID                 Reserved,
  _Inout_ PIP_ADAPTER_ADDRESSES AdapterAddresses,
  _Inout_ PULONG                SizePointer
)
{
  return zsLib::internal::Rx64GetAdaptersAddresses(Family, Flags, Reserved, AdapterAddresses, SizePointer);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //WIN32_RX64
