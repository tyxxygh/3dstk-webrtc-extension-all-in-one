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
        #pragma mark GenerateStructC
        #pragma mark

        struct GenerateStructC : public IIDLCompilerTarget,
                                  public IDLCompiler
        {
          typedef String NamePath;
          typedef std::set<String> StringSet;
          typedef std::set<StructPtr> StructSet;
          typedef std::map<NamePath, StructSet> NamePathStructSetMap;

          struct HelperFile
          {
            NamespacePtr global_;
            NamePathStructSetMap derives_;
            StringSet boxings_;

            String headerFileName_;
            String cppFileName_;

            std::stringstream headerCIncludeSS_;
            std::stringstream headerCFunctionsSS_;
            std::stringstream headerCppIncludeSS_;
            std::stringstream headerCppFunctionsSS_;

            std::stringstream cIncludeSS_;
            std::stringstream cFunctionsSS_;

            std::stringstream cppIncludeSS_;
            std::stringstream cppFunctionsSS_;

            StringSet headerCAlreadyIncluded_;
            StringSet headerCppAlreadyIncluded_;
            StringSet cAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            HelperFile();
            ~HelperFile();

            void headerIncludeC(const String &headerFile);
            void headerIncludeCpp(const String &headerFile);
            void includeC(const String &headerFile);
            void includeCpp(const String &headerFile);
            bool hasBoxing(const String &namePathStr);
          };

          struct StructFile
          {
            StructPtr struct_;

            String headerFileName_;
            String cppFileName_;

            std::stringstream headerCIncludeSS_;
            std::stringstream headerCFunctionsSS_;
            std::stringstream headerCppIncludeSS_;
            std::stringstream headerCppFunctionsSS_;

            std::stringstream cIncludeSS_;
            std::stringstream cFunctionsSS_;
            std::stringstream cppIncludeSS_;
            std::stringstream cppFunctionsSS_;

            StringSet headerCAlreadyIncluded_;
            StringSet headerCppAlreadyIncluded_;
            StringSet cAlreadyIncluded_;
            StringSet cppAlreadyIncluded_;

            StructFile();
            ~StructFile();

            void headerIncludeC(const String &headerFile);
            void headerIncludeCpp(const String &headerFile);
            void includeC(const String &headerFile);
            void includeCpp(const String &headerFile);
          };

          GenerateStructC();

          static GenerateStructCPtr create();

          static String fixBasicType(IEventingTypes::PredefinedTypedefs type);
          static String fixCType(IEventingTypes::PredefinedTypedefs type);
          static String fixCType(TypePtr type);
          static String fixCType(
                                 bool isOptional,
                                 TypePtr type
                                 );
          static String fixType(TypePtr type);
          static String getApiImplementationDefine(ContextPtr context);
          static String getApiCastRequiredDefine(ContextPtr context);
          static String getApiExportDefine(ContextPtr context);
          static String getApiExportCastedDefine(ContextPtr context);
          static String getApiCallingDefine(ContextPtr context);
          static String getApiGuardDefine(
                                          ContextPtr context,
                                          bool endGuard = false
                                          );
          static String getToHandleMethod(
                                          bool isOptional, 
                                          TypePtr type
                                          );
          static String getFromHandleMethod(
                                            bool isOptional,
                                            TypePtr type
                                            );

          static void includeType(
                                  HelperFile &helperFile,
                                  TypePtr type
                                  );
          static void includeType(
                                  StructFile &structFile,
                                  TypePtr type
                                  );

          static void calculateRelations(
                                         NamespacePtr namespaceObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         );
          static void calculateRelations(
                                         StructPtr structObj,
                                         NamePathStructSetMap &ioDerivesInfo
                                         );

          static void calculateBoxings(
                                       NamespacePtr namespaceObj,
                                       StringSet &ioBoxings
                                       );
          static void calculateBoxings(
                                       StructPtr structObj,
                                       StringSet &ioBoxings
                                       );
          static void calculateBoxings(
                                       TemplatedStructTypePtr templatedStructObj,
                                       StringSet &ioBoxings
                                       );
          static void calculateBoxings(
                                       MethodPtr method,
                                       StringSet &ioBoxings,
                                       TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                       );
          static void calculateBoxings(
                                       PropertyPtr property,
                                       StringSet &ioBoxings,
                                       TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                       );
          static void calculateBoxingType(
                                          bool isOptional,
                                          TypePtr type,
                                          StringSet &ioBoxings,
                                          TemplatedStructTypePtr templatedStruct = TemplatedStructTypePtr()
                                          );

          static void insertInto(
                                 StructPtr structObj,
                                 const NamePath &namePath,
                                 NamePathStructSetMap &ioDerivesInfo
                                 );

          static void appendStream(
                                   std::stringstream &output,
                                   std::stringstream &source,
                                   bool appendEol = true
                                   );

          static void prepareHelperFile(HelperFile &helperFile);
          static void prepareHelperCallback(HelperFile &helperFile);
          static void prepareHelperExceptions(HelperFile &helperFile);
          static void prepareHelperExceptions(
                                              HelperFile &helperFile,
                                              const String &exceptionName
                                              );
          static void prepareHelperBoxing(HelperFile &helperFile);
          static void prepareHelperBoxing(
                                          HelperFile &helperFile,
                                          const IEventingTypes::PredefinedTypedefs basicType
                                          );
          static void prepareHelperNamespace(
                                             HelperFile &helperFile,
                                             NamespacePtr namespaceObj
                                             );
          static void prepareHelperStruct(
                                          HelperFile &helperFile,
                                          StructPtr structObj
                                          );
          static void prepareHelperEnum(
                                        HelperFile &helperFile,
                                        EnumTypePtr enumObj
                                        );
          static void prepareHelperEnumBoxing(
                                              HelperFile &helperFile,
                                              EnumTypePtr enumObj
                                              );
          static void prepareHelperString(HelperFile &helperFile);
          static void prepareHelperBinary(HelperFile &helperFile);
          static void prepareHelperDuration(
                                            HelperFile &helperFile,
                                            const String &durationType
                                            );
          static void prepareHelperList(
                                        HelperFile &helperFile,
                                        const String &listOrSetStr
                                        );
          static void prepareHelperSpecial(
                                           HelperFile &helperFile,
                                           const String &specialName
                                           );
          static void preparePromiseWithValue(HelperFile &helperFile);
          static void preparePromiseWithRejectionReason(HelperFile &helperFile);

          static void finalizeHelperFile(HelperFile &helperFile);

          static void processNamespace(
                                       HelperFile &helperFile,
                                       NamespacePtr namespaceObj
                                       );
          static void processStruct(
                                    HelperFile &helperFile,
                                    StructPtr structObj
                                    );
          static void processStruct(
                                    HelperFile &helperFile,
                                    StructFile &structFile,
                                    StructPtr rootStructObj,
                                    StructPtr structObj
                                    );
          static void processMethods(
                                     HelperFile &helperFile,
                                     StructFile &structFile,
                                     StructPtr rootStructObj,
                                     StructPtr structObj
                                     );
          static void processProperties(
                                        HelperFile &helperFile,
                                        StructFile &structFile,
                                        StructPtr rootStructObj,
                                        StructPtr structObj
                                        );
          static void processEventHandlers(
                                           HelperFile &helperFile,
                                           StructFile &structFile,
                                           StructPtr structObj
                                           );
          static void processEventHandlersStart(
                                                HelperFile &helperFile,
                                                StructFile &structFile,
                                                StructPtr structObj
                                                );
          static void processEventHandlersEnd(
                                              HelperFile &helperFile,
                                              StructFile &structFile,
                                              StructPtr structObj
                                              );

          static SecureByteBlockPtr generateTypesHeader(ProjectPtr project) throw (Failure);
          
          static void processTypesNamespace(
                                            std::stringstream &ss,
                                            NamespacePtr namespaceObj
                                            );
          static void processTypesStruct(
                                         std::stringstream &ss,
                                         StructPtr structObj
                                         );
          static void processTypesEnum(
                                       std::stringstream &ss,
                                       ContextPtr context
                                       );
          static void processTypesTemplatesAndSpecials(
                                                       std::stringstream &ss,
                                                       ProjectPtr project
                                                       );
          static void processTypesTemplate(
                                           std::stringstream &ss,
                                           ContextPtr structContextObj
                                           );
          static void processTypesSpecialStruct(
                                                std::stringstream &ss,
                                                ContextPtr structContextObj
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
