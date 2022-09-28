/*

Copyright (c) 2016, Robin Raymond
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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructC.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateHelper.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateTypesHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructHeader.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>

#include <zsLib/eventing/tool/OutputStream.h>

#include <sstream>

#define ZS_WRAPPER_COMPILER_DIRECTIVE_EXCLUSIZE "EXCLUSIVE"

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IIDLTypes::Project, Project);

    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::tool::internal::Helper, UseHelper);
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHasher, UseHasher);
      typedef std::set<String> HashSet;

      namespace internal
      {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark (helpers)
        #pragma mark
        //---------------------------------------------------------------------
        static void doInclude(
                              const String &headerFile,
                              std::stringstream &ss,
                              GenerateStructC::StringSet &alreadyIncluded
                              )
        {
          if (alreadyIncluded.end() != alreadyIncluded.find(headerFile)) return;
          alreadyIncluded.insert(headerFile);

          ss << "#include " << headerFile << "\n";
        }
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC::HelperFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructC::HelperFile::HelperFile()
        {
        }
        
        //---------------------------------------------------------------------
        GenerateStructC::HelperFile::~HelperFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::headerIncludeC(const String &headerFile)
        {
          doInclude(headerFile, headerCIncludeSS_, headerCAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::headerIncludeCpp(const String &headerFile)
        {
          doInclude(headerFile, headerCppIncludeSS_, headerCppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::includeC(const String &headerFile)
        {
          doInclude(headerFile, cIncludeSS_, cAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::HelperFile::includeCpp(const String &headerFile)
        {
          doInclude(headerFile, cppIncludeSS_, cppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        bool GenerateStructC::HelperFile::hasBoxing(const String &namePathStr)
        {
          auto found = boxings_.find(namePathStr);
          return found != boxings_.end();
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructC::StructFile::StructFile()
        {
        }
        
        //---------------------------------------------------------------------
        GenerateStructC::StructFile::~StructFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::headerIncludeC(const String &headerFile)
        {
          doInclude(headerFile, headerCIncludeSS_, headerCAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::headerIncludeCpp(const String &headerFile)
        {
          doInclude(headerFile, headerCppIncludeSS_, headerCppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::includeC(const String &headerFile)
        {
          doInclude(headerFile, cIncludeSS_, cAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::StructFile::includeCpp(const String &headerFile)
        {
          doInclude(headerFile, cppIncludeSS_, cppAlreadyIncluded_);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructC
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructC::GenerateStructC() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructCPtr GenerateStructC::create()
        {
          return make_shared<GenerateStructC>();
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixBasicType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        
            case PredefinedTypedef_bool:        
            case PredefinedTypedef_uchar:       break;
            case PredefinedTypedef_char:        type = PredefinedTypedef_schar; break;
            case PredefinedTypedef_schar:       
            case PredefinedTypedef_ushort:      break;
            case PredefinedTypedef_short:       type = PredefinedTypedef_sshort; break;
            case PredefinedTypedef_sshort:      
            case PredefinedTypedef_uint:        break;
            case PredefinedTypedef_int:         type = PredefinedTypedef_sint; break;
            case PredefinedTypedef_sint:        
            case PredefinedTypedef_ulong:       break;
            case PredefinedTypedef_long:        type = PredefinedTypedef_slong; break;
            case PredefinedTypedef_slong:       
            case PredefinedTypedef_ulonglong:   break;
            case PredefinedTypedef_longlong:    type = PredefinedTypedef_slonglong; break;
            case PredefinedTypedef_slonglong:   
            case PredefinedTypedef_uint8:       
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       
            case PredefinedTypedef_uint16:      
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      
            case PredefinedTypedef_uint32:      
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      
            case PredefinedTypedef_uint64:      
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      

            case PredefinedTypedef_byte:        
            case PredefinedTypedef_word:        
            case PredefinedTypedef_dword:       
            case PredefinedTypedef_qword:       

            case PredefinedTypedef_float:       
            case PredefinedTypedef_double:      
            case PredefinedTypedef_ldouble:     
            case PredefinedTypedef_float32:     
            case PredefinedTypedef_float64:     

            case PredefinedTypedef_pointer:     

            case PredefinedTypedef_binary:      
            case PredefinedTypedef_size:        

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     break;
          }
          String result = GenerateHelper::getBasicTypeString(type);
          result.replaceAll(" ", "_");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixCType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_void:      return "void";

            case PredefinedTypedef_bool:      return "bool_t";

            case PredefinedTypedef_uchar:     return "uchar_t";
            case PredefinedTypedef_char:
            case PredefinedTypedef_schar:     return "schar_t";
            case PredefinedTypedef_ushort:    return "ushort_t";
            case PredefinedTypedef_short:
            case PredefinedTypedef_sshort:    return "sshort_t";
            case PredefinedTypedef_uint:      return "uint_t";
            case PredefinedTypedef_int:
            case PredefinedTypedef_sint:      return "sint_t";
            case PredefinedTypedef_ulong:     return "ulong_t";
            case PredefinedTypedef_long:
            case PredefinedTypedef_slong:     return "slong_t";
            case PredefinedTypedef_ulonglong: return "ullong_t";
            case PredefinedTypedef_longlong:
            case PredefinedTypedef_slonglong: return "sllong_t";
            case PredefinedTypedef_uint8:     return "uint8_t";
            case PredefinedTypedef_int8:
            case PredefinedTypedef_sint8:     return "int8_t";
            case PredefinedTypedef_uint16:    return "uint16_t";
            case PredefinedTypedef_int16:
            case PredefinedTypedef_sint16:    return "int16_t";
            case PredefinedTypedef_uint32:    return "uint32_t";
            case PredefinedTypedef_int32:
            case PredefinedTypedef_sint32:    return "int32_t";
            case PredefinedTypedef_uint64:    return "uint64_t";
            case PredefinedTypedef_int64:
            case PredefinedTypedef_sint64:    return "int64_t";

            case PredefinedTypedef_byte:      return "uint8_t";
            case PredefinedTypedef_word:      return "uint16_t";
            case PredefinedTypedef_dword:     return "uint32_t";
            case PredefinedTypedef_qword:     return "uint64_t";

            case PredefinedTypedef_float:     return "float_t";
            case PredefinedTypedef_double:    return "double_t";
            case PredefinedTypedef_ldouble:   return "ldouble_t";
            case PredefinedTypedef_float32:   return "float32_t";
            case PredefinedTypedef_float64:   return "float64_t";

            case PredefinedTypedef_pointer:   return "raw_pointer_t";

            case PredefinedTypedef_binary:    return "binary_t";
            case PredefinedTypedef_size:      return "binary_size_t";

            case PredefinedTypedef_string:
            case PredefinedTypedef_astring:
            case PredefinedTypedef_wstring:   return "string_t";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixCType(TypePtr type)
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return fixCType(basicType->mBaseType);
            }
          }

          auto result = fixType(type);
          return result + "_t";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixCType(
                                         bool isOptional,
                                         TypePtr type
                                         )
        {
          if (!isOptional) return fixCType(type);
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return String("box_") + fixCType(basicType->mBaseType);
            }
          }
          {
            auto enumObj = type->toEnumType();
            if (enumObj) {
              return String("box_") + fixCType(enumObj);
            }
          }
          return fixCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructC::fixType(TypePtr type)
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              return fixCType(basicType->mBaseType);
            }
          }

          {
            auto templateType = type->toTemplatedStructType();
            if (templateType) {
              auto result = fixType(templateType->getParentStruct());
              for (auto iter = templateType->mTemplateArguments.begin(); iter != templateType->mTemplateArguments.end(); ++iter) {
                auto typeArgument = (*iter);
                String temp = fixType(typeArgument);
                if (temp.hasData()) {
                  if (result.hasData()) {
                    result += "_";
                  }
                  result += temp;
                }
              }
              return result;
            }
          }

          auto result = type->getPathName();
          if ("::" == result.substr(0, 2)) {
            result = result.substr(2);
          }
          result.replaceAll("::", "_");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiImplementationDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_GENERATED_IMPLEMENTATION";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_IMPLEMENTATION";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiCastRequiredDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_GENERATED_REQUIRES_CAST";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_REQUIRES_CAST";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiExportDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_EXPORT_API";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_EXPORT_API";
        }


        //---------------------------------------------------------------------
        String GenerateStructC::getApiExportCastedDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_CASTED_EXPORT_API";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_CASTED_EXPORT_API";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiCallingDefine(ContextPtr context)
        {
          String result = "WRAPPER_C_CALLING_CONVENTION";
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + "_WRAPPER_C_CALLING_CONVENTION";
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getApiGuardDefine(
                                                  ContextPtr context,
                                                  bool endGuard
                                                  )
        {
          String result = (!endGuard ? "WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "WRAPPER_C_PLUS_PLUS_END_GUARD");
          if (!context) return result;
          auto project = context->getProject();
          if (!project) return result;

          if (project->mName.isEmpty()) return result;

          auto name = project->mName;
          name.toUpper();
          return name + (!endGuard ? "_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD" : "_WRAPPER_C_PLUS_PLUS_END_GUARD");
        }
        
        //---------------------------------------------------------------------
        String GenerateStructC::getToHandleMethod(
                                                  bool isOptional,
                                                  TypePtr type
                                                  )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              if (isOptional) {
                return "wrapper::box_" + fixBasicType(basicType->mBaseType) + "_wrapperToHandle";
              }
              String cTypeStr = fixCType(basicType);
              if ("string_t" == cTypeStr) return "wrapper::string_t_wrapperToHandle";
              if ("binary_t" == cTypeStr) return "wrapper::binary_t_wrapperToHandle";
              return String();
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              if (isOptional) {
                return "box_" + fixCType(enumType) + "_wrapperToHandle";
              }
              return String("static_cast<") + fixCType(enumType->mBaseType) + ">";
            }
          }
          {
            if (GenerateHelper::isBuiltInType(type)) {
              auto structObj = type->toStruct();
              if (!structObj) {
                auto templatedStructObj = type->toTemplatedStructType();
                if (templatedStructObj) {
                  auto parentObj = templatedStructObj->getParent();
                  if (parentObj) structObj = parentObj->toStruct();
                }
              }

              if (!structObj) return String();

              String specialName = structObj->getPathName();

              if ("::zs::Any" == specialName) return "wrapper::zs_Any_wrapperToHandle";
              if ("::zs::Promise" == specialName) return "wrapper::zs_Promise_wrapperToHandle";
              if ("::zs::PromiseWith" == specialName) return "wrapper::zs_Promise_wrapperToHandle";
              if ("::zs::PromiseRejectionReason" == specialName) return String();
              if ("::zs::exceptions::Exception" == specialName) return "wrapper::exception_Exception_wrapperToHandle";
              if ("::zs::exceptions::InvalidArgument" == specialName) return "wrapper::exception_InvalidArgument_wrapperToHandle";
              if ("::zs::exceptions::BadState" == specialName) return "wrapper::exception_BadState_wrapperToHandle";
              if ("::zs::exceptions::NotImplemented" == specialName) return "wrapper::exception_NotImplemented_wrapperToHandle";
              if ("::zs::exceptions::NotSupported" == specialName) return "wrapper::exception_NotSupported_wrapperToHandle";
              if ("::zs::exceptions::UnexpectedError" == specialName) return "wrapper::exception_UnexpectedError_wrapperToHandle";
              if ("::zs::Time" == specialName) return "wrapper::zs_Time_wrapperToHandle";
              if ("::zs::Milliseconds" == specialName) return "wrapper::zs_Milliseconds_wrapperToHandle";
              if ("::zs::Microseconds" == specialName) return "wrapper::zs_Microseconds_wrapperToHandle";
              if ("::zs::Nanoseconds" == specialName) return "wrapper::zs_Nanoseconds_wrapperToHandle";
              if ("::zs::Seconds" == specialName) return "wrapper::zs_Seconds_wrapperToHandle";
              if ("::zs::Minutes" == specialName) return "wrapper::zs_Minutes_wrapperToHandle";
              if ("::zs::Hours" == specialName) return "wrapper::zs_Hours_wrapperToHandle";
              if ("::zs::Days" == specialName) return "wrapper::zs_Days_wrapperToHandle";
              if ("::std::set" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
              if ("::std::list" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
              if ("::std::map" == specialName) return String("wrapper::") + fixType(type) + "_wrapperToHandle";
            }
          }
          {
            auto structObj = type->toStruct();
            if (structObj) {
              return String("wrapper::") + fixType(structObj) + "_wrapperToHandle";
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructC::getFromHandleMethod(
                                                    bool isOptional,
                                                    TypePtr type
                                                    )
        {
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              if (!isOptional) {
                return String("static_cast<wrapper") + enumType->getPathName() + ">";
              }
            }
          }
          auto result = getToHandleMethod(isOptional, type);
          result.replaceAll("_wrapperToHandle", "_wrapperFromHandle");
          return result;
        }

        //---------------------------------------------------------------------
        void GenerateStructC::includeType(
                                          HelperFile &helperFile,
                                          TypePtr type
                                          )
        {
          if (!type) return;
          if (type->toBasicType()) return;
          if (type->toEnumType()) return;
          if (GenerateHelper::isBuiltInType(type)) return;
          
          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                auto subType = (*iter);
                includeType(helperFile, subType);
              }
              return;
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              String fileName = "\"c_" + fixType(type) + ".h\"";
              helperFile.includeC(fileName);
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::includeType(
                                          StructFile &structFile,
                                          TypePtr type
                                          )
        {
          if (!type) return;
          if (type->toBasicType()) return;
          if (type->toEnumType()) return;
          if (GenerateHelper::isBuiltInType(type)) return;

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              for (auto iter = templatedType->mTemplateArguments.begin(); iter != templatedType->mTemplateArguments.end(); ++iter) {
                auto subType = (*iter);
                includeType(structFile, subType);
              }
              return;
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              String fileName = "\"c_" + fixType(type) + ".h\"";
              structFile.includeC(fileName);
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateRelations(
                                                  NamespacePtr namespaceObj,
                                                  NamePathStructSetMap &ioDerivesInfo
                                                  )
        {
          if (!namespaceObj) return;
          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            calculateRelations(subNamespaceObj, ioDerivesInfo);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto structObj = (*iter).second;
            calculateRelations(structObj, ioDerivesInfo);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateRelations(
                                                 StructPtr structObj,
                                                 NamePathStructSetMap &ioDerivesInfo
                                                 )
        {
          if (!structObj) return;

          String currentNamePath = structObj->getPathName();

          StructSet allParents;
          allParents.insert(structObj);

          while (allParents.size() > 0)
          {
            auto top = allParents.begin();
            StructPtr parentStructObj = (*top);
            allParents.erase(top);
            
            if (structObj != parentStructObj) {
              insertInto(parentStructObj, currentNamePath, ioDerivesInfo);
            }
            insertInto(structObj, parentStructObj->getPathName(), ioDerivesInfo);

            for (auto iter = parentStructObj->mIsARelationships.begin(); iter != parentStructObj->mIsARelationships.end(); ++iter)
            {
              auto foundObj = (*iter).second;
              if (!foundObj) continue;
              auto foundStructObj = foundObj->toStruct();
              if (!foundStructObj) continue;
              allParents.insert(foundStructObj);
            }
          }

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto foundStruct = (*iter).second;
            calculateRelations(foundStruct, ioDerivesInfo);
          }
        }
        
        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxings(
                                               NamespacePtr namespaceObj,
                                               StringSet &ioBoxings
                                               )
        {
          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespace = (*iter).second;
            calculateBoxings(subNamespace, ioBoxings);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto subStruct = (*iter).second;
            calculateBoxings(subStruct, ioBoxings);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxings(
                                               StructPtr structObj,
                                               StringSet &ioBoxings
                                               )
        {
          if (!structObj) return;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter)
          {
            auto subStruct = (*iter).second;
            calculateBoxings(subStruct, ioBoxings);
          }

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter)
          {
            auto subTemplate = (*iter).second;
            calculateBoxings(subTemplate, ioBoxings);
          }

          if (structObj->mGenerics.size() > 0) return;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            calculateBoxings(method, ioBoxings);
          }

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter)
          {
            auto property = (*iter);
            calculateBoxings(property, ioBoxings);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxings(
                                               TemplatedStructTypePtr templatedStructObj,
                                               StringSet &ioBoxings
                                               )
        {
          if (!templatedStructObj) return;
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxings(
                                               MethodPtr method,
                                               StringSet &ioBoxings,
                                               TemplatedStructTypePtr templatedStruct
                                               )
        {
          if (!method) return;

          calculateBoxingType(method->hasModifier(Modifier_Optional), method->mResult, ioBoxings, templatedStruct);

          for (auto iter = method->mArguments.begin(); iter != method->mArguments.end(); ++iter)
          {
            auto arg = (*iter);
            calculateBoxings(arg, ioBoxings, templatedStruct);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxings(
                                               PropertyPtr property,
                                               StringSet &ioBoxings,
                                               TemplatedStructTypePtr templatedStruct
                                               )
        {
          if (!property) return;
          calculateBoxingType(property->hasModifier(Modifier_Optional), property->mType, ioBoxings, templatedStruct);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::calculateBoxingType(
                                                  bool isOptional,
                                                  TypePtr type,
                                                  StringSet &ioBoxings,
                                                  TemplatedStructTypePtr templatedStruct
                                                  )
        {
          if (!isOptional) return;
          if (!type) return;

          {
            auto genericType = type->toGenericType();
            if (genericType) {
              if (!templatedStruct) return;

              auto parentStruct = templatedStruct->getParentStruct();
              if (!parentStruct) return;

              String name = genericType->getMappingName();

              size_t index {};
              for (auto iter = parentStruct->mGenerics.begin(); iter != parentStruct->mGenerics.end(); ++iter, ++index)
              {
                auto structGeneric = (*iter);
                if (!structGeneric) continue;
                if (name == structGeneric->getMappingName()) break;
              }

              TypePtr checkType;
              for (auto iter = templatedStruct->mTemplateArguments.begin(); iter != templatedStruct->mTemplateArguments.end(); ++iter, --index)
              {
                auto foundType = (*iter);
                if (0 != index) continue;
                checkType = foundType;
                break;
              }

              if (!checkType) return;
              if (checkType->toGenericType()) return;

              type = checkType;
            }
          }

          String pathName;
          {
            auto basicType = type->toBasicType();
            if (basicType) {
              pathName = fixBasicType(basicType->mBaseType);
            } else {
              pathName = type->getPathName();
            }
          }
          ioBoxings.insert(pathName);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::insertInto(
                                          StructPtr structObj,
                                          const NamePath &namePath,
                                          NamePathStructSetMap &ioDerivesInfo
                                          )
        {
          if (!structObj) return;

          auto found = ioDerivesInfo.find(namePath);
          if (found == ioDerivesInfo.end()) {
            StructSet newSet;
            newSet.insert(structObj);
            ioDerivesInfo[namePath] = newSet;
            return;
          }

          auto &existingSet = (*found).second;
          existingSet.insert(structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::appendStream(
                                           std::stringstream &output,
                                           std::stringstream &source,
                                           bool appendEol
                                           )
        {
          String str = source.str();
          if (str.isEmpty()) return;

          if (appendEol) {
            output << "\n";
          }
          output << str;
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperFile(HelperFile &helperFile)
        {
          {
            auto &ss = helperFile.headerCIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "#pragma once\n\n";
            ss << "#include \"types.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << getApiGuardDefine(helperFile.global_) << "\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "\n";
            ss << "#include <zsLib/date.h>\n";
            ss << "#include \"c_helpers.h\"\n";
            ss << "#include <zsLib/types.h>\n";
            ss << "#include <zsLib/eventing/types.h>\n";
            ss << "#include <zsLib/SafeInt.h>\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;
            ss << "using namespace wrapper;\n\n";
            ss << "using namespace date;\n\n";
          }

          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "\n";
            ss << getApiGuardDefine(helperFile.global_, true) << "\n";
            ss << "\n";

            ss << "#ifdef __cplusplus\n";
            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;

            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          prepareHelperCallback(helperFile);
          prepareHelperExceptions(helperFile);
          prepareHelperBoxing(helperFile);
          prepareHelperString(helperFile);
          prepareHelperBinary(helperFile);

          prepareHelperDuration(helperFile, "Time");
          prepareHelperDuration(helperFile, "Days");
          prepareHelperDuration(helperFile, "Hours");
          prepareHelperDuration(helperFile, "Seconds");
          prepareHelperDuration(helperFile, "Minutes");
          prepareHelperDuration(helperFile, "Milliseconds");
          prepareHelperDuration(helperFile, "Microseconds");
          prepareHelperDuration(helperFile, "Nanoseconds");

          prepareHelperList(helperFile, "list");
          prepareHelperList(helperFile, "set");
          prepareHelperList(helperFile, "map");

          prepareHelperSpecial(helperFile, "Any");
          prepareHelperSpecial(helperFile, "Promise");
          preparePromiseWithValue(helperFile);
          preparePromiseWithRejectionReason(helperFile);

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }

          prepareHelperNamespace(helperFile, helperFile.global_);

          {
            auto &ss = helperFile.headerCppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
            ss << "#endif /* __cplusplus */\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperCallback(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
            ss << "/* void wrapperCallbackFunction(callback_event_t handle); */\n";
            ss << "typedef void (ORTC_WRAPPER_C_CALLING_CONVENTION *wrapperCallbackFunction)(callback_event_t);\n";
            ss << "\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperInstall(wrapperCallbackFunction function);\n";
            ss << "\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperObserverDestroy(event_observer_t handle);\n";
            ss << "\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " callback_event_wrapperDestroy(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " callback_event_wrapperInstanceId(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " event_observer_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_observer(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_namespace_actual(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_class_actual(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_method_actual(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " generic_handle_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_source(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_source_instance_id(callback_event_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " generic_handle_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_data(callback_event_t handle, int argumentIndex);\n";

            ss << "\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;
            ss << dash;
            ss << "static wrapperCallbackFunction &callback_get_singleton()\n";
            ss << "{\n";
            ss << "  static wrapperCallbackFunction function {};\n";
            ss << "  return function;\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperInstall(wrapperCallbackFunction function)\n";
            ss << "{\n";
            ss << "  callback_get_singleton() = function;\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void " << getApiCallingDefine(helperFile.global_) << " callback_wrapperObserverDestroy(event_observer_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperObserverPtr * IWrapperObserverPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  (*reinterpret_cast<IWrapperObserverPtrRawPtr>(handle))->observerCancel();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void " << getApiCallingDefine(helperFile.global_) << " callback_event_wrapperDestroy(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  delete reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle);\n";
            ss << "}\n";
            ss << "\n";

            ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " callback_event_wrapperInstanceId(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle)).get());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "event_observer_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_observer(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return static_cast<event_observer_t>(NULL);\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getObserver();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_namespace_actual(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<const_char_star_t>((*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getNamespace());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_class_actual(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<const_char_star_t>((*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getClass());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const_char_star_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_method_actual(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<const_char_star_t>((*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getMethod());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "generic_handle_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_source(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getSource();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_source_instance_id(callback_event_t handle)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getInstanceId();\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "generic_handle_t " << getApiCallingDefine(helperFile.global_) << " callback_event_get_data(callback_event_t handle, int argumentIndex)\n";
            ss << "{\n";
            ss << "  typedef IWrapperCallbackEventPtr * IWrapperCallbackEventPtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return (*reinterpret_cast<IWrapperCallbackEventPtrRawPtr>(handle))->getEventData(argumentIndex);\n";
            ss << "}\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;
            ss << dash2;
            ss << "  void IWrapperCallbackEvent::fireEvent(IWrapperCallbackEventPtr event)\n";
            ss << "  {\n";
            ss << "    if (!event) return;\n";
            ss << "    auto singleton = callback_get_singleton();\n";
            ss << "    if (!singleton) return;\n";
            ss << "    uintptr_t handle = reinterpret_cast<uintptr_t>(new IWrapperCallbackEventPtr(event));\n";
            ss << "    singleton(handle);\n";
            ss << "  }\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperExceptions(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << getApiExportDefine(helperFile.global_) << " exception_handle_t " << getApiCallingDefine(helperFile.global_) << " exception_wrapperCreate_exception();\n";
            ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " exception_wrapperDestroy(exception_handle_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " exception_wrapperInstanceId(exception_handle_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " bool " << getApiCallingDefine(helperFile.global_) << " exception_hasException(exception_handle_t handle);\n";
            ss << getApiExportDefine(helperFile.global_) << " const_char_star_t " << getApiCallingDefine(helperFile.global_) << " exception_what_actual(exception_handle_t handle);\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "  void exception_set_Exception(exception_handle_t handle, shared_ptr<::zsLib::Exception> exception);\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;
            ss << dash;
            ss << "exception_handle_t " << getApiCallingDefine(helperFile.global_) << " exception_wrapperCreate_exception()\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  return reinterpret_cast<exception_handle_t>(new ExceptionTypePtr());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "void " << getApiCallingDefine(helperFile.global_) << " exception_wrapperDestroy(exception_handle_t handle)\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return;\n";
            ss << "  delete reinterpret_cast<ExceptionTypePtrRawPtr>(handle);\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " exception_wrapperInstanceId(exception_handle_t handle)\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<ExceptionTypePtrRawPtr>(handle)).get());\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "bool " << getApiCallingDefine(helperFile.global_) << " exception_hasException(exception_handle_t handle)\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return false;\n";
            ss << "  return ((bool)(*reinterpret_cast<ExceptionTypePtrRawPtr>(handle)));\n";
            ss << "}\n";
            ss << "\n";

            ss << dash;
            ss << "const_char_star_t " << getApiCallingDefine(helperFile.global_) << " exception_what_actual(exception_handle_t handle)\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  return reinterpret_cast<const_char_star_t>((*reinterpret_cast<ExceptionTypePtrRawPtr>(handle))->what());\n";
            ss << "}\n";
            ss << "\n";
          }

          {
            auto &ss = helperFile.cppFunctionsSS_;
            ss << dash2;
            ss << "  void exception_set_Exception(exception_handle_t handle, shared_ptr<::zsLib::Exception> exception)\n";
            ss << "  {\n";
            ss << "    typedef ::zsLib::Exception ExceptionType;\n";
            ss << "    typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "    typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "    if (0 == handle) return;\n";
            ss << "    auto ptr = (*reinterpret_cast<ExceptionTypePtrRawPtr>(handle));\n";
            ss << "    ptr = exception;\n";
            ss << "  }\n";
            ss << "\n";
          }

          prepareHelperExceptions(helperFile, "Exception");
          prepareHelperExceptions(helperFile, "InvalidArgument");
          prepareHelperExceptions(helperFile, "BadState");
          prepareHelperExceptions(helperFile, "NotImplemented");
          prepareHelperExceptions(helperFile, "NotSupported");
          prepareHelperExceptions(helperFile, "UnexpectedError");

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }
          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperExceptions(
                                                      HelperFile &helperFile,
                                                      const String &exceptionName
                                                      )
        {
          auto context = helperFile.global_->toContext()->findType("::zs::exceptions::" + exceptionName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << getApiExportDefine(helperFile.global_) << " bool " << getApiCallingDefine(helperFile.global_) << " exception_is_" << exceptionName << "(exception_handle_t handle);\n";
          }
          {
            auto &ss = helperFile.cFunctionsSS_;

            ss << dash;
            ss << "bool " << getApiCallingDefine(helperFile.global_) << " exception_is_" << exceptionName << "(exception_handle_t handle)\n";
            ss << "{\n";
            ss << "  typedef ::zsLib::Exception ExceptionType;\n";
            ss << "  typedef shared_ptr<ExceptionType> ExceptionTypePtr;\n";
            ss << "  typedef ExceptionTypePtr * ExceptionTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return false;\n";
            ss << "  return ((bool)std::dynamic_pointer_cast<::zsLib::" << ("Exception" == exceptionName ? "" : "Exceptions::" ) << exceptionName << ">(*reinterpret_cast<ExceptionTypePtrRawPtr>(handle)));\n";
            ss << "}\n";
            ss << "\n";
          }
          {
            auto &ss = helperFile.headerCppFunctionsSS_;
            ss << "  exception_handle_t exception_" << exceptionName << "_wrapperToHandle(const ::zsLib::" << (exceptionName == "Exception" ? "" : "Exceptions::") << exceptionName << " &value);\n";
          }
          {
            auto &ss = helperFile.cppFunctionsSS_;

            ss << dash2;
            ss << "  exception_handle_t exception_" << exceptionName << "_wrapperToHandle(const ::zsLib::" << (exceptionName == "Exception" ? "" : "Exceptions::") << exceptionName << " &value)\n";
            ss << "  {\n";
            ss << "    typedef ::zsLib::" << (exceptionName == "Exception" ? "" : "Exceptions::") << exceptionName << " ExceptionType;\n";
            ss << "    auto handle = exception_wrapperCreate_exception();\n";
            ss << "    exception_set_Exception(handle, make_shared<ExceptionType>(value));\n";
            ss << "    return handle;\n";
            ss << "  }\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperBoxing(HelperFile &helperFile)
        {

          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_bool);

          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uchar);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_schar);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_ushort);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sshort);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uint);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sint);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_ulong);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_slong);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_ulonglong);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_slonglong);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uint8);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sint8);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uint16);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sint16);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uint32);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sint32);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_uint64);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_sint64);

          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_byte);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_word);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_dword);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_qword);

          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_float);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_double);
          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_ldouble);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_float32);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_float64);

          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_pointer);

          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_binary);
          //prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_size);

          prepareHelperBoxing(helperFile, IEventingTypes::PredefinedTypedef_string);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperBoxing(
                                                  HelperFile &helperFile,
                                                  const IEventingTypes::PredefinedTypedefs basicType
                                                  )
        {
          bool isBinary = IEventingTypes::PredefinedTypedef_binary == basicType;
          bool isString = IEventingTypes::PredefinedTypedef_string == basicType;

          String cTypeStr = fixCType(basicType);
          String boxedTypeStr = "box_" + cTypeStr;

          auto pathStr = fixBasicType(basicType);
          if (!helperFile.hasBoxing(pathStr)) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(helperFile.global_) << " " << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "();\n";
              ss << getApiExportDefine(helperFile.global_) << " " << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(" << cTypeStr << " value);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << "bool " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << cTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << "void " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_set_value(" << boxedTypeStr << " handle, " << cTypeStr << " value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  " << boxedTypeStr << " box_" << fixBasicType(basicType) << "_wrapperToHandle(Optional< " << GenerateHelper::getBasicTypeString(basicType) << " > value);\n";
              ss << "  Optional< " << GenerateHelper::getBasicTypeString(basicType) << " > box_" << fixBasicType(basicType) << "_wrapperFromHandle(" << boxedTypeStr << " handle);\n";
              ss << "\n";
            }
          }
          {
            String sharedPtrStr = "typedef shared_ptr< BasicType > BasicTypePtr;";
            if (isBinary) {
              sharedPtrStr = "typedef BasicType BasicTypePtr;";
            }
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "()\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  return reinterpret_cast<CBoxType>(new BasicTypePtr());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(" << cTypeStr << " value)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  auto ptr = new BasicTypePtr();\n";
              if (isBinary) {
                ss << "  (*ptr) = wrapper::binary_t_wrapperFromHandle(value);\n";
              } else if (isString) {
                ss << "  (*ptr) = make_shared<BasicType>();\n";
                ss << "  (*(*ptr)) = wrapper::string_t_wrapperFromHandle(value);\n";
              } else {
                ss << "  (*ptr) = make_shared<BasicType>();\n";
                ss << "  (*((*ptr).get())) = value;\n";
              }

              ss << "  return reinterpret_cast<CBoxType>(ptr);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<BasicTypePtrRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<BasicTypePtrRawPtr>(handle)).get());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "bool " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_has_value(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return false;\n";
              ss << "  BasicTypePtr ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              ss << "  return ((bool)ptr);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << cTypeStr << " " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_get_value(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << cTypeStr << " CType;\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return CType();\n";
              ss << "  BasicTypePtr ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              if (isBinary) {
                ss << "  return wrapper::binary_t_wrapperToHandle(ptr);\n";
              } else if (isString) {
                ss << "  return wrapper::string_t_wrapperToHandle(*ptr);\n";
              } else {
                ss << "  if (!ptr) return CType();\n";
                ss << "  return *ptr;\n";
              }
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " box_" << cTypeStr << "_set_value(" << boxedTypeStr << " handle, " << cTypeStr << " value)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "  " << sharedPtrStr << "\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  BasicTypePtr &ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              if (isBinary) {
                ss << "  ptr = wrapper::binary_t_wrapperFromHandle(value);\n";
              } else if (isString) {
                ss << "  ptr = make_shared<String>(wrapper::string_t_wrapperFromHandle(value));\n";
              } else {
                ss << "  if (!ptr) ptr = make_shared<BasicType>();\n";
                ss << "  (*(ptr.get())) = value;\n";
              }
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << boxedTypeStr << " box_" << fixBasicType(basicType) << "_wrapperToHandle(Optional< " << GenerateHelper::getBasicTypeString(basicType) << " > value)\n";
              ss << "  {\n";
              ss << "    if (!value.hasValue()) return box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "();\n";
              if (isBinary) {
                ss << "    return box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(binary_t_wrapperToHandle(value.value()));\n";
              } else if (isString) {
                ss << "    return box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(string_t_wrapperToHandle(value.value()));\n";
              } else {
                ss << "    return box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(value.value());\n";
              }
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  Optional< " << GenerateHelper::getBasicTypeString(basicType) << " > box_" << fixBasicType(basicType) << "_wrapperFromHandle(" << boxedTypeStr << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "    typedef " << GenerateHelper::getBasicTypeString(basicType) << " BasicType;\n";
              ss << "    " << sharedPtrStr << "\n";
              ss << "    typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "    if (0 == handle) return Optional< " << GenerateHelper::getBasicTypeString(basicType) << " >();\n";
              ss << "    if (!box_" << cTypeStr << "_has_value(handle)) return Optional< " << GenerateHelper::getBasicTypeString(basicType) << " >();\n";
              if (isBinary) {
                ss << "    return Optional< " << GenerateHelper::getBasicTypeString(basicType) << " >(binary_t_wrapperFromHandle(box_binary_t_get_value(handle)));\n";
              } else if (isString) {
                ss << "    return Optional< " << GenerateHelper::getBasicTypeString(basicType) << " >(string_t_wrapperFromHandle(box_string_t_get_value(handle)));\n";
              } else {
                ss << "    return Optional< " << GenerateHelper::getBasicTypeString(basicType) << " >(box_" << cTypeStr <<"_get_value(handle));\n";
              }
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperNamespace(
                                                     HelperFile &helperFile,
                                                     NamespacePtr namespaceObj
                                                     )
        {
          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            prepareHelperNamespace(helperFile, subNamespaceObj);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareHelperStruct(helperFile, subStructObj);
          }
          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareHelperEnum(helperFile, subEnumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperStruct(
                                                  HelperFile &helperFile,
                                                  StructPtr structObj
                                                  )
        {
          if (!structObj) return;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareHelperStruct(helperFile, subStructObj);
          }
          for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareHelperEnum(helperFile, subEnumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperEnum(
                                                HelperFile &helperFile,
                                                EnumTypePtr enumObj
                                                )
        {
          if (!enumObj) return;
          prepareHelperEnumBoxing(helperFile, enumObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperEnumBoxing(
                                                      HelperFile &helperFile,
                                                      EnumTypePtr enumObj
                                                      )
        {
          if (!enumObj) return;
          if (!helperFile.hasBoxing(enumObj->getPathName())) return;

          String cTypeStr = fixCType(enumObj);
          String boxedTypeStr = "box_" + cTypeStr;
          String wrapperTypeStr = "wrapper" + enumObj->getPathName();

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(helperFile.global_) << " " << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "();\n";
              ss << getApiExportDefine(helperFile.global_) << " " << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(" << cTypeStr << " value);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << "bool " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << cTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " " << "void " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_set_value(" << boxedTypeStr << " handle, " << cTypeStr << " value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  " << boxedTypeStr << " " << boxedTypeStr << "_wrapperToHandle(Optional< " << wrapperTypeStr << " > value);\n";
              ss << "  Optional< " << wrapperTypeStr << " > " << boxedTypeStr << "_wrapperFromHandle(" << boxedTypeStr << " handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "()\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  return reinterpret_cast<CBoxType>(new BasicTypePtr());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << boxedTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(" << cTypeStr << " value)\n";
              ss << "{\n";
              ss << "  typedef " << boxedTypeStr << " CBoxType;\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  auto ptr = new BasicTypePtr();\n";
              ss << "  (*ptr) = make_shared<BasicType>();\n";
              ss << "  (*((*ptr).get())) = static_cast<BasicType>(value);\n";
              ss << "  return reinterpret_cast<CBoxType>(ptr);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<BasicTypePtrRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<BasicTypePtrRawPtr>(handle)).get());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "bool " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_has_value(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return false;\n";
              ss << "  BasicTypePtr ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              ss << "  return ((bool)ptr);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << cTypeStr << " " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_get_value(" << boxedTypeStr << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << cTypeStr << " CType;\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return " << cTypeStr << "();\n";
              ss << "  BasicTypePtr ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              ss << "  if (!ptr) return CType();\n";
              ss << "  return static_cast<CType>(*ptr);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " " << boxedTypeStr << "_set_value(" << boxedTypeStr << " handle, " << cTypeStr << " value)\n";
              ss << "{\n";
              ss << "  typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "  typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "  typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  BasicTypePtr &ptr = (*reinterpret_cast<BasicTypePtrRawPtr>(handle));\n";
              ss << "  if (!ptr) ptr = make_shared<BasicType>();\n";
              ss << "  (*(ptr.get())) = static_cast<BasicType>(value);\n";
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << boxedTypeStr << " " << boxedTypeStr << "_wrapperToHandle(Optional< " << wrapperTypeStr << " > value)\n";
              ss << "  {\n";
              ss << "    typedef " << cTypeStr << " CType;\n";
              ss << "    if (!value.hasValue()) return " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "();\n";
              ss << "    return " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(static_cast<CType>(value.value()));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  Optional< " << wrapperTypeStr << " > " << boxedTypeStr << "_wrapperFromHandle(" << boxedTypeStr << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << wrapperTypeStr << " BasicType;\n";
              ss << "    typedef shared_ptr< BasicType > BasicTypePtr;\n";
              ss << "    typedef BasicTypePtr * BasicTypePtrRawPtr;\n";
              ss << "    if (0 == handle) return Optional< BasicType >();\n";
              ss << "    if (!" << boxedTypeStr << "_has_value(handle)) return Optional< BasicType >();\n";
              ss << "    return Optional< BasicType >(static_cast<BasicType>(" << boxedTypeStr << "_get_value(handle)));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperString(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_string();\n";
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_stringWithValue(const char *value);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperDestroy(string_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperInstanceId(string_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " const_char_star_t " << getApiCallingDefine(helperFile.global_) << " string_t_get_value_actual(string_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " string_t_set_value(string_t handle, const char *value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  string_t string_t_wrapperToHandle(const ::std::string &value);\n";
              ss << "  ::zsLib::String string_t_wrapperFromHandle(string_t handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << getApiExportDefine(helperFile.global_) << " string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_string()\n";
              ss << "{\n";
              ss << "  return reinterpret_cast<string_t>(new ::zsLib::String());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "string_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperCreate_stringWithValue(const char *value)\n";
              ss << "{\n";
              ss << "  return reinterpret_cast<string_t>(new ::zsLib::String(value));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperDestroy(string_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<::zsLib::String *>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " string_t_wrapperInstanceId(string_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>(reinterpret_cast<::zsLib::String *>(handle));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "const_char_star_t " << getApiCallingDefine(helperFile.global_) << " string_t_get_value_actual(string_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<const_char_star_t>((*reinterpret_cast<::zsLib::String *>(handle)).c_str());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " string_t_set_value(string_t handle, const char *value)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  (*reinterpret_cast<::zsLib::String *>(handle)) = ::zsLib::String(value);\n";
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  string_t string_t_wrapperToHandle(const ::std::string &value)\n";
              ss << "  {\n";
              ss << "    return reinterpret_cast<string_t>(new ::zsLib::String(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  ::zsLib::String string_t_wrapperFromHandle(string_t handle)\n";
              ss << "  {\n";
              ss << "    if (0 == handle) return ::zsLib::String();\n";
              ss << "    return (*reinterpret_cast<::zsLib::String *>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperBinary(HelperFile &helperFile)
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(helperFile.global_) << " binary_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperCreate_binary_t();\n";
              ss << getApiExportDefine(helperFile.global_) << " binary_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperCreate_binary_tWithValue(const uint8_t *value, binary_size_t size);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperDestroy(binary_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " instance_id_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperInstanceId(binary_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " const uint8_t * " << getApiCallingDefine(helperFile.global_) << " binary_t_get_value(binary_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " binary_size_t " << getApiCallingDefine(helperFile.global_) << " binary_t_get_size(binary_t handle);\n";
              ss << getApiExportDefine(helperFile.global_) << " void " << getApiCallingDefine(helperFile.global_) << " binary_t_set_value(binary_t handle, const uint8_t *value, binary_size_t size);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  binary_t binary_t_wrapperToHandle(SecureByteBlockPtr value);\n";
              ss << "  SecureByteBlockPtr binary_t_wrapperFromHandle(binary_t handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << "binary_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperCreate_binary_t()\n";
              ss << "{\n";
              ss << "  return reinterpret_cast<binary_t>(new SecureByteBlockPtr());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "binary_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperCreate_binary_tWithValue(const uint8_t *value, binary_size_t size)\n";
              ss << "{\n";
              ss << "  if ((NULL == value) || (0 == size)) return reinterpret_cast<binary_t>(new SecureByteBlockPtr());\n";
              ss << "  return reinterpret_cast<binary_t>(new SecureByteBlockPtr(make_shared<SecureByteBlock>(value, SafeInt<SecureByteBlock::size_type>(size))));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperDestroy(binary_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<SecureByteBlockPtr *>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "instance_id_t " << getApiCallingDefine(helperFile.global_) << " binary_t_wrapperInstanceId(binary_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<SecureByteBlockPtr *>(handle)).get());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "const uint8_t * " << getApiCallingDefine(helperFile.global_) << " binary_t_get_value(binary_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return static_cast<const uint8_t *>(NULL);\n";
              ss << "  SecureByteBlockPtr ptr = (*reinterpret_cast<SecureByteBlockPtr *>(handle));\n";
              ss << "  if (!ptr) return static_cast<const uint8_t *>(NULL);\n";
              ss << "  return ptr->BytePtr();\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "binary_size_t " << getApiCallingDefine(helperFile.global_) << " binary_t_get_size(binary_t handle)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  SecureByteBlockPtr ptr = (*reinterpret_cast<SecureByteBlockPtr *>(handle));\n";
              ss << "  if (!ptr) return 0;\n";
              ss << "  return SafeInt<size_t>(ptr->SizeInBytes());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(helperFile.global_) << " binary_t_set_value(binary_t handle, const uint8_t *value, binary_size_t size)\n";
              ss << "{\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  if ((NULL == value) || (0 == size)) { (*reinterpret_cast<SecureByteBlockPtr *>(handle)) = SecureByteBlockPtr(); return; }\n";
              ss << "  (*reinterpret_cast<SecureByteBlockPtr *>(handle)) = make_shared<SecureByteBlock>(value, SafeInt<SecureByteBlock::size_type>(size));\n";
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  binary_t binary_t_wrapperToHandle(SecureByteBlockPtr value)\n";
              ss << "  {\n";
              ss << "    return reinterpret_cast<binary_t>(new SecureByteBlockPtr(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  SecureByteBlockPtr binary_t_wrapperFromHandle(binary_t handle)\n";
              ss << "  {\n";
              ss << "    if (0 == handle) return SecureByteBlockPtr();\n";
              ss << "    return (*reinterpret_cast<SecureByteBlockPtr *>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperDuration(
                                                    HelperFile &helperFile,
                                                    const String &durationType
                                                    )
        {
          bool isTime = "Time" == durationType;

          auto durationContext = helperFile.global_->toContext()->findType("::zs::" + durationType);
          if (!durationContext) return;

          auto durationStruct = durationContext->toStruct();
          if (!durationStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));
          String zsDurationType = "::zsLib::" + durationType;

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(durationStruct) << " " << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "();\n";
              ss << getApiExportDefine(durationStruct) << " " << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(int64_t value);\n";
              ss << getApiExportDefine(durationStruct) << " void " << getApiCallingDefine(durationStruct) <<" zs_" << durationType << "_wrapperDestroy(" << fixCType(durationStruct) << " handle);\n";
              ss << getApiExportDefine(durationStruct) << " instance_id_t " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperInstanceId(" << fixCType(durationStruct) << " handle);\n";
              ss << getApiExportDefine(durationStruct) << " " << "int64_t " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_get_value(" << fixCType(durationStruct) << " handle);\n";
              ss << getApiExportDefine(durationStruct) << " " << "void " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_set_value(" << fixCType(durationStruct) << " handle, int64_t value);\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              ss << "  " << fixCType(durationStruct) << " zs_" << durationType << "_wrapperToHandle(" << zsDurationType << " value);\n";
              ss << "  " << zsDurationType << " zs_" << durationType << "_wrapperFromHandle(" << fixCType(durationStruct) << " handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;
              ss << dash;
              ss << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "()\n";
              ss << "{\n";
              ss << "  typedef " << fixCType(durationStruct) << " CType;\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  return reinterpret_cast<CType>(new DurationType());\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << fixCType(durationStruct) << " " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(int64_t value)\n";
              ss << "{\n";
              if (isTime) {
                ss << "  auto result = zs_" << durationType << "_wrapperCreate_" << durationType << "();\n";
                ss << "  zs_" << durationType << "_set_value(result, value);\n";
                ss << "  return result;\n";
              } else {
                ss << "  typedef " << fixCType(durationStruct) << " CType;\n";
                ss << "  typedef " << zsDurationType << " DurationType;\n";
                ss << "  typedef DurationType::rep DurationTypeRep;\n";
                ss << "  return reinterpret_cast<CType>(new DurationType(static_cast<DurationTypeRep>(SafeInt<DurationTypeRep>(value))));\n";
              }
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperDestroy(" << fixCType(durationStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<DurationTypeRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "instance_id_t " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_wrapperInstanceId(" << fixCType(durationStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>(reinterpret_cast<DurationTypeRawPtr>(handle));\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "int64_t " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_get_value(" << fixCType(durationStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              if (isTime) {
                ss << "  auto t = day_point(jan / 1 / 1601);\n";
                ss << "  auto diff = (*reinterpret_cast<DurationTypeRawPtr>(handle)) - t;\n";
                ss << "  auto nano = ::zsLib::toNanoseconds(diff);\n";
                ss << "  return SafeInt<int64_t>(nano.count() / static_cast<::zsLib::Nanoseconds::rep>(100));\n";
              } else {
                ss << "  return SafeInt<int64_t>(reinterpret_cast<DurationTypeRawPtr>(handle)->count());\n";
              }
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "void " << getApiCallingDefine(durationStruct) << " zs_" << durationType << "_set_value(" << fixCType(durationStruct) << " handle, int64_t value)\n";
              ss << "{\n";
              ss << "  typedef " << zsDurationType << " DurationType;\n";
              ss << "  typedef DurationType * DurationTypeRawPtr;\n";
              ss << "  typedef DurationType::rep DurationTypeRep;\n";
              ss << "  if (0 == handle) return;\n";
              if (isTime) {
                ss << "  ::zsLib::Time t = day_point(jan / 1 / 1601);\n";
                ss << "  auto nano = std::chrono::duration_cast<::zsLib::Time::duration>(zsLib::Nanoseconds(static_cast<::zsLib::Nanoseconds::rep>(value) * static_cast<::zsLib::Nanoseconds::rep>(100)));\n";
                ss << "  (*reinterpret_cast<DurationTypeRawPtr>(handle)) = ::zsLib::Time(t + nano);\n";
              } else {
                ss << "  (*reinterpret_cast<DurationTypeRawPtr>(handle)) = DurationType(SafeInt<DurationTypeRep>(value));\n";
              }
              ss << "}\n";
              ss << "\n";
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << fixCType(durationStruct) << " zs_" << durationType << "_wrapperToHandle(" << zsDurationType << " value)\n";
              ss << "  {\n";
              ss << "    typedef " << fixCType(durationStruct) << " CType;\n";
              ss << "    typedef " << zsDurationType << " DurationType;\n";
              ss << "    typedef DurationType * DurationTypeRawPtr;\n";
              ss << "    if (DurationType() == value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new DurationType(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << zsDurationType << " zs_" << durationType << "_wrapperFromHandle(" << fixCType(durationStruct) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << zsDurationType << " DurationType;\n";
              ss << "    typedef DurationType * DurationTypeRawPtr;\n";
              ss << "    if (0 == handle) return DurationType();\n";
              ss << "    return (*reinterpret_cast<DurationTypeRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperList(
                                                HelperFile &helperFile,
                                                const String &listOrSetStr
                                                )
        {
          bool isMap = ("map" == listOrSetStr);
          bool isList = ("list" == listOrSetStr);
          auto context = helperFile.global_->toContext()->findType("::std::" + listOrSetStr);
          if (!context) return;

          auto structType = context->toStruct();
          if (!structType) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          for (auto iter = structType->mTemplatedStructs.begin(); iter != structType->mTemplatedStructs.end(); ++iter) {
            auto templatedStructType = (*iter).second;

            TypePtr keyType;
            TypePtr listType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              listType = (*iterArg);
              if (isMap) {
                ++iterArg;
                if (iterArg != templatedStructType->mTemplateArguments.end()) {
                  keyType = listType;
                  listType = (*iterArg);
                }
              }
            }

            includeType(helperFile, listType);

            {
              {
                auto &ss = helperFile.headerCFunctionsSS_;
                ss << getApiExportDefine(structType) << " " << fixCType(templatedStructType) << " " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "();\n";
                ss << getApiExportDefine(structType) << " void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperDestroy(" << fixCType(templatedStructType) << " handle);\n";
                ss << getApiExportDefine(structType) << " instance_id_t " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperInstanceId(" << fixCType(templatedStructType) << " handle);\n";
                if (isMap) {
                  ss << getApiExportDefine(structType) << " " << "void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(keyType) << " key, " << fixCType(listType) << " value);\n";
                }
                else {
                  ss << getApiExportDefine(structType) << " " << "void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(listType) << " value);\n";
                }

                ss << getApiExportDefine(structType) << " iterator_handle_t " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterBegin(" << fixCType(templatedStructType) << " handle);\n";
                ss << getApiExportDefine(structType) << " void " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterNext(iterator_handle_t iterHandle);\n";
                ss << getApiExportDefine(structType) << " bool " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterIsEnd(" << fixCType(templatedStructType) << " handle, iterator_handle_t iterHandle);\n";
                if (isMap) {
                  ss << getApiExportDefine(structType) << " " << fixCType(keyType) << " " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterKey(iterator_handle_t iterHandle);\n";
                }
                ss << getApiExportDefine(structType) << " " << fixCType(listType) << " " << getApiCallingDefine(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperIterValue(iterator_handle_t iterHandle);\n";
                ss << "\n";
              }
              {
                auto &ss = helperFile.headerCppFunctionsSS_;
                ss << "  " << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " " << fixType(templatedStructType) << "_wrapperFromHandle(" << fixCType(templatedStructType) << " handle);\n";
                ss << "  " << fixCType(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " value);\n";
                ss << "\n";
              }
            }

            std::stringstream typedefsSS;
            std::stringstream typedefsWithIterSS;
            std::stringstream typedefsSS2;
            {
              auto &ss = typedefsSS;
              ss << "  typedef " << fixCType(templatedStructType) << " CType;\n";
              if (isMap) {
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, keyType) << " WrapperKeyType;\n";
              }
              ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, listType) << " WrapperType;\n";
              ss << "  typedef ::std::" << listOrSetStr << "<" << (isMap ? "WrapperKeyType, " : "") << "WrapperType> WrapperTypeList;\n";
              ss << "  typedef shared_ptr<WrapperTypeList> WrapperTypeListPtr;\n";
              ss << "  typedef WrapperTypeListPtr * WrapperTypeListPtrRawPtr;\n";
            }
            {
              auto &ss = typedefsWithIterSS;
              ss << typedefsSS.str();
              ss << "  typedef WrapperTypeList::iterator WrapperTypeListIterator;\n";
              ss << "  typedef WrapperTypeListIterator * WrapperTypeListIteratorRawPtr;\n";
            }
            {
              String tmp = typedefsSS.str();
              tmp.replaceAll("  ", "    ");
              typedefsSS2 << tmp;
            }

            {
              {
                auto &ss = helperFile.cFunctionsSS_;
                ss << dash;
                ss << fixCType(templatedStructType) << " " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "()\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  return reinterpret_cast<CType>(new WrapperTypeListPtr(make_shared<WrapperTypeList>()));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperDestroy(" << fixCType(templatedStructType) << " handle)\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  if (0 == handle) return;\n";
                ss << "  delete reinterpret_cast<WrapperTypeListPtrRawPtr>(handle);\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "instance_id_t " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperInstanceId(" << fixCType(templatedStructType) << " handle)\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle)).get());\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_insert(" << fixCType(templatedStructType) << " handle, " << fixCType(listType) << " value)\n";
                ss << "{\n";
                ss << typedefsSS.str();
                ss << "  if (0 == handle) return;\n";

                String keyTypeStr;
                String listTypeStr;

                if (isMap) {
                  if (keyType->toEnumType()) {
                    keyTypeStr = "static_cast<" + GenerateStructHeader::getWrapperTypeString(false, listType) + ">(key)";
                  } else if ((keyType->toBasicType()) &&
                             ("string_t" != fixCType(keyType))) {
                    keyTypeStr = "key";
                  } else {
                    keyTypeStr = fixType(keyType) + "_wrapperFromHandle(key)";
                  }
                }
                if (listType->toEnumType()) {
                  listTypeStr = "static_cast<" + GenerateStructHeader::getWrapperTypeString(false, listType) + ">(value)";
                } else if ((listType->toBasicType()) &&
                           ("string_t" != fixCType(listType))) {
                  listTypeStr = "value";
                } else {
                  listTypeStr = fixType(listType) + "_wrapperFromHandle(value)";
                }

                if (isMap) {
                  ss << "  (*(*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle)))[" << keyTypeStr << "] = " << listTypeStr << ";\n";
                } else if (isList) {
                  ss << "  (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->push_back(" << listTypeStr << ");\n";
                } else {
                  ss << "  (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->insert(" << listTypeStr << ");\n";
                }
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "uintptr_t " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperIterBegin(" << fixCType(templatedStructType) << " handle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return reinterpret_cast<uintptr_t>(new WrapperTypeListIterator((*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->begin()));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperIterNext(iterator_handle_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == iterHandle) return;\n";
                ss << "  ++(*reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperIterIsEnd(" << fixCType(templatedStructType) << " handle, iterator_handle_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == handle) return true;\n";
                ss << "  if (0 == iterHandle) return true;\n";
                ss << "  auto iterRawPtr = reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle);\n";
                ss << "  bool isEnd = (*iterRawPtr) == (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle))->end();\n";
                ss << "  if (isEnd) delete iterRawPtr;\n";
                ss << "  return isEnd;\n";
                ss << "}\n";
                ss << "\n";

                if (isMap) {
                  ss << dash;
                  ss << fixCType(keyType) << " " << fixType(templatedStructType) << "_wrapperIterKey(iterator_handle_t iterHandle)\n";
                  ss << "{\n";
                  ss << typedefsWithIterSS.str();
                  ss << "  if (0 == iterHandle) return " << fixCType(keyType) << "();\n";
                  if (((keyType->toBasicType()) ||
                    (keyType->toEnumType())) &&
                    ("string_t" != fixCType(keyType))) {
                    ss << "  return (*(*reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle))).first;\n";
                  }
                  else {
                    ss << "  return " << fixType(keyType) << "_wrapperToHandle(*(*reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle)).first);\n";
                  }
                  ss << "}\n";
                  ss << "\n";
                }

                ss << dash;
                ss << fixCType(listType) << " " << getApiCallingDefine(structType) << " " << fixType(templatedStructType) << "_wrapperIterValue(iterator_handle_t iterHandle)\n";
                ss << "{\n";
                ss << typedefsWithIterSS.str();
                ss << "  if (0 == iterHandle) return " << fixCType(listType) << "();\n";
                if (((listType->toBasicType()) ||
                     (listType->toEnumType())) &&
                   ("string_t" != fixCType(listType))) {
                  ss << "  return (*(*reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle)))" << (isMap ? ".second" : "") << ";\n";
                } else {
                  ss << "  return " << fixType(listType) << "_wrapperToHandle(*(*reinterpret_cast<WrapperTypeListIteratorRawPtr>(iterHandle))" << (isMap ? ".second" : "") << ");\n";
                }
                ss << "}\n";
                ss << "\n";
              }
              {
                auto &ss = helperFile.cppFunctionsSS_;
                ss << dash2;
                ss << "  " << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " " << fixType(templatedStructType) << "_wrapperFromHandle(" << fixCType(templatedStructType) << " handle)\n";
                ss << "  {\n";
                ss << typedefsSS2.str();
                ss << "    if (0 == handle) return WrapperTypeListPtr();\n";
                ss << "    return (*reinterpret_cast<WrapperTypeListPtrRawPtr>(handle));\n";
                ss << "  }\n";
                ss << "\n";

                ss << dash2;
                ss << "  " << fixCType(templatedStructType) << " " << fixType(templatedStructType) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, templatedStructType) << " value)\n";
                ss << "  {\n";
                ss << typedefsSS2.str();
                ss << "    if (!value) return 0;\n";
                ss << "    return reinterpret_cast<CType>(new WrapperTypeListPtr(value));\n";
                ss << "  }\n";
                ss << "\n";
              }
            }

          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::prepareHelperSpecial(
                                                   HelperFile &helperFile,
                                                   const String &specialName
                                                   )
        {
          bool isPromise = "Promise" == specialName;

          auto context = helperFile.global_->toContext()->findType("::zs::" + specialName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));
          String zsSpecialType = "::zsLib::" + specialName;

          {
            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " void " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperDestroy(" << fixCType(contextStruct) << " handle);\n";
              ss << getApiExportDefine(contextStruct) << " instance_id_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperInstanceId(" << fixCType(contextStruct) << " handle);\n";
              if (isPromise) {
                ss << getApiExportDefine(contextStruct) << " event_observer_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperObserveEvents(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " uint64_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_get_id(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isSettled(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isResolved(" << fixCType(contextStruct) << " handle);\n";
                ss << getApiExportDefine(contextStruct) << " bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isRejected(" << fixCType(contextStruct) << " handle);\n";
              }
              ss << "\n";
            }
            {
              auto &ss = helperFile.headerCppFunctionsSS_;
              if (isPromise) {
                ss << "  event_observer_t zs_" << specialName << "_wrapperObserveEvents(" << specialName << "Ptr value);\n";
              }
              ss << "  " << fixCType(contextStruct) << " zs_" << specialName << "_wrapperToHandle(" << specialName << "Ptr value);\n";
              ss << "  " << specialName << "Ptr zs_" << specialName << "_wrapperFromHandle(" << fixCType(contextStruct) << " handle);\n";
              ss << "\n";
            }
          }
          {
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << "void " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperDestroy(" << fixCType(contextStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << specialName << "Ptr WrapperType;\n";
              ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "  if (0 == handle) return;\n";
              ss << "  delete reinterpret_cast<WrapperTypeRawPtr>(handle);\n";
              ss << "}\n";
              ss << "\n";

              ss << dash;
              ss << "instance_id_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperInstanceId(" << fixCType(contextStruct) << " handle)\n";
              ss << "{\n";
              ss << "  typedef " << specialName << "Ptr WrapperType;\n";
              ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypeRawPtr>(handle)).get());\n";
              ss << "}\n";
              ss << "\n";

              if (isPromise) {
                ss << dash;
                ss << "event_observer_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_wrapperObserveEvents(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return wrapper::zs_" << specialName << "_wrapperObserveEvents((*reinterpret_cast<WrapperTypeRawPtr>(handle)));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "uint64_t " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_get_id(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return SafeInt<uint64_t>((*reinterpret_cast<WrapperTypeRawPtr>(handle))->getID());\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isSettled(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isSettled();\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isResolved(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isResolved();\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "bool " << getApiCallingDefine(contextStruct) << " zs_" << specialName << "_isRejected(" << fixCType(contextStruct) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << specialName << "Ptr WrapperType;\n";
                ss << "  typedef WrapperType * WrapperTypeRawPtr;\n";
                ss << "  if (0 == handle) return false;\n";
                ss << "  return (*reinterpret_cast<WrapperTypeRawPtr>(handle))->isRejected();\n";
                ss << "}\n";
                ss << "\n";
              }
            }
            {
              auto &ss = helperFile.cppFunctionsSS_;
              if (isPromise) {
                ss << dash2;
                ss << dash2;
                ss << dash2;
                ss << dash2;
                ss << "  //\n";
                ss << "  // PromiseCallback\n";
                ss << "  //\n";
                ss << "\n";
                ss << "  ZS_DECLARE_CLASS_PTR(PromiseCallback);\n";
                ss << "\n";
                ss << "  class PromiseCallback : public IWrapperObserver,\n";
                ss << "                          public IWrapperCallbackEvent,\n";
                ss << "                          public ::zsLib::IPromiseSettledDelegate\n";
                ss << "  {\n";
                ss << "  public:\n";
                ss << "    PromiseCallback(PromisePtr promise) : promise_(promise) {}\n";
                ss << "\n";
                ss << "    static IWrapperObserverPtr *create(PromisePtr promise)\n";
                ss << "    {\n";
                ss << "      if (!promise) return static_cast<IWrapperObserverPtr *>(NULL);\n";
                ss << "\n";
                ss << "      auto pThis = make_shared<PromiseCallback>(promise);\n";
                ss << "      pThis->thisObserverRaw_ = new IWrapperObserverPtr(pThis);\n";
                ss << "      pThis->thisWeak_ = pThis;\n";
                ss << "      promise->then(pThis);\n";
                ss << "      promise->background();\n";
                ss << "      return pThis->thisObserverRaw_;\n";
                ss << "    }\n";
                ss << "\n";
                ss << "    /* IWrapperObserver */\n";
                ss << "\n";
                ss << "    virtual event_observer_t getObserver()\n";
                ss << "    {\n";
                ss << "      ::zsLib::AutoLock lock(lock_);\n";
                ss << "      if (NULL == thisObserverRaw_) return 0;\n";
                ss << "      return reinterpret_cast<event_observer_t>(thisObserverRaw_);\n";
                ss << "    }\n";
                ss << "\n";
                ss << "    virtual void observerCancel()\n";
                ss << "    {\n";
                ss << "      IWrapperObserverPtr pThis;\n";
                ss << "      {\n";
                ss << "        ::zsLib::AutoLock lock(lock_);\n";
                ss << "        if (NULL == thisObserverRaw_) return;\n";
                ss << "        pThis = *thisObserverRaw_;\n";
                ss << "        delete thisObserverRaw_;\n";
                ss << "        thisObserverRaw_ = NULL;\n";
                ss << "        promise_.reset();\n";
                ss << "      }\n";
                ss << "    }\n";
                ss << "\n";
                ss << "    /* IWrapperCallbackEvent */\n";
                ss << "\n";
                ss << "    /* (duplicate) virtual event_observer_t getObserver() = 0; */;\n";
                ss << "    virtual const char *getNamespace()    {return \"::zs\";}\n";
                ss << "    virtual const char *getClass()        {return \"Promise\";}\n";
                ss << "    virtual const char *getMethod()       {return \"onSettled\";}\n";
                ss << "    virtual generic_handle_t getSource()  {return zs_Promise_wrapperToHandle(promise_);}\n";
                ss << "    virtual instance_id_t getInstanceId() {return reinterpret_cast<instance_id_t>(promise_.get());}\n";
                ss << "    virtual generic_handle_t getEventData(int argumentIndex) {return 0;}\n";
                ss << "\n";
                ss << "    virtual void onPromiseSettled(PromisePtr promise)\n";
                ss << "    {\n";
                ss << "      {\n";
                ss << "        ::zsLib::AutoLock lock(lock_);\n";
                ss << "        if (!promise_) return;\n";
                ss << "      }\n";
                ss << "\n";
                ss << "      IWrapperCallbackEvent::fireEvent(thisWeak_.lock());\n";
                ss << "    }\n";
                ss << "\n";
                ss << "  private:\n";
                ss << "    ::zsLib::Lock lock_;\n";
                ss << "    IWrapperObserverPtr *thisObserverRaw_ {};\n";
                ss << "    PromiseCallbackWeakPtr thisWeak_;\n";
                ss << "    PromisePtr promise_;\n";
                ss << "  };\n";
                ss << "\n";

                ss << dash2;
                ss << "  event_observer_t zs_" << specialName << "_wrapperObserveEvents(" << specialName << "Ptr value)\n";
                ss << "  {\n";
                ss << "    return reinterpret_cast<event_observer_t>(PromiseCallback::create(value));\n";
                ss << "  }\n";
                ss << "\n";
              }
              ss << dash2;
              ss << "  " << fixCType(contextStruct) << " zs_" << specialName << "_wrapperToHandle(" << specialName << "Ptr value)\n";
              ss << "  {\n";
              ss << "    typedef " << fixCType(contextStruct) << " CType;\n";
              ss << "    typedef " << specialName << "Ptr WrapperType;\n";
              ss << "    if (!value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new WrapperType(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << specialName << "Ptr zs_" << specialName << "_wrapperFromHandle(" << fixCType(contextStruct) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << specialName << "Ptr WrapperType;\n";
              ss << "    typedef WrapperType * WrapperTypeRawPtr;\n";
              ss << "    if (0 == handle) return WrapperType();\n";
              ss << "    return (*reinterpret_cast<WrapperTypeRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::preparePromiseWithValue(HelperFile &helperFile)
        {
          auto context = helperFile.global_->toContext()->findType("::zs::PromiseWith");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            includeType(helperFile, promiseType);

            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " " << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_resolveValue_" << fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_resolveValue_" << fixType(promiseType) << "(zs_Promise_t handle)\n";
              ss << "{\n";
              ss << "  typedef ::zsLib::AnyHolder< " << GenerateStructHeader::getWrapperTypeString(false, promiseType) << " > AnyHolderWrapper;\n";
              ss << "  typedef PromisePtr * PromisePtrRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  PromisePtr promise = (*reinterpret_cast<PromisePtrRawPtr>(handle));\n";
              ss << "  if (!promise) return 0;\n";
              ss << "  auto holder = promise->value<AnyHolderWrapper>();\n";
              ss << "  if (!holder) return 0;\n";
              ss << "  return " << fixType(promiseType) << "_wrapperToHandle(holder->value_);\n";
              ss << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::preparePromiseWithRejectionReason(HelperFile &helperFile)
        {
          auto context = helperFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = helperFile.headerCFunctionsSS_;
            ss << "\n";
          }

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            includeType(helperFile, promiseType);

            {
              auto &ss = helperFile.headerCFunctionsSS_;
              ss << getApiExportDefine(contextStruct) << " " << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_rejectReason_" << fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
            {
              auto &ss = helperFile.cFunctionsSS_;

              ss << dash;
              ss << fixCType(promiseType) << " " << getApiCallingDefine(contextStruct) << " zs_PromiseWith_rejectReason_" << fixType(promiseType) << "(zs_Promise_t handle)\n";
              ss << "{\n";
              ss << "  typedef ::zsLib::AnyHolder< " << GenerateStructHeader::getWrapperTypeString(false, promiseType) << " > AnyHolderWrapper;\n";
              ss << "  typedef PromisePtr * PromisePtrRawPtr;\n";
              ss << "  if (0 == handle) return 0;\n";
              ss << "  PromisePtr promise = (*reinterpret_cast<PromisePtrRawPtr>(handle));\n";
              ss << "  if (!promise) return 0;\n";
              ss << "  auto holder = promise->value<AnyHolderWrapper>();\n";
              ss << "  if (!holder) return 0;\n";
              ss << "  return " << fixType(promiseType) << "_wrapperToHandle(holder->value_);\n";
              ss << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::finalizeHelperFile(HelperFile &helperFile)
        {
          {
            std::stringstream ss;

            appendStream(ss, helperFile.headerCIncludeSS_);
            appendStream(ss, helperFile.headerCFunctionsSS_);
            appendStream(ss, helperFile.headerCppIncludeSS_);
            appendStream(ss, helperFile.headerCppFunctionsSS_);

            writeBinary(helperFile.headerFileName_, UseHelper::convertToBuffer(ss.str()));
          }
          {
            std::stringstream ss;

            appendStream(ss, helperFile.cIncludeSS_);
            appendStream(ss, helperFile.cFunctionsSS_);
            appendStream(ss, helperFile.cppIncludeSS_);
            appendStream(ss, helperFile.cppFunctionsSS_);

            writeBinary(helperFile.cppFileName_, UseHelper::convertToBuffer(ss.str()));
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processNamespace(
                                               HelperFile &helperFile,
                                               NamespacePtr namespaceObj
                                               )
        {
          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            processNamespace(helperFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processStruct(helperFile, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processStruct(
                                            HelperFile &helperFile,
                                            StructPtr structObj
                                            )
        {
          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          StructFile structFile;
          structFile.cppFileName_ = UseHelper::fixRelativeFilePath(helperFile.cppFileName_, "c_" + fixType(structObj) + ".cpp");
          structFile.headerFileName_ = UseHelper::fixRelativeFilePath(helperFile.headerFileName_, "c_" + fixType(structObj) + ".h");

          {
            auto &ss = structFile.headerCIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "#pragma once\n\n";
            ss << "#include \"types.h\"\n";
            ss << "\n";
          }

          {
            auto &ss = structFile.headerCFunctionsSS_;
            ss << getApiGuardDefine(helperFile.global_) << "\n";
            ss << "\n";
          }

          {
            auto &ss = structFile.cIncludeSS_;
            ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
            ss << "\n";
            ss << "#include \"c_helpers.h\"\n";
            ss << "#include <zsLib/types.h>\n";
            ss << "#include <zsLib/eventing/types.h>\n";
            ss << "#include <zsLib/SafeInt.h>\n";
            ss << "\n";
          }
          {
            auto &ss = structFile.cFunctionsSS_;
            ss << "using namespace wrapper;\n\n";
          }

          {
            auto &ss = structFile.headerCppIncludeSS_;
            ss << "\n";
            ss << getApiGuardDefine(helperFile.global_, true) << "\n";
            ss << "\n";

            ss << "#ifdef __cplusplus\n";
          }
          {
            auto &ss = structFile.headerCppFunctionsSS_;
            ss << "\n";
            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          {
            auto &ss = structFile.cppFunctionsSS_;

            ss << "namespace wrapper\n";
            ss << "{\n";
          }

          structFile.includeC("\"c_" + fixType(structObj) + ".h\"");
          structFile.includeC("\"../" + fixType(structObj) + ".h\"");
          processStruct(helperFile, structFile, structObj, structObj);

          {
            auto &ss = structFile.headerCppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
            ss << "#endif /* __cplusplus */\n";
          }

          {
            auto &ss = structFile.cppFunctionsSS_;

            ss << "\n";
            ss << "} /* namespace wrapper */\n";
          }

          {
            std::stringstream ss;

            appendStream(ss, structFile.headerCIncludeSS_);
            appendStream(ss, structFile.headerCFunctionsSS_);
            appendStream(ss, structFile.headerCppIncludeSS_);
            appendStream(ss, structFile.headerCppFunctionsSS_);

            writeBinary(structFile.headerFileName_, UseHelper::convertToBuffer(ss.str()));
          }
          {
            std::stringstream ss;

            appendStream(ss, structFile.cIncludeSS_);
            appendStream(ss, structFile.cFunctionsSS_);
            appendStream(ss, structFile.cppIncludeSS_);
            appendStream(ss, structFile.cppFunctionsSS_);

            writeBinary(structFile.cppFileName_, UseHelper::convertToBuffer(ss.str()));
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processStruct(
                                            HelperFile &helperFile,
                                            StructFile &structFile,
                                            StructPtr rootStructObj,
                                            StructPtr structObj
                                            )
        {
          if (!structObj) return;

          if (rootStructObj == structObj) {
            for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
              auto subStructObj = (*iter).second;
              processStruct(helperFile, subStructObj);
            }
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter) {
            auto relatedTypeObj = (*iter).second;
            if (!relatedTypeObj) continue;
            processStruct(helperFile, structFile, rootStructObj, relatedTypeObj->toStruct());
          }

          {
            auto &ss = structFile.headerCFunctionsSS_;
            ss << "\n";
            ss << "/* " << fixType(structObj) << "*/\n";
            ss << "\n";
          }

          processMethods(helperFile, structFile, rootStructObj, structObj);
          processProperties(helperFile, structFile, rootStructObj, structObj);
          if (rootStructObj == structObj) {
            processEventHandlers(helperFile, structFile, structObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processMethods(
                                             HelperFile &helperFile,
                                             StructFile &structFile,
                                             StructPtr rootStructObj,
                                             StructPtr structObj
                                             )
        {
          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          auto exportStr = (rootStructObj == structObj ? getApiExportDefine(helperFile.global_) : getApiExportCastedDefine(helperFile.global_));

          bool foundConstructor = false;

          std::stringstream headerCSS;
          std::stringstream headerCppSS;
          std::stringstream cSS;
          std::stringstream cppSS;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (!method) continue;
            if (method->hasModifier(Modifier_Method_EventHandler)) continue;

            includeType(structFile, method->mResult);

            bool isConstructor = method->hasModifier(Modifier_Method_Ctor);
            bool isStatic = method->hasModifier(Modifier_Static);
            bool hasThis = ((!isStatic) && (!isConstructor));

            if (isConstructor) foundConstructor = true;

            if (rootStructObj != structObj) {
              if ((isStatic) || (isConstructor)) continue;
            }
            if (method->hasModifier(Modifier_Method_Delete)) continue;

            String name = method->mName;
            if (method->hasModifier(Modifier_AltName)) {name = method->getModifierValue(Modifier_AltName);}

            String resultCTypeStr = (isConstructor ? fixCType(structObj) : fixCType(method->hasModifier(Modifier_Optional), method->mResult));
            bool hasResult = resultCTypeStr != "void";

            {
              auto &ss = headerCSS;
              ss << exportStr << " " << resultCTypeStr << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_" << (isConstructor ? "wrapperCreate_" : "")  << name << "(";
            }
            {
              auto &ss = cSS;
              ss << dash;
              ss << resultCTypeStr << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_" << (isConstructor ? "wrapperCreate_" : "") << name << "(";
            }

            std::stringstream argSS;

            size_t totalArgs = method->mArguments.size();
            if (hasThis) ++totalArgs;
            if (method->mThrows.size() > 0) ++totalArgs;

            if (totalArgs > 1) argSS << "\n  ";

            bool first = true;

            if (method->mThrows.size() > 0) {
              argSS << "exception_handle_t wrapperExceptionHandle";
              first = false;
            }

            if (hasThis) {
              if (!first) argSS << ",\n  ";
              argSS << fixCType(structObj) << " " << "wrapperThisHandle";
              first = false;
            }

            for (auto iterArg = method->mArguments.begin(); iterArg != method->mArguments.end(); ++iterArg) {
              auto argPropertyObj = (*iterArg);
              includeType(structFile, argPropertyObj->mType);
              if (!first) argSS << ",\n  ";
              first = false;
              argSS << fixCType(argPropertyObj->mType) << " " << argPropertyObj->mName;
            }
            argSS << ")";

            {
              auto &ss = headerCSS;
              ss << argSS.str() << ";\n";
            }
            {
              auto &ss = cSS;
              ss << argSS.str() << "\n";
              ss << "{\n";
              String indentStr = "  ";
              if (method->mThrows.size() > 0) {
                indentStr += "  ";
                if (hasResult) {
                  ss << "  " << resultCTypeStr << " wrapperResult {};\n";
                }
                ss << "  try {\n";
              }
              ss << indentStr;
              if (isConstructor) {
                ss << "auto wrapperThis = wrapper" << structObj->getPathName() << "::wrapper_create();\n";
                ss << indentStr << "wrapperThis->wrapper_init_" << GenerateStructHeader::getStructInitName(structObj) << "(";
              } else {
                if (hasThis) {
                  ss << "auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << indentStr << "if (!wrapperThis) return";
                  if ("void" != resultCTypeStr) {
                    ss << " " << resultCTypeStr << "()";
                  }
                  ss << ";\n";
                  ss << indentStr;
                }
                if (hasResult) {
                  ss << (method->mThrows.size() > 0 ? "wrapperResult = " : "return ") << getToHandleMethod(method->hasModifier(Modifier_Optional), method->mResult) << "(";
                }
                if (hasThis) {
                  ss << "wrapperThis->" << method->getMappingName() << "(";
                } else {
                  ss << "wrapper" << structObj->getPathName() << "::" << method->getMappingName() << "(";
                }
              }

              first = true;
              for (auto iterNamedArgs = method->mArguments.begin(); iterNamedArgs != method->mArguments.end(); ++iterNamedArgs) {
                auto propertyObj = (*iterNamedArgs);
                if (!first) ss << ", ";
                first = false;
                ss << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(" << propertyObj->getMappingName() << ")";
              }

              if (isConstructor) {
                ss << ");\n";
                ss << indentStr << "return " << getToHandleMethod(method->hasModifier(Modifier_Optional), structObj) << "(wrapperThis);\n";
              } else {
                if (hasResult) {
                  ss << ")";
                }
                ss << ");\n";
              }

              if (method->mThrows.size() > 0) {
                for (auto iterThrow = method->mThrows.begin(); iterThrow != method->mThrows.end(); ++iterThrow) {
                  auto throwType = (*iterThrow);
                  includeType(structFile, throwType);
                  ss << "  } catch (const " << GenerateStructHeader::getWrapperTypeString(false, throwType) << " &e) {\n";
                  ss << "    wrapper::exception_set_Exception(wrapperExceptionHandle, make_shared<::zsLib::" << ("Exception" == throwType->getMappingName() ? "" : "Exceptions::") << throwType->getMappingName() << ">(e));\n";
                }
                ss << "  }\n";
                if (hasResult) {
                  ss << "  return wrapperResult;\n";
                }
              }
              ss << "}\n";
              ss << "\n";
            }
          }

          bool onlyStatic = GenerateHelper::hasOnlyStaticMethods(structObj) || structObj->hasModifier(Modifier_Static);

          if (rootStructObj == structObj) {
            if (!onlyStatic) {
              {
                auto found = helperFile.derives_.find(structObj->getPathName());
                if (found != helperFile.derives_.end()) {
                  auto &structSet = (*found).second;

                  bool foundRelated = false;
                  for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet) {
                    auto relatedStruct = (*iterSet);
                    if (!relatedStruct) continue;
                    if (relatedStruct == structObj) continue;

                    foundRelated = true;
                    includeType(structFile, relatedStruct);

                    structFile.includeC("\"../" + fixType(relatedStruct) + ".h\"");

                    {
                      auto &ss = structFile.headerCFunctionsSS_;
                      ss << exportStr << " " << fixCType(relatedStruct) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCastAs_" << fixType(relatedStruct) << "(" << fixCType(structObj) << " handle);\n";
                    }
                    {
                      auto &ss = structFile.cFunctionsSS_;
                      ss << dash;
                      ss << fixCType(relatedStruct) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCastAs_" << fixType(relatedStruct) << "(" << fixCType(structObj) << " handle)\n";
                      ss << "{\n";
                      ss << "  typedef wrapper" << relatedStruct->getPathName() << " RelatedWrapperType;\n";
                      ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                      ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                      ss << "  if (0 == handle) return 0;\n";
                      ss << "  auto originalType = *reinterpret_cast<WrapperTypePtrRawPtr>(handle);\n";
                      ss << "  auto castType = std::dynamic_pointer_cast<RelatedWrapperType>(originalType);\n";
                      ss << "  if (!castType) return 0;\n";
                      ss << "  return " << getToHandleMethod(false, relatedStruct) << "(castType);\n";
                      ss << "}\n";
                      ss << "\n";
                    }
                  }

                  if (foundRelated) {
                    auto &ss = structFile.headerCFunctionsSS_;
                    ss << "\n";
                  }
                }
              }

              if (!foundConstructor) {
                {
                  auto &ss = structFile.headerCFunctionsSS_;
                  ss << exportStr << " " << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCreate_" << structObj->getMappingName() << "();\n";
                }
                {
                  auto &ss = structFile.cFunctionsSS_;
                  ss << dash;
                  ss << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperCreate_" << structObj->getMappingName() << "()\n";
                  ss << "{\n";
                  ss << "  typedef " << fixCType(structObj) << " CType;\n";
                  ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                  ss << "  auto result = wrapper" << structObj->getPathName() << "::wrapper_create();\n";
                  ss << "  result->wrapper_init_" << GenerateStructHeader::getStructInitName(structObj) << "();\n";
                  ss << "  return reinterpret_cast<CType>(new WrapperTypePtr(result));\n";
                  ss << "}\n";
                  ss << "\n";
                }
              }

              {
                auto &ss = structFile.headerCFunctionsSS_;
                ss << exportStr << " " << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperClone(" << fixCType(structObj) << " handle);\n";
                ss << exportStr << " void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperDestroy(" << fixCType(structObj) << " handle);\n";
                ss << exportStr << " instance_id_t " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperInstanceId(" << fixCType(structObj) << " handle);\n";
              }
              {
                auto &ss = structFile.cFunctionsSS_;
                ss << dash;
                ss << fixCType(structObj) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperClone(" << fixCType(structObj) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return reinterpret_cast<" << fixCType(structObj) << ">(new WrapperTypePtr(*reinterpret_cast<WrapperTypePtrRawPtr>(handle)));\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperDestroy(" << fixCType(structObj) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                ss << "  if (0 == handle) return;\n";
                ss << "  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);\n";
                ss << "}\n";
                ss << "\n";

                ss << dash;
                ss << "instance_id_t " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_wrapperInstanceId(" << fixCType(structObj) << " handle)\n";
                ss << "{\n";
                ss << "  typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
                ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
                ss << "  if (0 == handle) return 0;\n";
                ss << "  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());\n";
                ss << "}\n";
                ss << "\n";
              }
            }

            {
              auto &ss = structFile.headerCppFunctionsSS_;
              ss << "  " << fixCType(structObj) << " " << fixType(rootStructObj) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value);\n";
              ss << "  " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " " << fixType(rootStructObj) << "_wrapperFromHandle(" << fixCType(structObj) << " handle);\n";
            }
            {
              auto &ss = structFile.cppFunctionsSS_;
              ss << dash2;
              ss << "  " << fixCType(structObj) << " " << fixType(rootStructObj) << "_wrapperToHandle(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value)\n";
              ss << "  {\n";
              ss << "    typedef " << fixCType(structObj) << " CType;\n";
              ss << "    typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
              ss << "    typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
              ss << "    if (!value) return 0;\n";
              ss << "    return reinterpret_cast<CType>(new WrapperTypePtr(value));\n";
              ss << "  }\n";
              ss << "\n";

              ss << dash2;
              ss << "  " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " " << fixType(rootStructObj) << "_wrapperFromHandle(" << fixCType(structObj) << " handle)\n";
              ss << "  {\n";
              ss << "    typedef " << GenerateStructHeader::getWrapperTypeString(false, structObj) << " WrapperTypePtr;\n";
              ss << "    typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
              ss << "    if (0 == handle) return WrapperTypePtr();\n";
              ss << "    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));\n";
              ss << "  }\n";
              ss << "\n";
            }
          }

          structFile.headerCFunctionsSS_ << headerCSS.str();
          structFile.headerCppFunctionsSS_ << headerCppSS.str();
          structFile.cFunctionsSS_ << cSS.str();
          structFile.cppFunctionsSS_ << cppSS.str();
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processProperties(
                                                HelperFile &helperFile,
                                                StructFile &structFile,
                                                StructPtr rootStructObj,
                                                StructPtr structObj
                                                )
        {
          bool onlyStatic = GenerateHelper::hasOnlyStaticMethods(structObj) || structObj->hasModifier(Modifier_Static);

          if (onlyStatic) {
            if (rootStructObj != structObj) return;
          }

          auto exportStr = (rootStructObj == structObj ? getApiExportDefine(helperFile.global_) : getApiExportCastedDefine(helperFile.global_));

          bool isDictionary = structObj->hasModifier(Modifier_Struct_Dictionary);

          auto dash = GenerateHelper::getDashedComment(String());

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter) {
            auto propertyObj = (*iter);
            if (!propertyObj) continue;

            bool isStatic = propertyObj->hasModifier(Modifier_Static);
            bool hasGetter = propertyObj->hasModifier(Modifier_Property_Getter);
            bool hasSetter = propertyObj->hasModifier(Modifier_Property_Setter);

            if (!isDictionary) {
              if ((!hasGetter) && (!hasSetter)) {
                hasGetter = hasSetter = true;
              }
            }

            includeType(structFile, propertyObj->mType);

            bool hasGet = true;
            bool hasSet = true;

            if ((hasGetter) && (!hasSetter)) hasSet = false;
            if ((hasSetter) && (!hasGetter)) hasGet = false;

            if (isStatic) {
              if (hasGet) hasGetter = true;
              if (hasSet) hasSetter = true;
            }

            {
              auto &ss = structFile.headerCFunctionsSS_;
              if (hasGet) {
                ss << exportStr << " " << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle")) << ");\n";
              }
              if (hasSet) {
                ss << exportStr << " void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle, ")) << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " value);\n";
              }
            }
            {
              auto &ss = structFile.cFunctionsSS_;
              if (hasGet) {
                ss << dash;
                ss << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle")) << ")\n";
                ss << "{\n";
                if (!isStatic) {
                  ss << "  auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << "  return " << getToHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(wrapperThis->" << (hasGetter ? "get_" : "") << propertyObj->getMappingName() << (hasGetter ? "()" : "") << ");\n";
                } else {
                  ss << "  return " << getToHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(wrapper" << structObj->getPathName() << "::get_" << propertyObj->getMappingName() << "());\n";
                }
                ss << "}\n";
                ss << "\n";
              }
              if (hasSet) {
                ss << dash;
                ss << "void " << getApiCallingDefine(structObj) << " " << fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String("") : String(fixCType(structObj) + " wrapperThisHandle, ")) << fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " value)\n";
                ss << "{\n";
                if (!isStatic) {
                  ss << "  auto wrapperThis = " << getFromHandleMethod(false, structObj) << "(wrapperThisHandle);\n";
                  ss << "  wrapperThis->" << (hasSetter ? "set_" : "") << propertyObj->getMappingName() << (hasSetter ? "(" : " = ") << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value)" << (hasSetter ? ")" : "") << ";\n";
                } else {
                  ss << "  wrapper" << structObj->getPathName() << "::set_" << propertyObj->getMappingName() << "(" << getFromHandleMethod(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value));\n";
                }
                ss << "}\n";
                ss << "\n";
              }
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processEventHandlers(
                                                   HelperFile &helperFile,
                                                   StructFile &structFile,
                                                   StructPtr structObj
                                                   )
        {
          if (!structObj) return;

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          bool foundEvent = false;
          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            if (!method->hasModifier(Modifier_Method_EventHandler)) continue;

            if (!foundEvent) {
              foundEvent = true;
              processEventHandlersStart(helperFile, structFile, structObj);
            }

            {
              auto &ss = structFile.headerCppFunctionsSS_;

              if (method->mArguments.size() > 0) {
                ss << "\n";
                ss << "    struct WrapperEvent_" << method->getMappingName() << " : public WrapperEvent\n";
                ss << "    {\n";
                ss << "      virtual const char *getMethod() {return \"" << method->getMappingName() << "\";}\n";
                ss << "      virtual generic_handle_t getEventData(int argumentIndex);\n";
                ss << "\n";
                size_t index = 1;
                for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                  auto propertyObj = (*iterArgs);
                  ss << "      " << GenerateStructHeader::getWrapperTypeString(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " param" << index << "_;\n";
                }
                ss << "    };\n";
                ss << "\n";
              }

              ss << "    virtual void " << method->getMappingName() << "(";
              bool first = true;
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto propertyObj = (*iterArgs);
                if (!first) {ss << ", ";}
                first = false;
                ss << GenerateStructHeader::getWrapperTypeString(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << propertyObj->getMappingName();
              }
              ss << ");\n";
            }
            {
              auto &ss = structFile.cppFunctionsSS_;

              if (method->mArguments.size() > 0) {
                ss << dash2;
                ss << "  generic_handle_t " << fixType(structObj) << "_WrapperObserver::WrapperEvent_" << method->getMappingName() << "::getEventData(int argumentIndex)\n";
                ss << "  {\n";
                size_t index = 1;
                for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                  auto propertyObj = (*iterArgs);

                  includeType(structFile, propertyObj->mType);

                  ss << "    if (" << (index-1) << " == argumentIndex) ";
                  bool isSimple = false;
                  {
                    auto basicType = propertyObj->toBasicType();
                    if (basicType) {
                      String basicTypeStr = fixCType(basicType->mBaseType);
                      if (("binary_t" != basicTypeStr) && ("string_t" != basicTypeStr)) isSimple = true;
                    }
                  }
                  ss << "return " << getToHandleMethod(propertyObj->hasModifier(Modifier_Optional) || isSimple, propertyObj->mType) << "(param" << index << "_);\n";
                }
                ss << "    return 0;\n";
                ss << "  }\n";
                ss << "\n";
              }

              ss << dash2;
              ss << "  void " << fixType(structObj) << "_WrapperObserver::" << method->getMappingName() << "(";
              bool first = true;
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto propertyObj = (*iterArgs);
                if (!first) { ss << ", "; }
                first = false;
                ss << GenerateStructHeader::getWrapperTypeString(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << propertyObj->getMappingName();
              }
              ss << ")\n";
              ss << "  {\n";
              if (method->mArguments.size() > 0) {
                ss << "    auto wrapperEvent = make_shared<" << fixType(structObj) << "_WrapperObserver::WrapperEvent_" << method->getMappingName() << ">();\n";
                ss << "    wrapperEvent->observer_ = thisWeak_.lock();\n";
                size_t index = 1;
                for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                  auto propertyObj = (*iterArgs);
                  first = false;
                  ss << "    wrapperEvent->param" << index << "_ = " << propertyObj->mName << ";\n";
                }
                ss << "    wrapper::IWrapperCallbackEvent::fireEvent(wrapperEvent);\n";
              } else {
                ss << "    auto wrapperEvent = make_shared<" << fixType(structObj) << "_WrapperObserver::WrapperEvent>();\n";
                ss << "    wrapperEvent->observer_ = thisWeak_.lock();\n";
                ss << "    wrapperEvent->method_ = \"" << method->getMappingName() << "\";\n";
                ss << "    wrapper::IWrapperCallbackEvent::fireEvent(wrapperEvent);\n";
              }
              ss << "  }\n";
              ss << "\n";
            }
          }

          if (!foundEvent) return;

          processEventHandlersEnd(helperFile, structFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processEventHandlersStart(
                                                        HelperFile &helperFile,
                                                        StructFile &structFile,
                                                        StructPtr structObj
                                                        )
        {
          structFile.headerIncludeCpp("\"../" + fixType(structObj) + ".h\"");

          auto dash = GenerateHelper::getDashedComment(String());
          auto dash2 = GenerateHelper::getDashedComment(String("  "));

          {
            auto &ss = structFile.headerCFunctionsSS_;
            ss << "\n";
            ss << getApiExportDefine(structObj) << " event_observer_t " << getApiCallingDefine(structObj) << " " << fixType(structObj) << "_wrapperObserveEvents(" << fixCType(structObj) << " handle);\n";
          }
          {
            auto &ss = structFile.cFunctionsSS_;
            ss << dash;
            ss << "event_observer_t " << getApiCallingDefine(structObj) << " " << fixType(structObj) << "_wrapperObserveEvents(" << fixCType(structObj) << " handle)\n";
            ss << "{\n";
            ss << "  typedef wrapper" << structObj->getPathName() << " WrapperType;\n";
            ss << "  typedef shared_ptr<WrapperType> WrapperTypePtr;\n";
            ss << "  typedef WrapperTypePtr * WrapperTypePtrRawPtr;\n";
            ss << "  if (0 == handle) return 0;\n";
            ss << "  auto pWrapper = (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));";
            ss << "  if (!pWrapper) return 0;\n";
            ss << "  return reinterpret_cast<event_observer_t>(" << fixType(structObj) << "_WrapperObserver::wrapperObserverCreate(pWrapper)" << ");\n";
            ss << "}\n";
            ss << "\n";
          }
          {
            auto &ss = structFile.headerCppFunctionsSS_;
            ss << "\n";
            ss << "  ZS_DECLARE_STRUCT_PTR(" << fixType(structObj) << "_WrapperObserver);\n";
            ss << "\n";
            ss << "  struct " << fixType(structObj) << "_WrapperObserver :\n";
            ss << "    public wrapper" << structObj->getPathName() << "::WrapperObserver, \n";
            ss << "    public IWrapperObserver\n";
            ss << "  {\n";
            ss << "    static IWrapperObserverPtr *wrapperObserverCreate(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value);\n";
            ss << "\n";
            ss << "    /* IWrapperObserver */\n";
            ss << "\n";
            ss << "    virtual event_observer_t getObserver();\n";
            ss << "    virtual void observerCancel();\n";
            ss << "\n";
            ss << "    /* WrapperEvent */\n";
            ss << "\n";
            ss << "    struct WrapperEvent : public IWrapperCallbackEvent\n";
            ss << "    {\n";
            ss << "      virtual event_observer_t getObserver() {return observer_->getObserver();}\n";
            ss << "      virtual const char *getNamespace()     {return \"" << structObj->getPath() << "\";}\n";
            ss << "      virtual const char *getClass()         {return \"" << structObj->getMappingName() << "\";}\n";
            ss << "      virtual const char *getMethod()        {return method_.c_str();}\n";
            ss << "      virtual generic_handle_t getSource()   {return reinterpret_cast<generic_handle_t>(new " << GenerateStructHeader::getWrapperTypeString(false, structObj) << "(observer_->source_.lock()));}\n";
            ss << "      virtual instance_id_t getInstanceId()  {auto source = observer_->source_.lock(); if (!source) return static_cast<instance_id_t>(0); return reinterpret_cast<instance_id_t>(source.get());}\n";

            ss << "      virtual generic_handle_t getEventData(int argumentIndex) {return 0;}\n";
            ss << "\n";
            ss << "      " << fixType(structObj) << "_WrapperObserverPtr observer_;\n";
            ss << "      ::zsLib::String method_;\n";
            ss << "    };\n";
            ss << "\n";
            ss << "    /* wrapper" << structObj->getPathName() << "::WrapperObserver */\n";
            ss << "\n";
          }
          {
            auto &ss = structFile.cppFunctionsSS_;
            ss << dash2;
            ss << "  IWrapperObserverPtr * " << fixType(structObj) << "_WrapperObserver::wrapperObserverCreate(" << GenerateStructHeader::getWrapperTypeString(false, structObj) << " value)\n";
            ss << "  {\n";
            ss << "    auto pThis = make_shared<" << fixType(structObj) << "_WrapperObserver>();\n";
            ss << "    pThis->thisObserverRaw_ = new IWrapperObserverPtr(pThis);\n";
            ss << "    pThis->thisWeak_ = pThis;\n";
            ss << "    pThis->source_ = value;\n";
            ss << "    if (value) value->wrapper_installObserver(pThis);\n";
            ss << "    return pThis->thisObserverRaw_;\n";
            ss << "  }\n";
            ss << "\n";

            ss << dash2;
            ss << "  event_observer_t " << fixType(structObj) << "_WrapperObserver::getObserver()\n";
            ss << "  {\n";
            ss << "    ::zsLib::AutoLock lock(lock_);\n";
            ss << "    if (NULL == thisObserverRaw_) return 0;\n";
            ss << "    return reinterpret_cast<event_observer_t>(thisObserverRaw_);\n";
            ss << "  }\n";
            ss << "\n";

            ss << dash2;
            ss << "  void " << fixType(structObj) << "_WrapperObserver::observerCancel()\n";
            ss << "  {\n";
            ss << "    IWrapperObserverPtr pObserverThis;\n";
            ss << "    {\n";
            ss << "      ::zsLib::AutoLock lock(lock_);\n";
            ss << "      if (NULL == thisObserverRaw_) return;\n";
            ss << "      auto pThis = thisWeak_.lock();\n";
            ss << "      pObserverThis = *thisObserverRaw_;\n";
            ss << "      auto value = source_.lock();\n";
            ss << "      if (value) value->wrapper_uninstallObserver(pThis);\n";
            ss << "      auto temp = thisObserverRaw_;\n";
            ss << "      thisObserverRaw_ = NULL;\n";
            ss << "      delete temp;\n";
            ss << "    }\n";
            ss << "  }\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processEventHandlersEnd(
                                                      HelperFile &helperFile,
                                                      StructFile &structFile,
                                                      StructPtr structObj
                                                      )
        {
          {
            auto &ss = structFile.headerCppFunctionsSS_;
            ss << "\n";
            ss << "    /* data */\n";
            ss << "\n";
            ss << "    ::zsLib::Lock lock_;\n";
            ss << "    " << fixType(structObj) << "_WrapperObserverWeakPtr thisWeak_;\n";
            ss << "    IWrapperObserverPtr *thisObserverRaw_;\n";
            ss << "    wrapper" << structObj->getPathName() << "WeakPtr source_;\n";
            ss << "  };\n";
            ss << " \n";
          }
        }

        //---------------------------------------------------------------------
        SecureByteBlockPtr GenerateStructC::generateTypesHeader(ProjectPtr project) throw (Failure)
        {
          if (!project) return SecureByteBlockPtr();
          if (!project->mGlobal) return SecureByteBlockPtr();

          std::stringstream ss;

          ss << "/* " ZS_EVENTING_GENERATED_BY " */\n\n";
          ss << "#pragma once\n\n";
          ss << "\n";
          ss << "#include <stdint.h>\n\n";
          ss << "\n";

          ss << "#ifdef __cplusplus\n";
          ss << "#define " << getApiGuardDefine(project) << "    extern \"C\" {\n";
          ss << "#define " << getApiGuardDefine(project, true) << "      }\n";
          ss << "#else /* __cplusplus */\n";
          ss << "#include <stdbool.h>\n";
          ss << "#define " << getApiGuardDefine(project) << "\n";
          ss << "#define " << getApiGuardDefine(project, true) << "\n";
          ss << "#endif /* __cplusplus */\n";
          ss << "\n";

          ss << "#ifndef " << getApiExportDefine(project) << "\n";
          ss << "#ifdef " << getApiImplementationDefine(project) << "\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiExportDefine(project) << " __declspec(dllexport)\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiExportDefine(project) << " __attribute__((visibility(\"default\")))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#else /* "<< getApiImplementationDefine(project) << " */\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiExportDefine(project) << " __declspec(dllimport)\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiExportDefine(project) << " __attribute__((visibility(\"default\")))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#endif /* " << getApiImplementationDefine(project) << " */\n";
          ss << "#endif /* ndef " << getApiExportDefine(project) << " */\n";
          ss << "\n";

          ss << "#ifndef " << getApiExportCastedDefine(project) << "\n";
          ss << "/* By defining " << getApiCastRequiredDefine(project) << " the wrapper will not export\n";
          ss << "   any base class methods and instead will expect the caller to cast the C object handle\n";
          ss << "   type to the base C object type to access base object methods and properties. */\n";
          ss << "#ifdef " << getApiCastRequiredDefine(project) << "\n";
          ss << "#define " << getApiExportCastedDefine(project) << "\n";
          ss << "#else /* " << getApiCastRequiredDefine(project) << " */\n";
          ss << "#define " << getApiExportCastedDefine(project) << " " << getApiExportDefine(project) << "\n";
          ss << "#endif /* " << getApiCastRequiredDefine(project) << " */\n";
          ss << "#endif /* ndef " << getApiExportCastedDefine(project) << " */\n";
          ss << "\n";

          ss << "#ifndef " << getApiCallingDefine(project) << "\n";
          ss << "#ifdef _WIN32\n";
          ss << "#define " << getApiCallingDefine(project) << " __cdecl\n";
          ss << "#else /* _WIN32 */\n";
          ss << "#define " << getApiCallingDefine(project) << " __attribute__((cdecl))\n";
          ss << "#endif /* _WIN32 */\n";
          ss << "#endif /* ndef " << getApiCallingDefine(project) << " */\n";
          ss << "\n";

          ss << getApiGuardDefine(project) << "\n";
          ss << "\n";
          ss << "typedef bool bool_t;\n";
          ss << "typedef signed char schar_t;\n";
          ss << "typedef unsigned char uchar_t;\n";
          ss << "typedef signed short sshort_t;\n";
          ss << "typedef unsigned short ushort_t;\n";
          ss << "typedef signed int sint_t;\n";
          ss << "typedef unsigned int uint_t;\n";
          ss << "typedef signed long slong_t;\n";
          ss << "typedef unsigned long ulong_t;\n";
          ss << "typedef signed long long sllong_t;\n";
          ss << "typedef unsigned long long ullong_t;\n";
          ss << "typedef float float_t;\n";
          ss << "typedef double double_t;\n";
          ss << "typedef float float32_t;\n";
          ss << "typedef double float64_t;\n";
          ss << "typedef long double ldouble_t;\n";
          ss << "typedef uintptr_t raw_pointer_t;\n";
          ss << "typedef uintptr_t binary_t;\n";
          ss << "typedef uint64_t binary_size_t;\n";
          ss << "typedef uintptr_t string_t;\n";
          ss << "typedef uintptr_t const_char_star_t;\n";
          ss << "\n";
          ss << "typedef uintptr_t box_bool_t;\n";
          ss << "typedef uintptr_t box_schar_t;\n";
          ss << "typedef uintptr_t box_uchar_t;\n";
          ss << "typedef uintptr_t box_sshort_t;\n";
          ss << "typedef uintptr_t box_ushort_t;\n";
          ss << "typedef uintptr_t box_sint_t;\n";
          ss << "typedef uintptr_t box_uint_t;\n";
          ss << "typedef uintptr_t box_slong_t;\n";
          ss << "typedef uintptr_t box_ulong_t;\n";
          ss << "typedef uintptr_t box_sllong_t;\n";
          ss << "typedef uintptr_t box_ullong_t;\n";
          ss << "typedef uintptr_t box_float_t;\n";
          ss << "typedef uintptr_t box_double_t;\n";
          ss << "typedef uintptr_t box_float32_t;\n";
          ss << "typedef uintptr_t box_float64_t;\n";
          ss << "typedef uintptr_t box_ldouble_t;\n";
          ss << "typedef uintptr_t box_int8_t;\n";
          ss << "typedef uintptr_t box_uint8_t;\n";
          ss << "typedef uintptr_t box_int16_t;\n";
          ss << "typedef uintptr_t box_uint16_t;\n";
          ss << "typedef uintptr_t box_int32_t;\n";
          ss << "typedef uintptr_t box_uint32_t;\n";
          ss << "typedef uintptr_t box_int64_t;\n";
          ss << "typedef uintptr_t box_uint64_t;\n";
          ss << "typedef uintptr_t box_raw_pointer_t;\n";
          ss << "typedef uintptr_t box_binary_t;\n";
          ss << "typedef uintptr_t box_binary_size_t;\n";
          ss << "typedef uintptr_t box_string_t;\n";
          ss << "\n";
          ss << "typedef uintptr_t instance_id_t;\n";
          ss << "typedef uintptr_t event_observer_t;\n";
          ss << "typedef uintptr_t callback_event_t;\n";
          ss << "typedef uintptr_t generic_handle_t;\n";
          ss << "typedef uintptr_t iterator_handle_t;\n";
          ss << "typedef uintptr_t exception_handle_t;\n";
          ss << "\n";

          processTypesNamespace(ss, project->mGlobal);

          ss << "\n";
          processTypesTemplatesAndSpecials(ss, project);
          ss << "\n";
          ss << getApiGuardDefine(project, true) << "\n";

          ss << "\n";
          ss << "#ifdef __cplusplus\n";
          ss << "#include \"../types.h\"\n";
          ss << "\n";

          ss << "namespace wrapper\n";
          ss << "{\n";
          ss << "  struct IWrapperObserver;\n";
          ss << "  typedef shared_ptr<IWrapperObserver> IWrapperObserverPtr;\n";
          ss << "\n";
          ss << "  struct IWrapperObserver\n";
          ss << "  {\n";
          ss << "    virtual event_observer_t getObserver() = 0;\n";
          ss << "    virtual void observerCancel() = 0;\n";
          ss << "  };\n";
          ss << "\n";

          ss << "  struct IWrapperCallbackEvent;\n";
          ss << "  typedef shared_ptr<IWrapperCallbackEvent> IWrapperCallbackEventPtr;\n";
          ss << "\n";
          ss << "  struct IWrapperCallbackEvent\n";
          ss << "  {\n";
          ss << "    static void fireEvent(IWrapperCallbackEventPtr event);\n";
          ss << "\n";
          ss << "    virtual event_observer_t getObserver() = 0;\n";
          ss << "    virtual const char *getNamespace() = 0;\n";
          ss << "    virtual const char *getClass() = 0;\n";
          ss << "    virtual const char *getMethod() = 0;\n";
          ss << "    virtual generic_handle_t getSource() = 0;\n";
          ss << "    virtual instance_id_t getInstanceId() = 0;\n";
          ss << "    virtual generic_handle_t getEventData(int argumentIndex) = 0;\n";
          ss << "  };\n";
          ss << "\n";
          ss << "} /* namespace wrapper */\n";
          ss << "\n";
          ss << "#endif /* __cplusplus */\n";

          return UseHelper::convertToBuffer(ss.str());
        }
        
        //---------------------------------------------------------------------
        void GenerateStructC::processTypesNamespace(
                                                    std::stringstream &ss,
                                                    NamespacePtr namespaceObj
                                                    )
        {
          if (!namespaceObj) return;
          if (namespaceObj->hasModifier(Modifier_Special)) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter)
          {
            auto subNamespaceObj = (*iter).second;
            processTypesNamespace(ss, subNamespaceObj);
          }

          processTypesEnum(ss, namespaceObj);

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter)
          {
            auto structObj = (*iter).second;
            processTypesStruct(ss, structObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesStruct(
                                                  std::stringstream &ss,
                                                  StructPtr structObj
                                                  )
        {
          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          ss << "typedef uintptr_t " << fixCType(structObj) << ";\n";

          processTypesEnum(ss, structObj);

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processTypesStruct(ss, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesEnum(
                                               std::stringstream &ss,
                                               ContextPtr context
                                               )
        {
          auto namespaceObj = context->toNamespace();
          auto structObj = context->toStruct();
          if ((!namespaceObj) && (!structObj)) return;

          auto &enums = namespaceObj ? (namespaceObj->mEnums) : (structObj->mEnums);
          for (auto iter = enums.begin(); iter != enums.end(); ++iter)
          {
            auto enumObj = (*iter).second;
            ss << "typedef " << fixCType(enumObj->mBaseType) << " " << fixCType(enumObj) << ";\n";
            ss << "typedef uintptr_t box_" << fixCType(enumObj) << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesTemplatesAndSpecials(
                                                               std::stringstream &ss,
                                                               ProjectPtr project
                                                               )
        {
          if (!project) return;

          ContextPtr context = project;
          processTypesTemplate(ss, context->findType("::std::list"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::std::map"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::std::set"));
          ss << "\n";
          processTypesTemplate(ss, context->findType("::zs::PromiseWith"));
          ss << "\n";

          processTypesSpecialStruct(ss, context->findType("::zs::Any"));
          processTypesSpecialStruct(ss, context->findType("::zs::Promise"));
          processTypesSpecialStruct(ss, context->findType("::zs::Exception"));
          processTypesSpecialStruct(ss, context->findType("::zs::InvalidArgument"));
          processTypesSpecialStruct(ss, context->findType("::zs::BadState"));
          processTypesSpecialStruct(ss, context->findType("::zs::NotImplemented"));
          processTypesSpecialStruct(ss, context->findType("::zs::NotSupported"));
          processTypesSpecialStruct(ss, context->findType("::zs::UnexpectedError"));
          ss << "\n";

          processTypesSpecialStruct(ss, context->findType("::zs::Time"));
          processTypesSpecialStruct(ss, context->findType("::zs::Milliseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Microseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Nanoseconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Seconds"));
          processTypesSpecialStruct(ss, context->findType("::zs::Minutes"));
          processTypesSpecialStruct(ss, context->findType("::zs::Hours"));
          processTypesSpecialStruct(ss, context->findType("::zs::Days"));
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesTemplate(
                                                   std::stringstream &ss,
                                                   ContextPtr structContextObj
                                                   )
        {
          if (!structContextObj) return;

          auto structObj = structContextObj->toStruct();
          if (!structObj) return;

          if (structObj->mGenerics.size() < 1) return;

          for (auto iter = structObj->mTemplatedStructs.begin(); iter != structObj->mTemplatedStructs.end(); ++iter) {
            auto templatedStruct = (*iter).second;
            ss << "typedef uintptr_t " << fixCType(templatedStruct) << ";\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructC::processTypesSpecialStruct(
                                                        std::stringstream &ss,
                                                        ContextPtr structContextObj
                                                        )
        {
          if (!structContextObj) return;

          auto structObj = structContextObj->toStruct();
          if (!structObj) return;

          if (!structObj->hasModifier(Modifier_Special)) return;

          ss << "typedef uintptr_t " << fixCType(structObj) << ";\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructC::targetKeyword()
        {
          return String("c");
        }

        //---------------------------------------------------------------------
        String GenerateStructC::targetKeywordHelp()
        {
          return String("Generate C wrapper");
        }

        //---------------------------------------------------------------------
        void GenerateStructC::targetOutput(
                                            const String &inPathStr,
                                            const ICompilerTypes::Config &config
                                            ) throw (Failure)
        {
          typedef std::stack<NamespacePtr> NamespaceStack;
          typedef std::stack<String> StringList;

          String pathStr(UseHelper::fixRelativeFilePath(inPathStr, String("wrapper")));

          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("generated"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("c"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          writeBinary(UseHelper::fixRelativeFilePath(pathStr, String("types.h")), generateTypesHeader(project));

          HelperFile helperFile;
          helperFile.global_ = project->mGlobal;

          calculateRelations(project->mGlobal, helperFile.derives_);
          calculateBoxings(project->mGlobal, helperFile.boxings_);

          helperFile.cppFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("c_helpers.cpp"));
          helperFile.headerFileName_ = UseHelper::fixRelativeFilePath(pathStr, String("c_helpers.h"));

          prepareHelperFile(helperFile);

          finalizeHelperFile(helperFile);
          processNamespace(helperFile, helperFile.global_);
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
