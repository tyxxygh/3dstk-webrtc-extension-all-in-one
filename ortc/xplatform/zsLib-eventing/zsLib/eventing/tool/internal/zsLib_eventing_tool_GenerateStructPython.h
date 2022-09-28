/*

Copyright (c) 2017, Robin Raymond
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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_IDLCompiler.h>

#include <sstream>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
        
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructPython
        #pragma mark

        struct GenerateStructPython : public IIDLCompilerTarget,
                                      public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          struct BaseFile
          {
            ProjectPtr project_;
            NamespacePtr global_;
            NamePathStructSetMap derives_;
            StringSet boxings_;

            String fileName_;
            String indent_;

            std::stringstream importSS_;
            std::stringstream usingAliasSS_;

            std::stringstream namespaceSS_;
            std::stringstream preStructSS_;
            std::stringstream structDeclationsSS_;
            std::stringstream structSS_;
            std::stringstream postStructSS_;

            std::stringstream preStructEndSS_;
            std::stringstream structEndSS_;
            std::stringstream postStructEndSS_;
            std::stringstream namespaceEndSS_;

            StringSet alreadyUsing_;

            BaseFile();
            ~BaseFile();

            void indentMore() { indent_ += "    "; }
            void indentLess() { indent_ = indent_.substr(0, indent_.length() - 4); }

            void usingTypedef(
                              const String &usingType,
                              const String &originalType = String(),
                              const String &asType = String()
                              );
            void usingTypedef(IEventingTypes::PredefinedTypedefs type);
            void usingTypedef(TypePtr type);

            bool hasBoxing(const String &namePathStr);

            void startRegion(const String &region);
            void endRegion(const String &region);

            void startOtherRegion(
                                  const String &region,
                                  bool preStruct
                                  );
            void endOtherRegion(
                                const String &region,
                                bool preStruct
                                );
          };

          struct SetupFile : public BaseFile
          {
          };

          struct ApiFile : public BaseFile
          {
            std::stringstream &helpersSS_;
            std::stringstream &helpersEndSS_;

            ApiFile();
            ~ApiFile();

            void startHelpersRegion(const String &region) { startOtherRegion(region, false); }
            void endHelpersRegion(const String &region) { endOtherRegion(region, false); }
          };

          struct EnumFile : public BaseFile
          {
          };

          struct InitFile : public BaseFile
          {
          };

          struct StructFile : public ApiFile
          {
            std::stringstream &interfaceSS_;
            std::stringstream &interfaceEndSS_;
            std::stringstream &delegateSS_;

            StructFile(
                       BaseFile &baseFile,
                       StructPtr structObj
                       );
            ~StructFile();

            StructPtr struct_;
            bool isStaticOnly_ {};
            bool isDictionary {};
            bool hasEvents_ {};
            bool shouldDefineInterface_ {};
            bool shouldInheritException_ {};
          };

          GenerateStructPython();

          static GenerateStructPythonPtr create();

          static String fixName(const String &originalName);
          static String fixNamespace(NamespacePtr namespaceObj);
          static String fixNamePath(ContextPtr context);
          static String flattenPath(const String &originalName);

          static String getPythonConvertedCType(IEventingTypes::PredefinedTypedefs type);
          static String getPythonConvertedCType(TypePtr type);
          static String getPythonCType(IEventingTypes::PredefinedTypedefs type);
          static String getPythonToCType(
                                         const String &inputValue,
                                         IEventingTypes::PredefinedTypedefs type
                                         );
          static String getCToPythonType(
                                         const String &inputValue,
                                         IEventingTypes::PredefinedTypedefs type
                                         );
          static String fixArgumentName(const String &value);
          static String fixCPythonType(IEventingTypes::PredefinedTypedefs type);
          static String fixCPythonType(TypePtr type);
          static String fixPythonType(
                                      TypePtr type,
                                      bool isInterface = false
                                      );
          static String fixPythonPathType(
                                          TypePtr type,
                                          bool isInterface = false
                                          );
          static String fixPythonType(
                                      bool isOptional,
                                      TypePtr type,
                                      bool isInterface = false
                                      );
          static String fixPythonPathType(
                                          bool isOptional,
                                          TypePtr type,
                                          bool isInterface = false
                                          );
          static String fixCsSystemType(IEventingTypes::PredefinedTypedefs type);
          static String combineIf(
                                  const String &combinePreStr,
                                  const String &combinePostStr,
                                  const String &ifHasValue
                                  );
          static String getHelpersMethod(
                                         BaseFile &baseFile,
                                         bool useApiHelper,
                                         const String &methodName,
                                         bool isOptional,
                                         TypePtr type
                                         );
          static String getToCMethod(
                                     BaseFile &baseFile,
                                     bool isOptional,
                                     TypePtr type
                                     );
          static String getFromCMethod(
                                       BaseFile &baseFile,
                                       bool isOptional,
                                       TypePtr type
                                       );
          static String getAdoptFromCMethod(
                                            BaseFile &baseFile,
                                            bool isOptional, 
                                            TypePtr type
                                            );
          static String getDestroyCMethod(
                                          BaseFile &baseFile,
                                          bool isOptional, 
                                          TypePtr type
                                          );

          static bool hasInterface(StructPtr structObj);

          static String getApiCastRequiredDefine(BaseFile &baseFile);
          static String getApiPath();
          static String getHelperPath();

          static bool shouldDeriveFromException(
                                                BaseFile &baseFile,
                                                StructPtr structObj
                                                );

          static void finalizeBaseFile(BaseFile &apiFile);

          static void prepareSetupFile(SetupFile &apiFile);
          static void finalizeSetupFile(SetupFile &apiFile);

          static void prepareApiInitFile(InitFile &apiFile);
          static void finalizeApiInitFile(InitFile &apiFile);

          static void prepareApiFile(ApiFile &apiFile);
          static void finalizeApiFile(ApiFile &apiFile);

          static void prepareApiCallback(ApiFile &apiFile);

          static void prepareApiExceptions(ApiFile &apiFile);
          static void prepareApiExceptions(
                                           ApiFile &apiFile,
                                           const String &exceptionName
                                           );

          static void prepareApiBasicTypes(ApiFile &apiFile);
          static void prepareApiBasicTypes(
                                           ApiFile &apiFile,
                                           const IEventingTypes::PredefinedTypedefs basicType
                                           );
          static void prepareApiBoxing(ApiFile &apiFile);
          static void prepareApiBoxing(
                                       ApiFile &apiFile,
                                       const IEventingTypes::PredefinedTypedefs basicType
                                       );

          static void prepareApiString(ApiFile &apiFile);
          static void prepareApiBinary(ApiFile &apiFile);

          static void prepareApiDuration(ApiFile &apiFile);
          static void prepareApiDuration(
                                         ApiFile &apiFile,
                                         const String &durationType
                                         );
          static void prepareApiList(
                                     ApiFile &apiFile,
                                     const String &listOrSetStr
                                     );
          static void prepareApiSpecial(
                                        ApiFile &apiFile,
                                        const String &specialName
                                        );
          static void preparePromiseWithValue(ApiFile &apiFile);
          static void preparePromiseWithRejectionReason(ApiFile &apiFile);

          static void prepareApiNamespace(
                                          ApiFile &apiFile,
                                          NamespacePtr namespaceObj
                                          );
          static void prepareApiStruct(
                                       ApiFile &apiFile,
                                       StructPtr structObj
                                       );
          static void prepareApiEnum(
                                     ApiFile &apiFile,
                                     EnumTypePtr enumObj
                                     );

          static void prepareInitFile(
                                      InitFile &initFile,
                                      NamespacePtr namespaceObj
                                      );
          static void finalizeInitFile(InitFile &initFile) { finalizeBaseFile(initFile); }

          static void prepareEnumFile(
                                      EnumFile &enumFile,
                                      NamespacePtr namespaceObj
                                      );
          static void finalizeEnumFile(EnumFile &enumFile) { finalizeBaseFile(enumFile); }

          static void prepareEnumNamespace(
                                           EnumFile &enumFile,
                                           NamespacePtr namespaceObj
                                           );

          static void processNamespace(
                                       const String &pathStr,
                                       SetupFile &setupFile,
                                       ApiFile &apiFile,
                                       NamespacePtr namespaceObj
                                       );
          static void processStruct(
                                    const String &pathStr,
                                    ApiFile &apiFile,
                                    InitFile &initFile,
                                    StructPtr structObj
                                    );
          static void processInheritance(
                                         ApiFile &apiFile,
                                         StructFile &structFile,
                                         StructFile &interfaceFile,
                                         StructPtr rootStructObj,
                                         StructPtr structObj,
                                         bool &first
                                         );
          static void processStruct(
                                    const String &pathStr,
                                    ApiFile &apiFile,
                                    InitFile &initFile,
                                    StructFile &structFile,
                                    StructFile &interfaceFile,
                                    StructPtr rootStructObj,
                                    StructPtr structObj
                                    );
          static void processEnum(
                                  ApiFile &apiFile,
                                  StructFile &structFile,
                                  StructPtr structObj,
                                  EnumTypePtr enumObj
                                  );
          static void processMethods(
                                     ApiFile &apiFile,
                                     StructFile &structFile,
                                     StructFile &interfaceFile,
                                     StructPtr rootStructObj,
                                     StructPtr structObj
                                     );
          static void processProperties(
                                        ApiFile &apiFile,
                                        StructFile &structFile,
                                        StructFile &interfaceFile,
                                        StructPtr rootStructObj,
                                        StructPtr structObj
                                        );
          static void processEventHandlers(
                                           ApiFile &apiFile,
                                           StructFile &structFile,
                                           StructFile &interfaceFile,
                                           StructPtr structObj
                                           );
          static void processEventHandlersStart(
                                                ApiFile &apiFile,
                                                StructFile &structFile,
                                                StructFile &interfaceFile,
                                                StructPtr structObj
                                                );
          static void processEventHandlersEnd(
                                              ApiFile &apiFile,
                                              StructFile &structFile,
                                              StructFile &interfaceFile,
                                              StructPtr structObj
                                              );

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark GenerateStructC::IIDLCompilerTarget
          #pragma mark

          //-------------------------------------------------------------------
          String targetKeyword() override;
          String targetKeywordHelp() override;
          void targetOutput(
                            const String &inPathStr,
                            const ICompilerTypes::Config &config
                            ) throw (Failure) override;
        };
         
      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
