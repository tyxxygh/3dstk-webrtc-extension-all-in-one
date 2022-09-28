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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructDotNet.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructC.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructCx.h>
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
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet::BaseFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructDotNet::BaseFile::BaseFile()
        {
        }

        //---------------------------------------------------------------------
        GenerateStructDotNet::BaseFile::~BaseFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::usingTypedef(
                                                          const String &usingType,
                                                          const String &originalType
                                                          )
        {
          if (alreadyUsing_.end() != alreadyUsing_.find(usingType)) return;
          alreadyUsing_.insert(usingType);

          auto &ss = (originalType.hasData() ? usingAliasSS_ : usingNamespaceSS_);
          ss << "using " << usingType;
          if (originalType.hasData()) {
            ss << " = " << originalType;
          }
          ss << ";\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::usingTypedef(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void: return;

            case PredefinedTypedef_binary: {
              usingTypedef("binary_t", "System.IntPtr");
              usingTypedef("box_binary_t", "System.IntPtr");
              return;
            }
            case PredefinedTypedef_string:
            case PredefinedTypedef_astring:
            case PredefinedTypedef_wstring: {
              usingTypedef("string_t", "System.IntPtr");
              usingTypedef("box_string_t", "System.IntPtr");
              return;
            }
            default: break;
          }
          usingTypedef(GenerateStructC::fixCType(type), fixCsSystemType(type));
          usingTypedef("box_" + GenerateStructC::fixCType(type), "System.IntPtr");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::usingTypedef(TypePtr type)
        {
          if (!type) return;

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              usingTypedef(basicType->mBaseType);
              return;
            }
          }

          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) {
                auto specialName = structType->getPathName();

                {
                  if ("::zs::Any" == specialName) goto define_using_default;
                  if ("::zs::Promise" == specialName) goto define_using_default;
                  if ("::zs::Time" == specialName) goto define_using_default;
                  if ("::zs::Nanoseconds" == specialName) goto define_using_default;
                  if ("::zs::Microseconds" == specialName) goto define_using_default;
                  if ("::zs::Milliseconds" == specialName) goto define_using_default;
                  if ("::zs::Seconds" == specialName) goto define_using_default;
                  if ("::zs::Minutes" == specialName) goto define_using_default;
                  if ("::zs::Hours" == specialName) goto define_using_default;
                  if ("::zs::Days" == specialName) goto define_using_default;
                  return;
                }
              define_using_default: {}
              }
              usingTypedef(GenerateStructC::fixCType(type), "System.IntPtr");
              return;
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              usingTypedef(GenerateStructC::fixCType(type), "System.IntPtr");
              return;
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto systemType = fixCsSystemType(enumType->mBaseType);
              if (systemType.hasData()) {
                usingTypedef(GenerateStructC::fixCType(type), systemType);
                usingTypedef(GenerateStructC::fixCType(true, type), "System.IntPtr");
              }
            }
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructDotNet::BaseFile::hasBoxing(const String &namePathStr)
        {
          auto found = boxings_.find(namePathStr);
          return found != boxings_.end();
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::startRegion(const String &region)
        {
          auto dash = GenerateHelper::getDashedComment(indent_);

          auto &ss = structSS_;
          ss << "\n";
          ss << indent_ << "#region " << region << "\n";
          ss << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "// " << region << "\n";
          ss << dash;
          ss << dash;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::endRegion(const String &region)
        {
          auto &ss = structSS_;
          ss << "\n";
          ss << indent_ << "#endregion // " << region << "\n";
          ss << "\n";
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::startOtherRegion(
                                                              const String &region,
                                                              bool preStruct
                                                              )
        {
          auto dash = GenerateHelper::getDashedComment(indent_);

          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << "\n";
          ss << indent_ << "#region " << region << "\n";
          ss << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "// " << region << "\n";
          ss << dash;
          ss << dash;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::BaseFile::endOtherRegion(
                                                            const String &region,
                                                            bool preStruct
                                                            )
        {
          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << "\n";
          ss << indent_ << "#endregion // " << region << "\n";
          ss << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet::ApiFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructDotNet::ApiFile::ApiFile() :
          helpersSS_(postStructSS_),
          helpersEndSS_(postStructEndSS_) 
        {
        }

        //---------------------------------------------------------------------
        GenerateStructDotNet::ApiFile::~ApiFile()
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructDotNet::StructFile::StructFile(
                                                     BaseFile &baseFile,
                                                     StructPtr structObj
                                                     ) :
          interfaceSS_(preStructSS_),
          interfaceEndSS_(preStructEndSS_),
          delegateSS_(structDeclationsSS_),
          struct_(structObj),
          isStaticOnly_(GenerateHelper::hasOnlyStaticMethods(structObj)),
          isDictionary(structObj->hasModifier(Modifier_Struct_Dictionary)),
          hasEvents_(GenerateHelper::hasEventHandlers(structObj)),
          shouldInheritException_((!isStaticOnly_) && shouldDeriveFromException(baseFile, structObj))
        {
          if ((!isStaticOnly_) &&
              (!isDictionary)) {
            auto parent = structObj->getParent();
            if (parent) {
              Context::FindTypeOptions options;
              options.mSearchParents = false;
              auto found = parent->findType("I" + GenerateStructCx::fixStructName(structObj), &options);
              shouldDefineInterface_ = (!((bool)found));
            }
          }
        }

        //---------------------------------------------------------------------
        GenerateStructDotNet::StructFile::~StructFile()
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructDotNet
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructDotNet::GenerateStructDotNet() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructDotNetPtr GenerateStructDotNet::create()
        {
          return make_shared<GenerateStructDotNet>();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getMarshalAsType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "I1";
            case PredefinedTypedef_uchar:       
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       
            case PredefinedTypedef_ushort:      
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      
            case PredefinedTypedef_uint:        
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        
            case PredefinedTypedef_ulong:       
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       
            case PredefinedTypedef_ulonglong:   
            case PredefinedTypedef_longlong:    
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
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixArgumentName(const String &value)
        {
          if ("params" == value) return "@params";
          if ("event" == value) return "@event";
          return value;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCCsType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        return "void";
            case PredefinedTypedef_bool:        return "bool";
            case PredefinedTypedef_uchar:       return "byte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "sbyte";
            case PredefinedTypedef_ushort:      return "ushort";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "short";
            case PredefinedTypedef_uint:        return "uint";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "int";
            case PredefinedTypedef_ulong:       return "ulong";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "long";
            case PredefinedTypedef_ulonglong:   return "ulong";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "long";
            case PredefinedTypedef_uint8:       return "byte";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "sbyte";
            case PredefinedTypedef_uint16:      return "ushort";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "short";
            case PredefinedTypedef_uint32:      return "uint";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "int";
            case PredefinedTypedef_uint64:      return "ulong";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "long";

            case PredefinedTypedef_byte:        return "byte";
            case PredefinedTypedef_word:        return "ushort";
            case PredefinedTypedef_dword:       return "uint";
            case PredefinedTypedef_qword:       return "ulong";

            case PredefinedTypedef_float:       return "float";
            case PredefinedTypedef_double:      return "double";
            case PredefinedTypedef_ldouble:     return "double";
            case PredefinedTypedef_float32:     return "float";
            case PredefinedTypedef_float64:     return "double";

            case PredefinedTypedef_pointer:     return "raw_pointer_t";

            case PredefinedTypedef_binary:      return "binary_t";
            case PredefinedTypedef_size:        return "binary_size_t";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "string_t";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCCsType(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return fixCCsType(basicType->mBaseType);
          }
          return GenerateStructC::fixCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsType(
                                               TypePtr type,
                                               bool isInterface
                                               )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case PredefinedTypedef_pointer:     return "System.IntPtr";

                case PredefinedTypedef_binary:      return "byte[]";
                case PredefinedTypedef_size:        return "ulong";

                case PredefinedTypedef_string:
                case PredefinedTypedef_astring:
                case PredefinedTypedef_wstring:     return "string";
                default:                            break;
              }
              return fixCCsType(basicType->toBasicType());
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) return GenerateStructCx::fixName(enumType->getMappingName());
          }
          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) {
                String specialName = structType->getPathName();
                if ("::zs::Time" == specialName) return "System.DateTimeOffset";
                if ("::zs::Nanoseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Microseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Milliseconds" == specialName) return "System.TimeSpan";
                if ("::zs::Seconds" == specialName) return "System.TimeSpan";
                if ("::zs::Minutes" == specialName) return "System.TimeSpan";
                if ("::zs::Hours" == specialName) return "System.TimeSpan";
                if ("::zs::Days" == specialName) return "System.TimeSpan";

                if ("::zs::exceptions::Exception" == specialName) return "System.Exception";
                if ("::zs::exceptions::InvalidArgument" == specialName) return "System.ArgumentException";
                if ("::zs::exceptions::BadState" == specialName) return "System.Runtime.InteropServices.COMException";
                if ("::zs::exceptions::NotImplemented" == specialName) return "System.NotImplementedException";
                if ("::zs::exceptions::NotSupported" == specialName) return "System.NotSupportedException";
                if ("::zs::exceptions::UnexpectedError" == specialName) return "System.Runtime.InteropServices.COMException";

                if ("::zs::Any" == specialName) return "object";
                if ("::zs::Promise" == specialName) return "System.Threading.Tasks.Task";
              }

              if (isInterface) {
                if (hasInterface(structType)) {
                  return "I" + GenerateStructCx::fixStructName(structType);
                }
              }
              return GenerateStructCx::fixStructName(structType);
            }
          }
          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              auto parentStruct = templatedType->getParentStruct();
              if (parentStruct) {
                String prefixStr;
                String postFixStr = ">";
                String specialName = parentStruct->getPathName();

                size_t maxArgs = templatedType->mTemplateArguments.size();

                if ("::zs::PromiseWith" == specialName) {
                  prefixStr = "System.Threading.Tasks.Task<";
                  maxArgs = 1;
                }
                if ("::std::list" == specialName) prefixStr = "System.Collections.Generic.IReadOnlyList<";
                if ("::std::set" == specialName) {
                  prefixStr = "System.Collections.Generic.IReadOnlyDictionary<";
                  postFixStr = ", object>";
                }
                if ("::std::map" == specialName) prefixStr = "System.Collections.Generic.IReadOnlyDictionary<";

                size_t index = 0;
                String argsStr;
                for (auto iter = templatedType->mTemplateArguments.begin(); (index < maxArgs) && (iter != templatedType->mTemplateArguments.end()); ++iter, ++index) {
                  auto subType = (*iter);
                  if (argsStr.hasData()) {
                    argsStr += ", ";
                  }
                  argsStr += fixCsPathType(subType, isInterface);
                }

                if ((">" == postFixStr.substr(0, 1)) &&
                    (argsStr.length() > 0) &&
                    (">" == argsStr.substr(argsStr.length() - 1, 1))) {
                  argsStr += " ";
                }

                return prefixStr + argsStr + postFixStr;
              }
            }
          }

          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsPathType(
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          if (!type) return String();

          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) return fixCsType(type, isInterface);

              if (isInterface) {
                if (hasInterface(structType)) {
                  auto parent = structType->getParent();
                  auto result = GenerateStructCx::fixNamePath(parent);
                  if (result.substr(0, 2) == "::") result = result.substr(2);
                  result.replaceAll("::", ".");
                  if (result.hasData()) {
                    result += ".";
                  }
                  result += "I" + GenerateStructCx::fixStructName(structType);
                  return result;
                }
              }

              auto result = GenerateStructCx::fixNamePath(structType);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::",".");
              return result;
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto parent = enumType->getParent();
              auto result = GenerateStructCx::fixNamePath(parent);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::", ".");
              if (result.hasData()) {
                result += ".";
              }
              result += GenerateStructCx::fixName(enumType->getMappingName());
              return result;
            }
          }

          return fixCsType(type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsType(
                                               bool isOptional,
                                               TypePtr type,
                                               bool isInterface
                                               )
        {
          auto result = fixCsType(type, isInterface);
          if (!isOptional) return result;

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case PredefinedTypedef_binary:      return result;
                case PredefinedTypedef_string:
                case PredefinedTypedef_astring:
                case PredefinedTypedef_wstring:     return result;
                default:                            break;
              }
              return result + "?";
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return result + "?";
            }
          }

          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsPathType(
                                                   bool isOptional,
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          if (!isOptional) return fixCsPathType(type, isInterface);
          if (type->toStruct()) return fixCsPathType(type, isInterface);
          return fixCsType(isOptional, type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::fixCsSystemType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        return String();
            case PredefinedTypedef_bool:        return "System.Boolean";
            case PredefinedTypedef_uchar:       return "System.Byte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "System.SByte";
            case PredefinedTypedef_ushort:      return "System.UInt16";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "System.Int16";
            case PredefinedTypedef_uint:        return "System.UInt32";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "System.Int32";
            case PredefinedTypedef_ulong:       return "System.UInt64";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "System.Int64";
            case PredefinedTypedef_ulonglong:   return "System.UInt64";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "System.Int64";
            case PredefinedTypedef_uint8:       return "System.Byte";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "System.SByte";
            case PredefinedTypedef_uint16:      return "System.UInt16";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "System.Int16";
            case PredefinedTypedef_uint32:      return "System.UInt32";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "System.Int32";
            case PredefinedTypedef_uint64:      return "System.UInt64";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "System.Int64";

            case PredefinedTypedef_byte:        return "System.Byte";
            case PredefinedTypedef_word:        return "System.UInt16";
            case PredefinedTypedef_dword:       return "System.UInt32";
            case PredefinedTypedef_qword:       return "System.UInt64";

            case PredefinedTypedef_float:       return "System.Single";
            case PredefinedTypedef_double:      return "System.Double";
            case PredefinedTypedef_ldouble:     return "System.Double";
            case PredefinedTypedef_float32:     return "System.Single";
            case PredefinedTypedef_float64:     return "System.Double";

            case PredefinedTypedef_pointer:     return "System.IntPtr";

            case PredefinedTypedef_binary:      return "System.IntPtr";
            case PredefinedTypedef_size:        return "System.UInt64";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "System.IntPtr";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::combineIf(
                                               const String &combinePreStr,
                                               const String &combinePostStr,
                                               const String &ifHasValue
                                               )
        {
          if (ifHasValue.isEmpty()) return String();
          return combinePreStr + ifHasValue + combinePostStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getReturnMarshal(
                                                      IEventingTypes::PredefinedTypedefs type,
                                                      const String &indentStr
                                                      )
        {
          return combineIf(indentStr + "[return: MarshalAs(UnmanagedType.", ")]\n", getMarshalAsType(type));
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getParamMarshal(IEventingTypes::PredefinedTypedefs type)
        {
          return combineIf("[MarshalAs(UnmanagedType.", ")] ", getMarshalAsType(type));
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getReturnMarshal(
                                                      TypePtr type,
                                                      const String &indentStr
                                                      )
        {
          if (!type) return String();
          {
            auto basicType = type->toBasicType();
            if (basicType) return getReturnMarshal(basicType->mBaseType, indentStr);
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getParamMarshal(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return getParamMarshal(basicType->mBaseType);
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getHelpersMethod(
                                                      BaseFile &baseFile,
                                                      bool useApiHelper,
                                                      const String &methodName,
                                                      bool isOptional,
                                                      TypePtr type
                                                      )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              String cTypeStr = GenerateStructC::fixCType(basicType->mBaseType);
              if (isOptional) {
                return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + ".box_" + cTypeStr + methodName;
              }
              if ("string_t" == cTypeStr) {
                return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + "." + GenerateStructC::fixType(type) + methodName;
              }
              if ("binary_t" == cTypeStr) {
                return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + "." + GenerateStructC::fixType(type) + methodName;
              }
              return String();
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              String cTypeStr = GenerateStructC::fixCType(enumType);
              if (isOptional) {
                return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + ".box_" + GenerateStructC::fixCType(type) + methodName;
              }
              return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + "." + GenerateStructC::fixCType(type) + methodName;
            }
          }

          {
            if (useApiHelper) {
              {
                auto templatedStruct = type->toTemplatedStructType();
                if (templatedStruct) {
                  auto parentStruct = templatedStruct->getParentStruct();
                  if (parentStruct) {
                    String specialName = parentStruct->getPathName();
                    if ("::zs::PromiseWith" == specialName) {
                      return getApiPath(baseFile) + ".zs_Promise" + methodName;
                    }
                  }
                }
              }
            }
          }

          return (useApiHelper ? getApiPath(baseFile) : getHelperPath(baseFile)) + "." + GenerateStructC::fixType(type) + methodName;
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getToCMethod(
                                                  BaseFile &baseFile,
                                                  bool isOptional,
                                                  TypePtr type
                                                  )
        {
          return getHelpersMethod(baseFile, false, "_ToC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getFromCMethod(
                                                    BaseFile &baseFile,
                                                    bool isOptional, 
                                                    TypePtr type
                                                    )
        {
          return getHelpersMethod(baseFile, false, "_FromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getAdoptFromCMethod(
                                                         BaseFile &baseFile,
                                                         bool isOptional, 
                                                         TypePtr type
                                                         )
        {
          return getHelpersMethod(baseFile, false, "_AdoptFromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getDestroyCMethod(
                                                       BaseFile &baseFile,
                                                       bool isOptional,
                                                       TypePtr type
                                                       )
        {
          if (!type) return String();
          if (!isOptional) {
            if (type->toEnumType()) return String();
          }
          return getHelpersMethod(baseFile, true, "_wrapperDestroy", isOptional, type);
        }

        //---------------------------------------------------------------------
        bool GenerateStructDotNet::hasInterface(StructPtr structObj)
        {
          if (!structObj) return false;
          if (structObj->hasModifier(Modifier_Struct_Dictionary)) return false;

          auto parent = structObj->getParent();
          if (!parent) return false;

          Context::FindTypeOptions options;
          auto found = parent->findType("I" + GenerateStructCx::fixStructName(structObj), &options);
          return !((bool)found);
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getApiCastRequiredDefine(BaseFile &baseFile)
        {
          String result = "WRAPPER_C_GENERATED_REQUIRES_CAST";
          if (!baseFile.project_) return result;
          if (baseFile.project_->mName.isEmpty()) return result;

          auto name = baseFile.project_->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_REQUIRES_CAST";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getApiPath(BaseFile &baseFile)
        {
          return "Wrapper." + GenerateStructCx::fixName(baseFile.project_->getMappingName()) + ".Api";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getHelperPath(BaseFile &baseFile)
        {
          return "Wrapper." + GenerateStructCx::fixName(baseFile.project_->getMappingName()) + ".Helpers";
        }

        //---------------------------------------------------------------------
        bool GenerateStructDotNet::shouldDeriveFromException(
                                                             BaseFile &baseFile,
                                                             StructPtr structObj
                                                             )
        {
          if (!structObj) return false;
          if (structObj->mIsARelationships.size() > 0) return false;
          if (structObj->hasModifier(Modifier_Static)) return false;

          auto context = baseFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (!context) return false;

          auto rejectionStruct = context->toStruct();
          if (!rejectionStruct) return false;

          auto checkName = structObj->getPathName();

          for (auto iterTemplate = rejectionStruct->mTemplatedStructs.begin(); iterTemplate != rejectionStruct->mTemplatedStructs.end(); ++iterTemplate) {
            auto templatedStruct = (*iterTemplate).second;
            if (!templatedStruct) continue;

            auto iterFoundType = templatedStruct->mTemplateArguments.begin();
            if (iterFoundType == templatedStruct->mTemplateArguments.end()) continue;

            auto foundType = (*iterFoundType);
            if (!foundType) continue;

            if (foundType->getPathName() == checkName) return true;
          }

          return false;
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::finalizeBaseFile(BaseFile &baseFile)
        {
          std::stringstream ss;

          GenerateStructC::appendStream(ss, baseFile.usingNamespaceSS_);
          GenerateStructC::appendStream(ss, baseFile.usingAliasSS_);
          GenerateStructC::appendStream(ss, baseFile.namespaceSS_);
          GenerateStructC::appendStream(ss, baseFile.preStructSS_);
          GenerateStructC::appendStream(ss, baseFile.preStructEndSS_);
          GenerateStructC::appendStream(ss, baseFile.structDeclationsSS_);
          GenerateStructC::appendStream(ss, baseFile.structSS_);
          GenerateStructC::appendStream(ss, baseFile.structEndSS_);
          GenerateStructC::appendStream(ss, baseFile.postStructSS_);
          GenerateStructC::appendStream(ss, baseFile.postStructEndSS_);
          GenerateStructC::appendStream(ss, baseFile.namespaceEndSS_);

          writeBinary(baseFile.fileName_, UseHelper::convertToBuffer(ss.str()));
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiFile(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          {
            auto &ss = apiFile.usingNamespaceSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
          }

          {
            auto &ss = apiFile.namespaceSS_;

            ss << "namespace Wrapper\n";
            ss << "{\n";
            apiFile.indentMore();
            ss << indentStr << "namespace " << GenerateStructCx::fixName(apiFile.project_->getMappingName()) << "\n";
            ss << indentStr << "{\n";
            apiFile.indentMore();
          }

          {
            auto &ss = apiFile.helpersSS_;

            ss << indentStr << "internal static class Helpers\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    private const System.UInt32 E_NOT_VALID_STATE = 5023;\n";
            ss << indentStr << "    private const System.UInt32 CO_E_NOT_SUPPORTED = 0x80004021;\n";
            ss << indentStr << "    private const System.UInt32 E_UNEXPECTED = 0x8000FFFF;\n";
          }

          {
            auto &ss = apiFile.structSS_;

            String libNameStr = "lib" + GenerateStructCx::fixName(apiFile.project_->getMappingName());
            ss << indentStr << "internal static class Api\n";
            ss << indentStr << "{\n";
            apiFile.indentMore();
            ss << "#if __TVOS__ && __UNIFIED__\n";
            ss << indentStr << "private const string UseDynamicLib = \"@rpath/" << libNameStr << ".framework/" << libNameStr << "\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __IOS__ && __UNIFIED__\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dylib\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __ANDROID__\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".so\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif __MACOS__\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dylib\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif DESKTOP\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dll\"; // redirect using .dll.config to '" << libNameStr << ".dylib' on OS X\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif WINDOWS_UWP\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << ".dll\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#elif NET_STANDARD\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << "\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#else\n";
            ss << indentStr << "private const string UseDynamicLib = \"" << libNameStr << "\";\n";
            ss << indentStr << "private const CallingConvention UseCallingConvention = CallingConvention.Cdecl;\n";
            ss << "#endif\n";
            ss << "\n";
            ss << indentStr << "private const UnmanagedType UseBoolMashal = UnmanagedType.I1;\n";
            ss << indentStr << "private const UnmanagedType UseStringMarshal = UnmanagedType.LPStr;\n";
            ss << "\n";
          }

          apiFile.usingTypedef("binary_t", "System.IntPtr");
          apiFile.usingTypedef("string_t", "System.IntPtr");
          apiFile.usingTypedef("raw_pointer_t", "System.IntPtr");
          apiFile.usingTypedef("binary_size_t", "System.UInt64");

          prepareApiCallback(apiFile);
          prepareApiExceptions(apiFile);
          prepareApiBoxing(apiFile);
          prepareApiString(apiFile);
          prepareApiBinary(apiFile);
          prepareApiDuration(apiFile);

          prepareApiList(apiFile, "list");
          prepareApiList(apiFile, "set");
          prepareApiList(apiFile, "map");

          prepareApiSpecial(apiFile, "Any");
          prepareApiSpecial(apiFile, "Promise");

          preparePromiseWithValue(apiFile);
          preparePromiseWithRejectionReason(apiFile);

          prepareApiNamespace(apiFile, apiFile.global_);

          apiFile.startRegion("Struct API helpers");
          apiFile.startHelpersRegion("Struct helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::finalizeApiFile(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.endRegion("Struct API helpers");
          apiFile.endHelpersRegion("Struct helpers");
          apiFile.indentLess();

          {
            auto &ss = apiFile.helpersEndSS_;
            ss << "\n";
            ss << indentStr << "}\n";
          }

          {
            auto &ss = apiFile.structEndSS_;
            ss << "\n";
            ss << indentStr << "}\n";
          }

          {
            auto &ss = apiFile.namespaceEndSS_;

            ss << "\n";
            apiFile.indentLess();
            ss << "    } // namespace " << GenerateStructCx::fixName(apiFile.project_->getMappingName()) << "\n";
            apiFile.indentLess();
            ss << "} // namespace Wrapper\n";
          }

          finalizeBaseFile(apiFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiCallback(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("System.Runtime.InteropServices");
          apiFile.usingTypedef("generic_handle_t", "System.IntPtr");
          apiFile.usingTypedef("instance_id_t", "System.IntPtr");
          apiFile.usingTypedef("callback_event_t", "System.IntPtr");
          apiFile.usingTypedef("event_observer_t", "System.IntPtr");
          apiFile.usingTypedef("const_char_star_t", "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;

            apiFile.startRegion("Callback and Event API helpers");

            static const char *callbackHelpers =
              "// void wrapperCallbackFunction(callback_event_t handle);\n"
              "[UnmanagedFunctionPointer(UseCallingConvention)]\n"
              "public delegate void WrapperCallbackFunction(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_wrapperInstall([MarshalAs(UnmanagedType.FunctionPtr)] WrapperCallbackFunction function);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_wrapperObserverDestroy(event_observer_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void callback_event_wrapperDestroy(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t callback_event_wrapperInstanceId(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static event_observer_t callback_event_get_observer(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static const_char_star_t callback_event_get_namespace_actual(callback_event_t handle);\n"
              "\n"
              "public static string callback_event_get_namespace(callback_event_t handle)\n"
              "{\n"
              "    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(callback_event_get_namespace_actual(handle));\n"
              "}\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static const_char_star_t callback_event_get_class_actual(callback_event_t handle);\n"
              "\n"
              "public static string callback_event_get_class(callback_event_t handle)\n"
              "{\n"
              "    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(callback_event_get_class_actual(handle));\n"
              "}\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static const_char_star_t callback_event_get_method_actual(callback_event_t handle);\n"
              "\n"
              "public static string callback_event_get_method(callback_event_t handle)\n"
              "{\n"
              "    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(callback_event_get_method_actual(handle));\n"
              "}\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static generic_handle_t callback_event_get_source(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t callback_event_get_source_instance_id(callback_event_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static generic_handle_t callback_event_get_data(callback_event_t handle, int argumentIndex);\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, callbackHelpers);

            apiFile.endRegion("Callback and Event API helpers");
          }

          {
            auto &ss = apiFile.helpersSS_;

            apiFile.startHelpersRegion("Callback and Event helpers");

            static const char *callbackHelpers =
              "public delegate void FireEventDelegate(object target, string methodName, callback_event_t handle);\n"
              "\n"
              "private class EventManager\n"
              "{\n"
              "    private static EventManager singleton_ = new EventManager();\n"
              "\n"
              "    private object lock_ = new System.Object();\n"
              "\n"
              "    private class Target\n"
              "    {\n"
              "        public System.WeakReference<object> target_;\n"
              "        public FireEventDelegate delegate_;\n"
              "\n"
              "        public Target(object target, FireEventDelegate fireDelegate)\n"
              "        {\n"
              "            this.target_ = new System.WeakReference<object>(target);\n"
              "            this.delegate_ = fireDelegate;\n"
              "        }\n"
              "        public void FireEvent(string methodName, callback_event_t handle)\n"
              "        {\n"
              "            object target = null;\n"
              "            if (!this.target_.TryGetTarget(out target)) return;\n"
              "            this.delegate_(target, methodName, handle);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private class InstanceObservers\n"
              "    {\n"
              "        public System.Collections.Generic.List<Target> targets_ = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            var oldList = targets_;\n"
              "            var newList = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "            foreach(Target value in oldList)\n"
              "            {\n"
              "                object existingTarget = null;\n"
              "                if (!value.target_.TryGetTarget(out existingTarget)) continue;\n"
              "                if (System.Object.ReferenceEquals(existingTarget, target)) continue;\n"
              "                newList.Add(value);\n"
              "            }\n"
              "            newList.Add(new Target(target, targetCallback));\n"
              "            targets_ = newList;\n"
              "        }\n"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            object target)\n"
              "        {\n"
              "            var oldList = targets_;\n"
              "            var newList = new System.Collections.Generic.List<Target>();\n"
              "\n"
              "            foreach (Target value in oldList)\n"
              "            {\n"
              "                object existingTarget = null;\n"
              "                if (!value.target_.TryGetTarget(out existingTarget)) continue;\n"
              "                if (System.Object.ReferenceEquals(existingTarget, target)) continue;\n"
              "                newList.Add(value);\n"
              "            }\n"
              "            targets_ = newList;\n"
              "        }\n"
              "\n"
              "        public System.Collections.Generic.List<Target> Targets { get { return targets_; } }\n"
              "    }\n"
              "\n"
              "    private class StructObservers\n"
              "    {\n"
              "        public System.Collections.Generic.Dictionary<instance_id_t, InstanceObservers> observers_ = new System.Collections.Generic.Dictionary<instance_id_t, InstanceObservers>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            instance_id_t source,\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            InstanceObservers observer = null;\n"
              "            if (!observers_.TryGetValue(source, out observer))\n"
              "            {\n"
              "                observer = new InstanceObservers();\n"
              "                this.observers_[source] = observer;\n"
              "            }\n"
              "            observer.ObserveEvents(target, targetCallback);\n"
              "        }\n"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            instance_id_t source,\n"
              "            object target)\n"
              "        {\n"
              "            InstanceObservers observer = null;\n"
              "            if (!observers_.TryGetValue(source, out observer)) return; \n"
              "            observer.ObserveEventsCancel(target);\n"
              "        }\n"
              "\n"
              "        public System.Collections.Generic.List<Target> GetTargets(instance_id_t source)\n"
              "        {\n"
              "            InstanceObservers observer;\n"
              "            if (observers_.TryGetValue(source, out observer)) { return observer.Targets; }\n"
              "            return null;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private class Structs\n"
              "    {\n"
              "        public System.Collections.Generic.Dictionary<string, StructObservers> observers_ = new System.Collections.Generic.Dictionary<string, StructObservers>();\n"
              "\n"
              "        public void ObserveEvents(\n"
              "            string className,\n"
              "            instance_id_t source,\n"
              "            object target,\n"
              "            FireEventDelegate targetCallback)\n"
              "        {\n"
              "            StructObservers observer = null;\n"
              "            if (!observers_.TryGetValue(className, out observer))\n"
              "            {\n"
              "                observer = new StructObservers();\n"
              "                this.observers_[className] = observer;\n"
              "            }\n"
              "            observer.ObserveEvents(source, target, targetCallback);\n"
              "        }\n"
              "\n"
              "        public void ObserveEventsCancel(\n"
              "            string className,\n"
              "            instance_id_t source,\n"
              "            object target)\n"
              "        {\n"
              "            StructObservers observer = null;\n"
              "            if (!observers_.TryGetValue(className, out observer)) return;\n"
              "            observer.ObserveEventsCancel(source, target);\n"
              "        }\n"
              "\n"
              "        public System.Collections.Generic.List<Target> GetTargets(\n"
              "            string className,\n"
              "            instance_id_t source)\n"
              "        {\n"
              "            StructObservers observer;\n"
              "            if (observers_.TryGetValue(className, out observer)) {\n"
              "                return observer.GetTargets(source);\n"
              "            }\n"
              "            return null;\n"
              "        }\n"
              "    }\n"
              "\n"
              "    private System.Collections.Generic.Dictionary<string, Structs> observers_ = new System.Collections.Generic.Dictionary<string, Structs>();\n"
              "    private $APINAMESPACE$.WrapperCallbackFunction _handlerFunction = HandleEventCallback;\n"
              "\n"
              "    public static EventManager Singleton { get { return singleton_; } }\n"
              "\n"
              "    private EventManager()\n"
              "    {\n"
              "        $APINAMESPACE$.callback_wrapperInstall(_handlerFunction);\n"
              "    }\n"
              "\n"
              "    public void ObserveEvents(\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        instance_id_t source,\n"
              "        object target,\n"
              "        FireEventDelegate targetCallback)\n"
              "    {\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer = null;\n"
              "            if (!observers_.TryGetValue(namespaceName, out observer))\n"
              "            {\n"
              "                observer = new Structs();\n"
              "                this.observers_[namespaceName] = observer;\n"
              "            }\n"
              "            observer.ObserveEvents(className, source, target, targetCallback);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    public void ObserveEventsCancel(\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        instance_id_t source,\n"
              "        object target)\n"
              "    {\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer = null;\n"
              "            if (!observers_.TryGetValue(namespaceName, out observer)) return;\n"
              "            observer.ObserveEventsCancel(className, source, target);\n"
              "        }\n"
              "    }\n"
              "\n"
              "    public void HandleEvent(\n"
              "        callback_event_t handle,\n"
              "        string namespaceName,\n"
              "        string className,\n"
              "        string methodName,\n"
              "        instance_id_t source)\n"
              "    {\n"
              "        System.Collections.Generic.List<Target> targets = null;\n"
              "        lock (lock_)\n"
              "        {\n"
              "            Structs observer;\n"
              "            if (observers_.TryGetValue(namespaceName, out observer)) {\n"
              "                targets = observer.GetTargets(className, source);\n"
              "            }\n"
              "        }\n"
              "        if (null == targets) return;\n"
              "        foreach (Target target in targets) { target.FireEvent(methodName, handle); }\n"
              "    }\n"
              "\n"
              "#if (__IOS__ && __UNIFIED__) || (__MACOS__)\n"
              "    [ObjCRuntime.MonoPInvokeCallback(typeof($APINAMESPACE$.WrapperCallbackFunction))]\n"
              "#endif\n"
              "    private static void HandleEventCallback(callback_event_t handle)\n"
              "    {\n"
              "        if (System.IntPtr.Zero == handle) return;\n"
              "        string namespaceName = $APINAMESPACE$.callback_event_get_namespace(handle);\n"
              "        string className = $APINAMESPACE$.callback_event_get_class(handle);\n"
              "        string methodName = $APINAMESPACE$.callback_event_get_method(handle);\n"
              "        var instanceId = $APINAMESPACE$.callback_event_get_source_instance_id(handle);\n"
              "        Singleton.HandleEvent(handle, namespaceName, className, methodName, instanceId);\n"
              "        $APINAMESPACE$.callback_event_wrapperDestroy(handle);\n"
              "    }\n"
              "}\n"
              "\n"
              "public static void ObserveEvents(\n"
              "    string namespaceName,\n"
              "    string className,\n"
              "    instance_id_t source,\n"
              "    object target,\n"
              "    FireEventDelegate targetCallback\n"
              "    )\n"
              "{\n"
              "    EventManager.Singleton.ObserveEvents(namespaceName, className, source, target, targetCallback);\n"
              "}\n"
              "\n"
              "public static void ObserveEventsCancel(\n"
              "    string namespaceName,\n"
              "    string className,\n"
              "    instance_id_t source,\n"
              "    object target)\n"
              "{\n"
              "    EventManager.Singleton.ObserveEventsCancel(namespaceName, className, source, target);\n"
              "}\n"
              ;

              String apiNamespaceStr = "Wrapper." + GenerateStructCx::fixName(apiFile.project_->getMappingName()) + ".Api";

              String callbackHelpersStr(callbackHelpers);
              callbackHelpersStr.replaceAll("$APINAMESPACE$", apiNamespaceStr);

              GenerateHelper::insertBlob(ss, indentStr, callbackHelpersStr);

              apiFile.endHelpersRegion("Callback and Event helpers");
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiExceptions(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("exception_handle_t", "System.IntPtr");
          apiFile.usingTypedef("instance_id_t", "System.IntPtr");
          apiFile.usingTypedef("const_char_star_t", "System.IntPtr");

          {
            apiFile.startRegion("Exception API helpers");

            auto &ss = apiFile.structSS_;

            static const char *exceptionHelpers =
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static exception_handle_t exception_wrapperCreate_exception();\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static void exception_wrapperDestroy(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static instance_id_t exception_wrapperInstanceId(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "[return: MarshalAs(UseBoolMashal)]\n"
              "public extern static bool exception_hasException(exception_handle_t handle);\n"
              "\n"
              "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n"
              "public extern static const_char_star_t exception_what_actual(exception_handle_t handle);\n"
              "\n"
              "public static string exception_what(exception_handle_t handle)\n"
              "{\n"
              "    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(exception_what_actual(handle));\n"
              "}\n"
              "\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, exceptionHelpers);
          }

          {
            apiFile.startHelpersRegion("Exception helpers");

            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "public static System.Exception exception_FromC(exception_handle_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (!" << getApiPath(apiFile) << ".exception_hasException(handle)) return null;\n";
          }

          prepareApiExceptions(apiFile, "InvalidArgument");
          prepareApiExceptions(apiFile, "BadState");
          prepareApiExceptions(apiFile, "NotImplemented");
          prepareApiExceptions(apiFile, "NotSupported");
          prepareApiExceptions(apiFile, "UnexpectedError");
          prepareApiExceptions(apiFile, "Exception");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "    return new System.Exception(" << getApiPath(apiFile) << ".exception_what(handle));\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static System.Exception exception_AdoptFromC(exception_handle_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = exception_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".exception_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";

            apiFile.endHelpersRegion("Exception helpers");
          }

          {
            apiFile.endRegion("Exception API helpers");
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiExceptions(
                                                        ApiFile &apiFile,
                                                        const String &exceptionName
                                                        )
        {
          auto &indentStr = apiFile.indent_;
         
          auto context = apiFile.global_->toContext()->findType("::zs::exceptions::" + exceptionName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
            ss << indentStr << "public extern static bool exception_is_" << exceptionName << "(exception_handle_t handle);\n";
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "    if (" << getApiPath(apiFile) << ".exception_is_" << exceptionName << "(handle)) return new " << fixCsType(contextStruct) << "(" << getApiPath(apiFile) << ".exception_what(handle)";
            if ("BadState" == exceptionName)  ss << ", unchecked((System.Int32)E_NOT_VALID_STATE)";
            if ("UnexpectedError" == exceptionName)  ss << ", unchecked((System.Int32)E_UNEXPECTED)";
            ss << ");\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBoxing(ApiFile &apiFile)
        {
          apiFile.startRegion("Boxing API helpers");
          apiFile.startHelpersRegion("Boxing helpers");

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_bool);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uchar);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_schar);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ushort);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sshort);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ulong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_slong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ulonglong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_slonglong);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint8);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint8);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint16);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint16);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint32);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint32);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_uint64);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_sint64);

          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_byte);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_word);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_dword);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_qword);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_double);
          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_ldouble);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float32);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_float64);

          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_pointer);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_binary);
          //prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_size);

          prepareApiBoxing(apiFile, IEventingTypes::PredefinedTypedef_string);

          apiFile.endRegion("Boxing API helpers");
          apiFile.endHelpersRegion("Boxing helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBoxing(
                                                    ApiFile &apiFile,
                                                    const IEventingTypes::PredefinedTypedefs basicType
                                                    )
        {
          auto &indentStr = apiFile.indent_;

          String cTypeStr = GenerateStructC::fixCType(basicType);
          String csTypeStr = fixCCsType(basicType);
          String boxedTypeStr = "box_" + cTypeStr;

          apiFile.usingTypedef(boxedTypeStr, "System.IntPtr");

          auto pathStr = GenerateStructC::fixBasicType(basicType);
          if (!apiFile.hasBoxing(pathStr)) return;

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(" << getParamMarshal(basicType) << csTypeStr << " value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void box_" << cTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t box_" << cTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static bool box_" << cTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << getReturnMarshal(basicType, indentStr);
            ss << indentStr << "public extern static " << csTypeStr << " box_" << cTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void box_" << cTypeStr << "_set_value(" << boxedTypeStr << " handle, " << getParamMarshal(basicType) << csTypeStr << " value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            if (IEventingTypes::PredefinedTypedef_binary == basicType) {
              ss << indentStr << "public static byte[] box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    var binaryHandle = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "    var result = " << cTypeStr << "_FromC(binaryHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(binaryHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static byte[] box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(byte[] value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    var tempHandle = " << cTypeStr << "_ToC(value);\n";
              ss << indentStr << "    var result = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
            } else if (IEventingTypes::getBaseType(basicType) == IEventingTypes::BaseType_String) {
              ss << indentStr << "public static string box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    var tempHandle = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "    var result = " << cTypeStr << "_FromC(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static string box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(string value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    var tempHandle = " << cTypeStr << "_ToC(value);\n";
              ss << indentStr << "    var result = " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << cTypeStr << "_wrapperDestroy(tempHandle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
            } else {
              ss << indentStr << "public static " << fixCCsType(basicType) << "? box_" << cTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << ".box_" << cTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << ".box_" << cTypeStr << "_get_value(handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << fixCCsType(basicType) << "? box_" << cTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = box_" << cTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " box_" << cTypeStr << "_ToC(" << fixCCsType(basicType) << "? value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr <<"WithValue(value.Value);\n";
              ss << indentStr << "}\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiString(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("string_t", "System.IntPtr");
          apiFile.usingTypedef("const_char_star_t", "System.IntPtr");

          apiFile.startRegion("String API helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static string_t string_t_wrapperCreate_string();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static string_t string_t_wrapperCreate_stringWithValue([MarshalAs(UseStringMarshal)] string value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void string_t_wrapperDestroy(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t string_t_wrapperInstanceId(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static const_char_star_t string_t_get_value_actual(string_t handle);\n";
            ss << "\n";
            ss << indentStr << "public static string string_t_get_value(string_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(string_t_get_value_actual(handle));\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void string_t_set_value(string_t handle, [MarshalAs(UseStringMarshal)] string value);\n";
            ss << "\n";
          }
          apiFile.endRegion("String API helpers");

          apiFile.startHelpersRegion("String helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static string string_t_FromC(string_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".string_t_get_value(handle);\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static string string_t_AdoptFromC(string_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = string_t_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".string_t_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static string_t string_t_ToC(string value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".string_t_wrapperCreate_stringWithValue(value);\n";
            ss << indentStr << "}\n";
          }

          apiFile.endHelpersRegion("String helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiBinary(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion("Binary API helpers");

          apiFile.usingTypedef("binary_t", "System.IntPtr");
          apiFile.usingTypedef("ConstBytePtr", "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_t binary_t_wrapperCreate_binary_t();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_t binary_t_wrapperCreate_binary_tWithValue(byte [] value, binary_size_t size);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void binary_t_wrapperDestroy(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t binary_t_wrapperInstanceId(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static ConstBytePtr binary_t_get_value(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static binary_size_t binary_t_get_size(binary_t handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void binary_t_set_value(binary_t handle, byte [] value, binary_size_t size);\n";
            ss << "\n";
          }
          apiFile.endRegion("Binary API helpers");

          apiFile.startHelpersRegion("Binary helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static byte[] binary_t_FromC(binary_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
            ss << indentStr << "    var length = " << getApiPath(apiFile) << ".binary_t_get_size(handle);\n";
            ss << indentStr << "    byte[] buffer = new byte[length];\n";
            ss << indentStr << "    Marshal.Copy(" << getApiPath(apiFile) << ".binary_t_get_value(handle), buffer, 0, checked((int)length));\n";
            ss << indentStr << "    return buffer;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static byte[] binary_t_AdoptFromC(binary_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    var result = binary_t_FromC(handle);\n";
            ss << indentStr << "    " << getApiPath(apiFile) << ".binary_t_wrapperDestroy(handle);\n";
            ss << indentStr << "    return result;\n";
            ss << indentStr << "}\n";
            ss << "\n";
            ss << indentStr << "public static binary_t binary_t_ToC(byte[] value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
            ss << indentStr << "    return " << getApiPath(apiFile) << ".binary_t_wrapperCreate_binary_tWithValue(value, checked((ulong)value.Length));\n";
            ss << indentStr << "}\n";
          }

          apiFile.endHelpersRegion("Binary helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiDuration(ApiFile &apiFile)
        {
          apiFile.startRegion("Time API helpers");
          apiFile.startHelpersRegion("Time helpers");

          prepareApiDuration(apiFile, "Time");
          prepareApiDuration(apiFile, "Days");
          prepareApiDuration(apiFile, "Hours");
          prepareApiDuration(apiFile, "Seconds");
          prepareApiDuration(apiFile, "Minutes");
          prepareApiDuration(apiFile, "Milliseconds");
          prepareApiDuration(apiFile, "Microseconds");
          prepareApiDuration(apiFile, "Nanoseconds");

          apiFile.endRegion("Time API helpers");
          apiFile.endHelpersRegion("Time helpers");
        }


        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiDuration(
                                                      ApiFile &apiFile,
                                                      const String &durationType
                                                      )
        {
          auto &indentStr = apiFile.indent_;

          auto durationContext = apiFile.global_->toContext()->findType("::zs::" + durationType);
          if (!durationContext) return;

          auto durationStruct = durationContext->toStruct();
          if (!durationStruct) return;

          String cTypeStr = GenerateStructC::fixCType(durationStruct);

          apiFile.usingTypedef(cTypeStr, "System.IntPtr");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << cTypeStr << " zs_" << durationType << "_wrapperCreate_" << durationType << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << cTypeStr << " zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(long value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void zs_" << durationType << "_wrapperDestroy(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t zs_" << durationType << "_wrapperInstanceId(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "long zs_" << durationType << "_get_value(" << cTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void zs_" << durationType << "_set_value(" << cTypeStr << " handle, long value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static " << fixCsPathType(durationContext) << " zs_" << durationType << "_FromC(" << cTypeStr << " handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    ";
            if ("Time" == durationType) {
              ss << "return System.DateTimeOffset.FromFileTime(" << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle));\n";
            } else if ("Nanoseconds" == durationType) {
              ss << "return System.TimeSpan.FromTicks(System.TimeSpan.TicksPerMillisecond * " << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle) / 1000 / 1000);\n";
            } else if ("Microseconds" == durationType) {
              ss << "return System.TimeSpan.FromTicks(System.TimeSpan.TicksPerMillisecond * " << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle) / 1000);\n";
            } else {
              ss << "return System.TimeSpan.From" << durationType << "(" << getApiPath(apiFile) << ".zs_" << durationType << "_get_value(handle));\n";
            }
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << fixCsPathType(durationContext) << " zs_" << durationType << "_AdoptFromC(" << cTypeStr << " handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "  var result = zs_" << durationType << "_FromC(handle);\n";
            ss << indentStr << "  " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperDestroy(handle);\n";
            ss << indentStr << "  return result;\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << cTypeStr << " zs_" << durationType << "_ToC(" << fixCsPathType(durationContext) << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    ";
            if ("Time" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.ToFileTime());\n";
            } else if ("Nanoseconds" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.Ticks * 1000 * 1000 / System.TimeSpan.TicksPerMillisecond);\n";
            } else if ("Microseconds" == durationType) {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value.Ticks * 1000 / System.TimeSpan.TicksPerMillisecond);\n";
            } else {
              ss << "return " << getApiPath(apiFile) << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(unchecked((long)(value.Total" << durationType << ")));\n";
            }
            ss << indentStr << "}\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiList(
                                                  ApiFile &apiFile,
                                                  const String &listOrSetStr
                                                  )
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion(listOrSetStr + " API helpers");
          apiFile.startHelpersRegion(listOrSetStr + " helpers");

          bool isMap = ("map" == listOrSetStr);
          bool isList = ("list" == listOrSetStr);
          auto context = apiFile.global_->toContext()->findType("::std::" + listOrSetStr);
          if (!context) return;

          auto structType = context->toStruct();
          if (!structType) return;

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
            
            apiFile.usingTypedef("iterator_handle_t", "System.IntPtr");
            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(keyType);
            apiFile.usingTypedef(listType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(templatedStructType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "();\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static void " << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static instance_id_t " << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              if (isMap) {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentStr << "public extern static " << "void " << GenerateStructC::fixType(templatedStructType) << "_insert(" << GenerateStructC::fixCType(templatedStructType) << " handle, " << getParamMarshal(keyType) << fixCCsType(keyType) << " key, " << getParamMarshal(listType) << fixCCsType(listType) << " value);\n";
              } else {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentStr << "public extern static " << "void " << GenerateStructC::fixType(templatedStructType) << "_insert(" << GenerateStructC::fixCType(templatedStructType) << " handle, " << getParamMarshal(listType) << fixCCsType(listType) << " value);\n";
              }

              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static iterator_handle_t " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin(" << GenerateStructC::fixCType(templatedStructType) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static void " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterator_handle_t iterHandle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(" << GenerateStructC::fixCType(templatedStructType) << " handle, iterator_handle_t iterHandle);\n";
              if (isMap) {
                ss << "\n";
                ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << getReturnMarshal(keyType, indentStr);
                ss << indentStr << "public extern static " << fixCCsType(keyType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey(iterator_handle_t iterHandle);\n";
              }
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << getReturnMarshal(listType, indentStr);
              ss << indentStr << "public extern static " << fixCCsType(listType) << " " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue(iterator_handle_t iterHandle);\n";
              ss << "\n";
            }

            {
              auto &ss = apiFile.helpersSS_;

              ss << "\n";
              ss << indentStr << "public static " <<  fixCsPathType(templatedStructType) << " " << GenerateStructC::fixType(templatedStructType) << "_FromC(" << GenerateStructC::fixCType(templatedStructType) << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    var result = new ";
              if (isList) {
                ss << "System.Collections.Generic.List< " << fixCsPathType(listType) << " >";
              } else if (isMap) {
                ss << "System.Collections.Generic.Dictionary< " << fixCsPathType(keyType) << ", " << fixCsPathType(listType) << " >";
              } else {
                ss << "System.Collections.Generic.Dictionary< " << fixCsPathType(listType) << ", object >";
              }
              ss << "();\n";
              ss << indentStr << "    var iterHandle = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin(handle);\n";
              ss << indentStr << "    while (!" << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(handle, iterHandle))\n";
              ss << indentStr << "    {\n";
              if (isMap) {
                ss << indentStr << "        var cKey = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey(iterHandle);\n";
              }
              ss << indentStr << "        var cValue = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue(iterHandle);\n";

              if (isMap) {
                ss << indentStr << "        var csKey = " << getAdoptFromCMethod(apiFile, false, listType) << "(cKey);\n";
              }
              ss << indentStr << "        var csValue = " << getAdoptFromCMethod(apiFile, false, listType) << "(cValue);\n";
              if (isList) {
                ss << indentStr << "        result.Add(csValue);\n";
              } else if (isMap) {
                ss << indentStr << "        result.Add(csKey, csValue);\n";
              } else {
                ss << indentStr << "        result.Add(csValue, null);\n";
              }
              ss << indentStr << "        " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterHandle);\n";

              ss << indentStr << "    }\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << fixCsPathType(templatedStructType) << " " << GenerateStructC::fixType(templatedStructType) << "_AdoptFromC(" << GenerateStructC::fixCType(templatedStructType) << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    var result = " << GenerateStructC::fixType(templatedStructType) << "_FromC(handle);\n";
              ss << indentStr << "    " << getDestroyCMethod(apiFile, false, templatedStructType) << "(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << GenerateStructC::fixCType(templatedStructType) << " " << GenerateStructC::fixType(templatedStructType) << "_ToC(" << fixCsPathType(templatedStructType) << " values)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == values) return System.IntPtr.Zero;\n";
              ss << indentStr << "    var handle = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << listOrSetStr << "();\n";
              if (isList) {
                ss << indentStr << "    foreach (var value in values)\n";
              } else if (isMap) {
                ss << indentStr << "    foreach (System.Collections.Generic.KeyValuePair< " << fixCsPathType(keyType) << ", " <<  fixCsPathType(listType) << " > value in values)\n";
              } else {
                ss << indentStr << "    foreach (System.Collections.Generic.KeyValuePair< " << fixCsPathType(listType) << ", object > value in values)\n";
              }
              ss << indentStr << "    {\n";
              if (isList) {
                ss << indentStr << "        var cValue = " << getToCMethod(apiFile, false, listType) << "(value);\n";
                ss << indentStr << "        " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_insert(handle, cValue);\n";
              } else if (isMap) {
                ss << indentStr << "        var cKey = " << getToCMethod(apiFile, false, keyType) << "(value.Key);\n";
                ss << indentStr << "        var cValue = " << getToCMethod(apiFile, false, listType) << "(value.Value);\n";
                ss << indentStr << "        " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) <<  "_insert(handle, cKey, cValue);\n";
                auto destroyStr = getDestroyCMethod(apiFile, false, keyType);
                if (destroyStr.hasData()) {
                  ss << indentStr << "        " << destroyStr << "(cKey);\n";
                }
              } else {
                ss << indentStr << "        var cValue = " << getToCMethod(apiFile, false, listType) << "(value.Key);\n";
                ss << indentStr << "        " << getApiPath(apiFile) << "." << GenerateStructC::fixType(templatedStructType) << "_insert(handle, cValue);\n";
              }
              {
                auto destroyStr = getDestroyCMethod(apiFile, false, listType);
                if (destroyStr.hasData()) {
                  ss << indentStr << "        " << destroyStr << "(cValue);\n";
                }
              }
              ss << indentStr << "    }\n";
              ss << indentStr << "    return handle;\n";
              ss << indentStr << "}\n";

            }
          }
          apiFile.endRegion(listOrSetStr + " API helpers");
          apiFile.endHelpersRegion(listOrSetStr + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiSpecial(
                                                     ApiFile &apiFile,
                                                     const String &specialName
                                                     )
        {
          auto &indentStr = apiFile.indent_;

          bool isPromise = "Promise" == specialName;
          bool isAny = "Any" == specialName;

          auto context = apiFile.global_->toContext()->findType("::zs::" + specialName);
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.usingTypedef(GenerateStructC::fixCType(contextStruct), "System.IntPtr");

          apiFile.startRegion(specialName + " API helpers");
          apiFile.startHelpersRegion(specialName + " helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void zs_" << specialName << "_wrapperDestroy(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t zs_" << specialName << "_wrapperInstanceId(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            if (isPromise) {
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static event_observer_t zs_" << specialName << "_wrapperObserveEvents(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static ulong zs_" << specialName << "_get_id(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isSettled(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isResolved(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
              ss << indentStr << "public extern static bool zs_" << specialName << "_isRejected(" << GenerateStructC::fixCType(contextStruct) << " handle);\n";
            }
            ss << "\n";
          }

          if (isPromise) {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";

            static const char *promiseAdopt =
              "public static System.Threading.Tasks.Task zs_Promise_AdoptFromC(zs_Promise_t handle)\n"
              "{\n"
              "    return System.Threading.Tasks.Task.Run(() => {\n"
              "        if (System.IntPtr.Zero == handle) return null;\n"
              "\n"
              "        var target = new object();\n"
              "        var tcs = new System.Threading.Tasks.TaskCompletionSource<object>();\n"
              "        var instanceId = $API$.zs_Promise_wrapperInstanceId(handle);\n"
              "\n"
              "        ObserveEvents(\"::zs\", \"Promise\", instanceId, target, (object callbackTarget, string methodName, callback_event_t callbackHandle) =>\n"
              "        {\n"
              "            bool resolved = $API$.zs_Promise_isResolved(handle);\n"
              "            if (resolved) {\n"
              "                tcs.SetResult(null);\n"
              "                return;\n"
              "            }\n"
              "            var exception = zs_PromiseRejectionReason_FromC(handle);\n"
              "            if (null != exception) {\n"
              "                tcs.SetException(exception);\n"
              "            } else {\n"
              "                tcs.SetCanceled();\n"
              "            }\n"
              "        });\n"
              "\n"
              "        var eventObserverHandle = $API$.zs_Promise_wrapperObserveEvents(handle);\n"
              "\n"
              "        tcs.Task.Wait();\n"
              "\n"
              "        $API$.callback_wrapperObserverDestroy(eventObserverHandle);\n"
              "        $API$.zs_Promise_wrapperDestroy(handle);\n"
              "\n"
              "        return tcs.Task.Result;\n"
              "    });\n"
              "}\n"
              ;
            String promiseAdoptStr(promiseAdopt);
            promiseAdoptStr.replaceAll("$API$", getApiPath(apiFile));
            GenerateHelper::insertBlob(ss, indentStr, promiseAdoptStr);
            ss << indentStr << "\n";
          } else if (isAny) {
            {
              auto &ss = apiFile.helpersSS_;

              String cTypeStr = GenerateStructC::fixCType(contextStruct);

              ss << "\n";
              ss << indentStr << "public static " << fixCsPathType(contextStruct) << " " << GenerateStructC::fixType(contextStruct) << "_FromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    return (object)handle;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << fixCsPathType(contextStruct) << " " << GenerateStructC::fixType(contextStruct) << "_AdoptFromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "  var result = " << GenerateStructC::fixType(contextStruct) << "_FromC(handle);\n";
              ss << indentStr << "  " << getDestroyCMethod(apiFile, false, contextStruct) << "(handle);\n";
              ss << indentStr << "  return result;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << cTypeStr << " " << GenerateStructC::fixType(contextStruct) << "_ToC(" << fixCsPathType(contextStruct) << " value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    return (" << cTypeStr << ")value;\n";
              ss << indentStr << "}\n";
            }
          }


          apiFile.endRegion(specialName + " API helpers");
          apiFile.endHelpersRegion(specialName + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::preparePromiseWithValue(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          auto context = apiFile.global_->toContext()->findType("::zs::PromiseWith");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.startRegion("PromiseWith API helpers");
          apiFile.startHelpersRegion("PromiseWith helpers");

          for (auto iter = contextStruct->mTemplatedStructs.begin(); iter != contextStruct->mTemplatedStructs.end(); ++iter)
          {
            auto templatedStructType = (*iter).second;
            if (!templatedStructType) continue;

            TypePtr promiseType;
            auto iterArg = templatedStructType->mTemplateArguments.begin();
            if (iterArg != templatedStructType->mTemplateArguments.end()) {
              promiseType = (*iterArg);
            }

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(promiseType) << " zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
            {
              auto &ss = apiFile.helpersSS_;
              ss << "\n";

              static const char *promiseAdopt =
                "public static System.Threading.Tasks.Task< $RESOLVECSTYPE$ > $ZSPROMISEWITHTYPE$_AdoptFromC($ZSPROMISEWITHCTYPE$ handle)\n"
                "{\n"
                "    return System.Threading.Tasks.Task.Run< $RESOLVECSTYPE$ >(() => {\n"
                "        if (System.IntPtr.Zero == handle) return null;\n"
                "\n"
                "        var target = new object();\n"
                "        var tcs = new System.Threading.Tasks.TaskCompletionSource< $RESOLVECSTYPE$ >();\n"
                "        var instanceId = $API$.zs_Promise_wrapperInstanceId(handle);\n"
                "\n"
                "        ObserveEvents(\"::zs\", \"Promise\", instanceId, target, (object callbackTarget, string methodName, callback_event_t callbackHandle) =>\n"
                "        {\n"
                "            bool resolved = $API$.zs_Promise_isResolved(handle);\n"
                "            if (resolved) {\n"
                "                tcs.SetResult($ADOPTMETHOD$($RESOLVEMETHOD$(handle)));\n"
                "                return;\n"
                "            }\n"
                "            var exception = zs_PromiseRejectionReason_FromC(handle);\n"
                "            if (null != exception) {\n"
                "                tcs.SetException(exception);\n"
                "            } else {\n"
                "                tcs.SetCanceled();\n"
                "            }\n"
                "        });\n"
                "\n"
                "        var eventObserverHandle = $API$.zs_Promise_wrapperObserveEvents(handle);\n"
                "\n"
                "        tcs.Task.Wait();\n"
                "\n"
                "        $API$.callback_wrapperObserverDestroy(eventObserverHandle);\n"
                "        $API$.zs_Promise_wrapperDestroy(handle);\n"
                "\n"
                "        return tcs.Task.Result;\n"
                "    });\n"
                "}\n"
                ;
              String promiseAdoptStr(promiseAdopt);
              promiseAdoptStr.replaceAll("$API$", getApiPath(apiFile));
              promiseAdoptStr.replaceAll("$ZSPROMISEWITHTYPE$", GenerateStructC::fixType(templatedStructType));
              promiseAdoptStr.replaceAll("$ZSPROMISEWITHCTYPE$", GenerateStructC::fixCType(templatedStructType));
              promiseAdoptStr.replaceAll("$RESOLVECSTYPE$", fixCsPathType(promiseType));
              promiseAdoptStr.replaceAll("$ADOPTMETHOD$", getAdoptFromCMethod(apiFile, false, promiseType));
              promiseAdoptStr.replaceAll("$RESOLVEMETHOD$", String(getApiPath(apiFile) + ".zs_PromiseWith_resolveValue_" + GenerateStructC::fixType(promiseType)));

              GenerateHelper::insertBlob(ss, indentStr, promiseAdoptStr);
              ss << indentStr << "\n";
            }
          }

          apiFile.endRegion("PromiseWith API helpers");
          apiFile.endHelpersRegion("PromiseWith helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::preparePromiseWithRejectionReason(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          auto context = apiFile.global_->toContext()->findType("::zs::PromiseRejectionReason");
          if (!context) return;

          auto contextStruct = context->toStruct();
          if (!contextStruct) return;

          apiFile.startRegion("PromiseWithRejectionReason API helpers");
          apiFile.startHelpersRegion("PromiseWithRejectionReason helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static System.Exception zs_PromiseRejectionReason_FromC(zs_Promise_t handle)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
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

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "System.IntPtr");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentStr << "public extern static " << GenerateStructC::fixCType(promiseType) << " zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(zs_Promise_t handle);\n";
            }
            {
              auto &ss = apiFile.helpersSS_;
              ss << indentStr << "    { var result = " << getAdoptFromCMethod(apiFile, false, promiseType) << "(" << getApiPath(apiFile) <<  ".zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(handle)); if (null != result) return result; }\n";
            }
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "    return null;\n";
            ss << indentStr << "}\n";
          }

          apiFile.endRegion("PromiseWithRejectionReason API helpers");
          apiFile.endHelpersRegion("PromiseWithRejectionReason helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiNamespace(
                                                       ApiFile &apiFile,
                                                       NamespacePtr namespaceObj
                                                       )
        {
          if (!namespaceObj) return;

          if (namespaceObj == apiFile.global_) {
            apiFile.startRegion("enum API helpers");
            apiFile.startHelpersRegion("enum helpers");
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            prepareApiNamespace(apiFile, subNamespaceObj);
          }
          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareApiStruct(apiFile, subStructObj);
          }
          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareApiEnum(apiFile, subEnumObj);
          }

          if (namespaceObj == apiFile.global_) {
            apiFile.endRegion("enum API helpers");
            apiFile.endHelpersRegion("enum helpers");
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiStruct(
                                                    ApiFile &apiFile,
                                                    StructPtr structObj
                                                    )
        {
          if (!structObj) return;

          for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            prepareApiStruct(apiFile, subStructObj);
          }
          for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
            auto subEnumObj = (*iter).second;
            prepareApiEnum(apiFile, subEnumObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareApiEnum(
                                                  ApiFile &apiFile,
                                                  EnumTypePtr enumObj
                                                  )
        {
          auto &indentStr = apiFile.indent_;

          if (!enumObj) return;
          bool hasBoxing = apiFile.hasBoxing(enumObj->getPathName());

          String cTypeStr = GenerateStructC::fixCType(enumObj);
          String boxedTypeStr = "box_" + cTypeStr;
          String csTypeStr = fixCsPathType(enumObj);

          apiFile.usingTypedef(cTypeStr, fixCsSystemType(enumObj->mBaseType));
          apiFile.usingTypedef(boxedTypeStr, "System.IntPtr");

          if (hasBoxing)
          {
            auto &ss = apiFile.structSS_;
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "();\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << boxedTypeStr << " " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(" << fixCCsType(enumObj->mBaseType) << " value);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static void " << boxedTypeStr << "_wrapperDestroy(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static instance_id_t " << boxedTypeStr << "_wrapperInstanceId(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "[return: MarshalAs(UseBoolMashal)]\n";
            ss << indentStr << "public extern static " << "bool " << boxedTypeStr << "_has_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << fixCCsType(enumObj->mBaseType) << " " << boxedTypeStr << "_get_value(" << boxedTypeStr << " handle);\n";
            ss << "\n";
            ss << indentStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
            ss << indentStr << "public extern static " << "void " << boxedTypeStr << "_set_value(" << boxedTypeStr << " handle, " << fixCCsType(enumObj->mBaseType) << " value);\n";
            ss << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";
            ss << indentStr << "public static " << csTypeStr << " " << cTypeStr << "_FromC(" << cTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << csTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << csTypeStr << " " << cTypeStr << "_AdoptFromC(" << cTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << csTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "public static " << cTypeStr << " " << cTypeStr << "_ToC(" << csTypeStr << " value)\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    return unchecked((" << cTypeStr << ")value);\n";
            ss << indentStr << "}\n";

            if (hasBoxing) {
              ss << "\n";
              ss << indentStr << "public static " << csTypeStr << "? " << boxedTypeStr << "_FromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    if (!" << getApiPath(apiFile) << "." << boxedTypeStr << "_has_value(handle)) return null;\n";
              ss << indentStr << "    return unchecked((" << csTypeStr << ")" << getApiPath(apiFile) << "." << boxedTypeStr << "_get_value(handle));\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << csTypeStr << "? " << boxedTypeStr << "_AdoptFromC(" << boxedTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    var result = " << boxedTypeStr << "_FromC(handle);\n";
              ss << indentStr << "    " << getApiPath(apiFile) << "." << boxedTypeStr << "_wrapperDestroy(handle);\n";
              ss << indentStr << "    return result;\n";
              ss << indentStr << "}\n";

              ss << "\n";
              ss << indentStr << "public static " << boxedTypeStr << " " << boxedTypeStr << "_ToC(" << csTypeStr << "? value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << "." << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(unchecked((" << cTypeStr << ")value.Value));\n";
              ss << indentStr << "}\n";
              ss << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareEnumFile(EnumFile &enumFile)
        {
          {
            auto &ss = enumFile.usingNamespaceSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
          }

          prepareEnumNamespace(enumFile, enumFile.global_);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::prepareEnumNamespace(
                                                        EnumFile &enumFile,
                                                        NamespacePtr namespaceObj
                                                        )
        {
          auto &indentStr = enumFile.indent_;
          auto &ss = enumFile.namespaceSS_;

          if (!namespaceObj) return;

          if (!namespaceObj->isGlobal()) {
            ss << "\n";
            ss << indentStr << "namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            ss << indentStr << "{\n";
            enumFile.indentMore();
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;

            if (namespaceObj->isGlobal()) {
              auto name = subNamespaceObj->getMappingName();
              if ("std" == name) continue;
              if ("zs" == name) continue;
            }

            prepareEnumNamespace(enumFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto enumObj = (*iter).second;
            if (!enumObj) continue;

            ss << "\n";
            ss << GenerateHelper::getDocumentation(indentStr + "/// ", enumObj, 80);
            ss << indentStr << "public enum " << GenerateStructCx::fixName(enumObj->getMappingName());
            if (enumObj->mBaseType != PredefinedTypedef_int) {
              ss << " : " << fixCCsType(enumObj->mBaseType);
            }
            ss << "\n";
            ss << indentStr << "{\n";
            enumFile.indentMore();

            bool first = true;
            for (auto iterVal = enumObj->mValues.begin(); iterVal != enumObj->mValues.end(); ++iterVal) {
              auto valueObj = (*iterVal);

              if (!first) ss << ",\n";
              first = false;
              ss << GenerateHelper::getDocumentation(indentStr + "/// ", valueObj, 80);
              ss << indentStr << GenerateStructCx::fixName(valueObj->getMappingName());
              if (valueObj->mValue.hasData()) {
                ss << " = " << valueObj->mValue;
              }
            }
            if (!first) ss << "\n";

            enumFile.indentLess();
            ss << indentStr << "}\n";
          }

          if (!namespaceObj->isGlobal()) {
            enumFile.indentLess();
            ss << indentStr << "} // namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processNamespace(
                                                    ApiFile &apiFile,
                                                    NamespacePtr namespaceObj
                                                    )
        {
          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            processNamespace(apiFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processStruct(apiFile, subStructObj);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processStruct(
                                                 ApiFile &apiFile,
                                                 StructPtr structObj
                                                 )
        {
          typedef std::list<NamespacePtr> NamespaceList;

          if (!structObj) return;
          if (GenerateHelper::isBuiltInType(structObj)) return;
          if (structObj->mGenerics.size() > 0) return;

          StructFile structFile(apiFile, structObj);
          structFile.project_ = apiFile.project_;
          structFile.global_ = apiFile.global_;
          structFile.fileName_ = UseHelper::fixRelativeFilePath(apiFile.fileName_, "net_" + GenerateStructC::fixType(structObj) + ".cs");

          {
            auto &ss = structFile.usingNamespaceSS_;
            ss << "// " ZS_EVENTING_GENERATED_BY "\n\n";
          }

          auto &indentStr = structFile.indent_;

          NamespaceList namespaceList;

          {
            auto parent = structObj->getParent();
            while (parent) {
              auto namespaceObj = parent->toNamespace();
              if (namespaceObj) {
                if (!namespaceObj->isGlobal()) {
                  namespaceList.push_front(namespaceObj);
                }
              }
              parent = parent->getParent();
            }
          }

          {
            auto &ss = structFile.namespaceSS_;

            for (auto iter = namespaceList.begin(); iter != namespaceList.end(); ++iter) {
              auto namespaceObj = (*iter);
              ss << indentStr << "namespace " << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
              ss << indentStr << "{\n";
              structFile.indentMore();
            }
          }

          if (structFile.shouldDefineInterface_) {
            auto &ss = structFile.interfaceSS_;
            ss << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);
            ss << indentStr << "public interface I" << GenerateStructCx::fixStructName(structObj) << "\n";
            ss << indentStr << "{\n";
          }

          String fixedTypeStr = GenerateStructC::fixType(structObj);
          String cTypeStr = GenerateStructC::fixCType(structObj);
          String csTypeStr = GenerateStructCx::fixStructName(structObj);

          {
            auto &ss = structFile.structSS_;
            ss << GenerateHelper::getDocumentation(indentStr + "/// ", structObj, 80);
            ss << indentStr << "public " << (structFile.isStaticOnly_ ? "static " : "sealed ") << "class " << csTypeStr;

            String indentPlusStr;
            size_t spaceLength = indentStr.length() + strlen("public sealed class  : ") + csTypeStr.length();
            indentPlusStr.reserve(spaceLength);
            for (size_t index = 0; index < spaceLength; ++index) { indentPlusStr += " "; }

            if (structFile.shouldInheritException_) {
              ss << " : System.Exception,\n";
              ss << indentPlusStr;
            } else if (!structFile.isStaticOnly_) {
              ss << " : ";
            }
            ss << (structFile.isStaticOnly_ ? "" : "System.IDisposable");

            if (!structFile.isStaticOnly_) {
              processInheritance(apiFile, structFile, structObj, indentPlusStr);
            }
            ss << "\n";
            ss << indentStr << "{\n";
          }

          apiFile.startRegion(fixCsPathType(structObj));

          structFile.indentMore();

          if (!structFile.isStaticOnly_)
          {
            structFile.usingTypedef(structObj);
            structFile.usingTypedef("instance_id_t", "System.IntPtr");

            {
              auto &ss = structFile.structSS_;
              structFile.startRegion("To / From C routines");
              ss << indentStr << "private class WrapperMakePrivate {}\n";
              ss << indentStr << "private " << cTypeStr << " native_ = System.IntPtr.Zero;\n";
              ss << "\n";
              ss << indentStr << "private " << csTypeStr << "(WrapperMakePrivate ignored, " << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    this.native_ = handle;\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    WrapperObserveEvents();\n";
              }
              ss << indentStr << "}\n";
              ss << "\n";
              ss << "\n";
              ss << indentStr << "public void Dispose()\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    Dispose(true);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "private void Dispose(bool disposing)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == this.native_) return;\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    WrapperObserveEventsCancel();\n";
              }
              ss << indentStr << "    " << getApiPath(apiFile) << "." << fixedTypeStr << "_wrapperDestroy(this.native_);\n";
              ss << indentStr << "    this.native_ = System.IntPtr.Zero;\n";
              ss << indentStr << "    if (disposing) System.GC.SuppressFinalize(this);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "~" << csTypeStr << "()\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    Dispose(false);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << csTypeStr << " " << fixedTypeStr << "_FromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    return new " << csTypeStr << "((WrapperMakePrivate)null, " << getApiPath(apiFile) << "." << fixedTypeStr << "_wrapperClone(handle));\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << csTypeStr << " " << fixedTypeStr << "_AdoptFromC(" << cTypeStr << " handle)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (System.IntPtr.Zero == handle) return null;\n";
              ss << indentStr << "    return new " << csTypeStr << "((WrapperMakePrivate)null, handle);\n";
              ss << indentStr << "}\n";
              ss << "\n";
              ss << indentStr << "internal static " << cTypeStr << " " << fixedTypeStr << "_ToC(" << csTypeStr << " value)\n";
              ss << indentStr << "{\n";
              ss << indentStr << "    if (null == value) return System.IntPtr.Zero;\n";
              ss << indentStr << "    return " << getApiPath(apiFile) << "." << fixedTypeStr << "_wrapperClone(value.native_);\n";
              ss << indentStr << "}\n";
              structFile.endRegion("To / From C routines");
            }
            {
              apiFile.usingTypedef(structObj);
              apiFile.usingTypedef("instance_id_t", "System.IntPtr");

              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.structSS_;
              ss << "\n";
              ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentApiStr << "public extern static " << cTypeStr << " " << fixedTypeStr << "_wrapperClone(" << cTypeStr << " handle);\n";
              ss << "\n";
              ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentApiStr << "public extern static void " << fixedTypeStr << "_wrapperDestroy(" << cTypeStr << " handle);\n";
              ss << "\n";
              ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              ss << indentApiStr << "public extern static instance_id_t " << fixedTypeStr << "_wrapperInstanceId(" << cTypeStr << " handle);\n";
              if (structFile.hasEvents_) {
                apiFile.usingTypedef("event_observer_t", "System.IntPtr");
                ss << "\n";
                ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentApiStr << "public extern static event_observer_t " << fixedTypeStr << "_wrapperObserveEvents(" << cTypeStr << " handle);\n";
              }
            }
            {
              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.helpersSS_;
              ss << "\n";
              ss << indentApiStr << "public static " << fixCsPathType(structObj) << " " << fixedTypeStr << "_FromC(" << cTypeStr << " handle)\n";
              ss << indentApiStr << "{\n";
              ss << indentApiStr << "    return " << fixCsPathType(structObj) << "." << fixedTypeStr << "_FromC(handle);\n";
              ss << indentApiStr << "}\n";
              ss << "\n";
              ss << indentApiStr << "public static " << fixCsPathType(structObj) << " " << fixedTypeStr << "_AdoptFromC(" << cTypeStr << " handle)\n";
              ss << indentApiStr << "{\n";
              ss << indentApiStr << "    return " << fixCsPathType(structObj) << "." << fixedTypeStr << "_AdoptFromC(handle);\n";
              ss << indentApiStr << "}\n";
              ss << "\n";
              ss << indentApiStr << "public static " << cTypeStr << " " << fixedTypeStr << "_ToC(" << fixCsPathType(structObj) << " value)\n";
              ss << indentApiStr << "{\n";
              ss << indentApiStr << "    return " << fixCsPathType(structObj) << "." << fixedTypeStr << "_ToC(value);\n";
              ss << indentApiStr << "}\n";
            }
          }

          if (!structFile.isStaticOnly_) {
            auto found = apiFile.derives_.find(structObj->getPathName());
            if (found != apiFile.derives_.end()) {
              auto &structSet = (*found).second;

              std::stringstream relStructSS;

              bool foundRelated = false;
              for (auto iterSet = structSet.begin(); iterSet != structSet.end(); ++iterSet) {
                auto relatedStruct = (*iterSet);
                if (!relatedStruct) continue;
                if (relatedStruct == structObj) continue;

                foundRelated = true;

                apiFile.usingTypedef(relatedStruct);

                {
                  auto &indentApiStr = apiFile.indent_;
                  auto &ss = apiFile.structSS_;
                  ss << "\n";
                  ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                  ss << indentApiStr << "public extern static " << GenerateStructC::fixCType(relatedStruct) << " " << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "(" << GenerateStructC::fixCType(structObj) << " handle);\n";
                }
                {
                  auto &indentStructStr = structFile.indent_;
                  auto &ss = relStructSS;
                  ss << "\n";
                  ss << indentStructStr << "public static " << fixCsType(structObj) << " Cast(" << fixCsPathType(relatedStruct) << " value)\n";
                  ss << indentStructStr << "{\n";
                  ss << indentStructStr << "    if (null == value) return null;\n";
                  ss << indentStructStr << "    var castHandle = " << getToCMethod(apiFile, false, relatedStruct) << "(value);\n";
                  ss << indentStructStr << "    if (System.IntPtr.Zero == castHandle) return null;\n";
                  ss << indentStructStr << "    var originalHandle = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "(castHandle);\n";
                  ss << indentStructStr << "    if (System.IntPtr.Zero == originalHandle) return null;\n";
                  ss << indentStructStr << "    return " << getAdoptFromCMethod(apiFile, false, structObj) << "(originalHandle);\n";
                  ss << indentStructStr << "}\n";
                }
              }

              if (foundRelated) {
                structFile.startRegion("Casting methods");
                structFile.structSS_ << relStructSS.str();
                structFile.endRegion("Casting methods");
              }
            }
          }

          processStruct(apiFile, structFile, structObj, structObj);

          apiFile.endRegion(fixCsPathType(structObj));

          structFile.indentLess();

          if (structFile.shouldDefineInterface_) {
            auto &ss = structFile.interfaceEndSS_;
            ss << indentStr << "}\n";
          }
          {
            auto &ss = structFile.structEndSS_;
            ss << indentStr << "}\n";
          }

          {
            auto &ss = structFile.namespaceEndSS_;
            for (auto iter = namespaceList.rbegin(); iter != namespaceList.rend(); ++iter) {
              auto namespaceObj = (*iter);
              structFile.indentLess();
              ss << indentStr << "} //" << GenerateStructCx::fixName(namespaceObj->getMappingName()) << "\n";
            }
          }

          finalizeBaseFile(structFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processInheritance(
                                                      ApiFile &apiFile,
                                                      StructFile &structFile,
                                                      StructPtr structObj,
                                                      const String &indentStr
                                                      )
        {
          if (!structObj) return;

          if (hasInterface(structObj)) {
            auto &ss = structFile.structSS_;
            ss << ",\n" << indentStr << fixCsPathType(structObj, true);
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedObj = (*iter).second;
            if (!relatedObj) continue;
            processInheritance(apiFile, structFile, relatedObj->toStruct(), indentStr);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processStruct(
                                                 ApiFile &apiFile,
                                                 StructFile &structFile,
                                                 StructPtr rootStructObj,
                                                 StructPtr structObj
                                                 )
        {
          if (!structObj) return;

          if (rootStructObj == structObj) {
            for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
              auto subStructObj = (*iter).second;
              processStruct(apiFile, subStructObj);
            }
            for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
              auto enumObj = (*iter).second;
              processEnum(apiFile, structFile, structObj, enumObj);
            }
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter) {
            auto relatedTypeObj = (*iter).second;
            if (!relatedTypeObj) continue;
            processStruct(apiFile, structFile, rootStructObj, relatedTypeObj->toStruct());
          }

          structFile.startRegion(fixCsPathType(structObj));

          processMethods(apiFile, structFile, rootStructObj, structObj);
          processProperties(apiFile, structFile, rootStructObj, structObj);
          if (rootStructObj == structObj) {
            processEventHandlers(apiFile, structFile, structObj);
          }

          structFile.endRegion(fixCsPathType(structObj));
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEnum(
                                               ApiFile &apiFile,
                                               StructFile &structFile,
                                               StructPtr structObj,
                                               EnumTypePtr enumObj
                                               )
        {
          if (!enumObj) return;

          auto &indentStr = structFile.indent_;
          auto &ss = structFile.structSS_;

          ss << "\n";
          ss << indentStr << "public enum " << GenerateStructCx::fixName(enumObj->getMappingName());
          if (enumObj->mBaseType != PredefinedTypedef_int) {
            ss << " : " << fixCCsType(enumObj->mBaseType);
          }
          ss << "\n";
          ss << indentStr << "{\n";
          structFile.indentMore();

          bool first = true;
          for (auto iter = enumObj->mValues.begin(); iter != enumObj->mValues.end(); ++iter) {
            auto valueObj = (*iter);

            if (!first) ss << ",\n";
            first = false;
            ss << indentStr << GenerateStructCx::fixName(valueObj->getMappingName());
            if (valueObj->mValue.hasData()) {
              ss << " = " << valueObj->mValue;
            }
          }
          if (!first) ss << "\n";

          structFile.indentLess();
          ss << indentStr << "}\n";
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getSpecialMethodPrefix(
                                                            ApiFile &apiFile,
                                                            StructFile &structFile,
                                                            StructPtr rootStructObj,
                                                            StructPtr structObj,
                                                            MethodPtr method
                                                            )
        {
          if (!method) return String();
          if (!structObj) return String();

          String specialName = (GenerateStructCx::fixName(method->getMappingName()));

          if ("ToString" == specialName) return String("override");
          return String();
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processMethods(
                                                  ApiFile &apiFile,
                                                  StructFile &structFile,
                                                  StructPtr rootStructObj,
                                                  StructPtr structObj
                                                  )
        {
          auto &indentStr = structFile.indent_;

          bool foundDllMethod {};

          if (rootStructObj == structObj) {
            if (GenerateHelper::needsDefaultConstructor(rootStructObj)) {
              {
                auto &indentApiStr = apiFile.indent_;
                auto &&ss = apiFile.structSS_;
                ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                apiFile.usingTypedef(rootStructObj);
                ss << indentApiStr << "public extern static " << GenerateStructC::fixCType(structObj) << " " << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << "();\n";
              }
              {
                auto &ss = structFile.structSS_;
                ss << "\n";

                String altNameStr = GenerateStructCx::fixName(rootStructObj->getMappingName());

                ss << indentStr << "public " << altNameStr << "()\n";

                ss << indentStr << "{\n";
                structFile.indentMore();

                structFile.usingTypedef(structObj);
                ss << indentStr << "this.native_ = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << altNameStr << "();\n";

                if (structFile.hasEvents_) {
                  ss << indentStr << "WrapperObserveEvents();\n";
                }

                structFile.indentLess();
                ss << indentStr << "}\n";
              }
            }
          }

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (!method) continue;
            if (method->hasModifier(Modifier_Method_EventHandler)) continue;

            bool isConstructor = method->hasModifier(Modifier_Method_Ctor);
            bool isStatic = method->hasModifier(Modifier_Static);

            bool generateInterface = (!isStatic) && (!isConstructor) && (rootStructObj == structObj) && (structFile.shouldDefineInterface_);

            if (rootStructObj != structObj) {
              if ((isStatic) || (isConstructor)) continue;
            }
            if (method->hasModifier(Modifier_Method_Delete)) continue;

            auto resultCsStr = fixCsPathType(method->hasModifier(Modifier_Optional), method->mResult);
            bool hasResult = "void" != resultCsStr && (!isConstructor);

            String altNameStr = method->getModifierValue(Modifier_AltName);
            if (altNameStr.isEmpty()) {
              altNameStr = method->getMappingName();
            }

            {
              auto &indentApiStr = apiFile.indent_;
              auto &&ss = apiFile.structSS_;

              if (!foundDllMethod) {
                if (rootStructObj != structObj) {
                  ss << "\n";
                  ss << "#if !" << getApiCastRequiredDefine(apiFile) << "\n";
                }
                foundDllMethod = true;
              }
              ss << "\n";
              ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
              if (isConstructor) {
                apiFile.usingTypedef(structObj);
                ss << indentApiStr << "public extern static " << GenerateStructC::fixCType(structObj) << " " << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << altNameStr << "(";
              } else {
                if (!method->hasModifier(Modifier_Optional)) {
                  ss << getReturnMarshal(method->mResult, indentApiStr);
                }
                apiFile.usingTypedef(method->mResult);
                ss << indentApiStr << "public extern static " << GenerateStructC::fixCType(method->hasModifier(Modifier_Optional), method->mResult) << " " << GenerateStructC::fixType(rootStructObj) << "_" << altNameStr << "(";
              }
              bool first {true};
              if (method->mThrows.size() > 0) {
                apiFile.usingTypedef("exception_handle_t", "System.IntPtr");
                ss << "exception_handle_t wrapperExceptionHandle";
                first = false;
              }
              if ((!isConstructor) &&
                  (!isStatic)) {
                if (!first) ss << ", ";
                first = false;
                ss << GenerateStructC::fixCType(rootStructObj) << " thisHandle";
              }
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                apiFile.usingTypedef(arg->mType);
                if (!arg->hasModifier(Modifier_Optional)) {
                  ss << getParamMarshal(arg->mType);
                }
                ss << GenerateStructC::fixCType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->getMappingName());
              }
              ss << ");\n";
            }

            {
              auto &iSS = structFile.interfaceSS_;
              auto &sSS = structFile.structSS_;

              if (generateInterface) {
                iSS << "\n";
                iSS << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);
                iSS << indentStr << resultCsStr << " " << GenerateStructCx::fixName(method->getMappingName()) << "(" << (method->mArguments.size() > 1 ? String("\n" + indentStr + "    ") : String());
              }

              String specialPrefix = getSpecialMethodPrefix(apiFile, structFile, rootStructObj, structObj, method);

              sSS << "\n";
              sSS << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);
              sSS << indentStr << specialPrefix << (specialPrefix.hasData() ? " " : "") << (isStatic ? "static " : "") << "public " << (isConstructor ? String() : String(resultCsStr + " ")) << GenerateStructCx::fixName(method->getMappingName()) << "(" << (method->mArguments.size() > 1 ? String("\n" + indentStr + "    ") : String());

              bool first {true};
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) {
                  if (generateInterface) {
                    iSS << ",\n" << indentStr << "    ";
                  }
                  sSS << ",\n" << indentStr << "    ";
                }
                first = false;
                if (generateInterface) {
                  iSS << fixCsPathType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->getMappingName());
                }
                sSS << fixCsPathType(arg->hasModifier(Modifier_Optional), arg->mType) << " " << fixArgumentName(arg->getMappingName());
              }

              if (generateInterface) {
                iSS << ");\n";
              }
              sSS << ")\n";
            }
            {
              auto &ss = structFile.structSS_;
              ss << indentStr << "{\n";
              structFile.indentMore();

              if (rootStructObj != structObj) {
                ss << "#if " << getApiCastRequiredDefine(apiFile) << "\n";

                ss << indentStr << "var cast = " << fixCsPathType(structObj) << ".Cast(this);\n";
                ss << indentStr << "if (null == cast) throw new System.NullReferenceException(\"this \\\"" << fixCsPathType(structObj) << "\\\" casted from \\\"" << fixCsPathType(rootStructObj) << "\\\" becomes null.\");\n";
                ss << indentStr << (hasResult ? "return " : "" ) << "cast." << GenerateStructCx::fixName(method->getMappingName()) << "(";
                bool first {true};
                for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                  auto arg = (*iterArgs);
                  if (!arg) continue;
                  if (!first) ss << ", ";
                  first = false;
                  ss << fixArgumentName(arg->getMappingName());
                }
                ss << ");\n";
                ss << "#else // " << getApiCastRequiredDefine(apiFile) << "\n";
              }

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                structFile.usingTypedef(arg->mType);
                ss << indentStr << GenerateStructC::fixCType(arg->hasModifier(Modifier_Optional), arg->mType) << " wrapper_c_" << arg->getMappingName() << " = " << getToCMethod(apiFile, arg->hasModifier(Modifier_Optional), arg->mType) << "(" << fixArgumentName(arg->getMappingName()) << ");\n";
              }
              if (method->mThrows.size() > 0) {
                structFile.usingTypedef("exception_handle_t", "System.IntPtr");
                ss << indentStr << "exception_handle_t wrapperException = " << getApiPath(apiFile) << ".exception_wrapperCreate_exception();\n";
              }
              if (hasResult) {
                structFile.usingTypedef(method->mResult);
                ss << indentStr << GenerateStructC::fixCType(method->hasModifier(Modifier_Optional), method->mResult) << " wrapper_c_wrapper_result = ";
              }
              if (isConstructor) {
                structFile.usingTypedef(structObj);
                ss << indentStr << "this.native_ = ";
              }

              if ((!hasResult) &&
                  (!isConstructor)) {
                ss << indentStr;
              }

              if (isConstructor) {
                ss << getApiPath(apiFile) << "." << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << altNameStr << "(";
              } else {
                ss << getApiPath(apiFile) << "." << GenerateStructC::fixType(rootStructObj) << "_" << altNameStr << "(";
              }

              bool first {true};
              if (method->mThrows.size() > 0) {
                ss << "wrapperException";
                first = false;
              }
              if ((!isConstructor) &&
                  (!isStatic)) {
                if (!first) ss << ", ";
                first = false;
                ss << "this.native_";
              }
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << "wrapper_c_" << arg->getMappingName();
              }
              ss << ");\n";

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                auto wrapperDestroyStr = getDestroyCMethod(apiFile, arg->hasModifier(Modifier_Optional), arg->mType);
                if (wrapperDestroyStr.hasData()) {
                  ss << indentStr << wrapperDestroyStr << "(wrapper_c_" << arg->getMappingName() << ");\n";
                }
              }

              if (method->mThrows.size() > 0) {
                ss << indentStr << "var wrapperCsException = " << getHelperPath(apiFile) << ".exception_AdoptFromC(wrapperException);\n";
                ss << indentStr << "if (null != wrapperCsException) {\n";
                if (hasResult) {
                  auto destroyStr = getDestroyCMethod(apiFile, method->hasModifier(Modifier_Optional), method->mResult);
                  if (destroyStr.hasData()) {
                    ss << indentStr << "    " << destroyStr << "(wrapper_c_wrapper_result);\n";
                  }
                }
                ss << indentStr << "    throw wrapperCsException;\n";
                ss << indentStr << "}\n";
              }

              if (hasResult) {
                ss << indentStr << "return " << getAdoptFromCMethod(apiFile, method->hasModifier(Modifier_Optional), method->mResult) << "(wrapper_c_wrapper_result);\n";
              }
              if (isConstructor) {
                if (structFile.hasEvents_) {
                  ss << indentStr << "WrapperObserveEvents();\n";
                }
              }

              if (rootStructObj != structObj) {
                ss << "#endif // " << getApiCastRequiredDefine(apiFile) << "\n";
              }

              structFile.indentLess();
              ss << indentStr << "}\n";
            }
          }

          if (foundDllMethod) {
            if (rootStructObj != structObj) {
              auto &&ss = apiFile.structSS_;
              ss << "\n";
              ss << "#endif // !" << getApiCastRequiredDefine(apiFile) << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::getSpecialPropertyPrefix(
                                                              ApiFile &apiFile,
                                                              StructFile &structFile,
                                                              StructPtr rootStructObj,
                                                              StructPtr structObj,
                                                              PropertyPtr propertyObj
                                                              )
        {
          if (!propertyObj) return String();
          if (!structObj) return String();

          String specialName = (GenerateStructCx::fixName(propertyObj->getMappingName()));

          if ("Message" == specialName) {
            bool isException = structFile.shouldInheritException_ || (shouldDeriveFromException(apiFile, structObj));
            if (isException) {
              if (propertyObj->hasModifier(Modifier_Property_Setter)) return String("new");
              if (propertyObj->hasModifier(Modifier_Property_Getter)) return String("override");
              return String("new");
            }
          }
          return String();
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processProperties(
                                                     ApiFile &apiFile,
                                                     StructFile &structFile,
                                                     StructPtr rootStructObj,
                                                     StructPtr structObj
                                                     )
        {
          auto &indentStr = structFile.indent_;
          if (structFile.isStaticOnly_) {
            if (rootStructObj != structObj) return;
          }

          bool isDictionary = structObj->hasModifier(Modifier_Struct_Dictionary);

          bool foundCastRequiredDll {};

          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter) {
            auto propertyObj = (*iter);
            if (!propertyObj) continue;

            bool isStatic = propertyObj->hasModifier(Modifier_Static);
            bool hasGetter = propertyObj->hasModifier(Modifier_Property_Getter);
            bool hasSetter = propertyObj->hasModifier(Modifier_Property_Setter);

            bool generateInterface = (!isStatic) && (rootStructObj == structObj) && (structFile.shouldDefineInterface_);

            if (!isDictionary) {
              if ((!hasGetter) && (!hasSetter)) {
                hasGetter = hasSetter = true;
              }
            }

            bool hasGet = true;
            bool hasSet = true;

            if ((hasGetter) && (!hasSetter)) hasSet = false;
            if ((hasSetter) && (!hasGetter)) hasGet = false;

            if (isStatic) {
              if (rootStructObj != structObj) continue;
              if (hasGet) hasGetter = true;
              if (hasSet) hasSetter = true;
            } else {
              apiFile.usingTypedef(rootStructObj);
            }

            apiFile.usingTypedef(propertyObj->mType);
            structFile.usingTypedef(propertyObj->mType);

            if (generateInterface) {
              auto &ss = structFile.interfaceSS_;
              ss << GenerateHelper::getDocumentation(indentStr + "/// ", propertyObj, 80);
              ss << indentStr << fixCsPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << GenerateStructCx::fixName(propertyObj->getMappingName()) << " { " << (hasGet ? "get; " : "") << (hasSet ? "set; " : "") << "}\n";
            }

            {
              auto &ss = structFile.structSS_;
              auto specialPrefix = getSpecialPropertyPrefix(apiFile, structFile, rootStructObj, structObj, propertyObj);
              ss << GenerateHelper::getDocumentation(indentStr + "/// ", propertyObj, 80);
              ss << indentStr << specialPrefix << (specialPrefix.hasData() ? " " : "") << (isStatic ? "static " : "") << "public " << fixCsPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << GenerateStructCx::fixName(propertyObj->getMappingName()) << "\n";
              ss << indentStr << "{\n";

              if (rootStructObj != structObj) {
                ss << "#if " << getApiCastRequiredDefine(apiFile) << "\n";
                if (hasGet) {
                  ss << indentStr << "    get\n";
                  ss << indentStr << "    {\n";
                  ss << indentStr << "        var cast = " << fixCsPathType(structObj) << ".Cast(this);\n";
                  ss << indentStr << "        if (null == cast) throw new System.NullReferenceException(\"this \\\"" << fixCsPathType(structObj) << "\\\" casted from \\\"" << fixCsPathType(rootStructObj) << "\\\" becomes null.\");\n";
                  ss << indentStr << "        return cast." << GenerateStructCx::fixName(propertyObj->getMappingName()) << ";\n";
                  ss << indentStr << "    }\n";
                }
                if (hasSet) {
                  ss << indentStr << "    set\n";
                  ss << indentStr << "    {\n";
                  ss << indentStr << "        var cast = " << fixCsPathType(structObj) << ".Cast(this);\n";
                  ss << indentStr << "        if (null == cast) throw new System.NullReferenceException(\"this \\\"" << fixCsPathType(structObj) << "\\\" casted from \\\"" << fixCsPathType(rootStructObj) << "\\\" becomes null.\");\n";
                  ss << indentStr << "        cast." << GenerateStructCx::fixName(propertyObj->getMappingName()) << " = value;\n";
                  ss << indentStr << "    }\n";
                }
                ss << "#else // " << getApiCastRequiredDefine(apiFile) << "\n";
              }

              if (hasGet) {
                ss << indentStr << "    get\n";
                ss << indentStr << "    {\n";
                ss << indentStr << "        var result = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(structObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "this.native_") << ");\n";
                ss << indentStr << "        return " << getAdoptFromCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(result);\n";
                ss << indentStr << "    }\n";
              }
              if (hasSet) {
                ss << indentStr << "    set\n";
                ss << indentStr << "    {\n";
                ss << indentStr << "        var cValue = " << getToCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value);\n";
                ss << indentStr << "        " << getApiPath(apiFile) << "." << GenerateStructC::fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "this.native_, ") << "cValue);\n";
                auto destroyRoutine = getDestroyCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType);
                if (destroyRoutine.hasData()) {
                  ss << indentStr << "        " << getDestroyCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(cValue);\n";
                }
                ss << indentStr << "    }\n";
              }

              if (rootStructObj != structObj) {
                ss << "#endif // " << getApiCastRequiredDefine(apiFile) << "\n";
              }

              ss << indentStr << "}\n";
            }

            {
              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.structSS_;
              if (rootStructObj != structObj) {
                if (!foundCastRequiredDll) {
                  ss << "#if !" << getApiCastRequiredDefine(apiFile) << "\n";
                  foundCastRequiredDll = true;
                }
              }
              if (hasGet) {
                ss << "\n";
                ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                if (!propertyObj->hasModifier(Modifier_Optional)) {
                  ss << getReturnMarshal(propertyObj->mType, indentApiStr);
                }
                ss << indentApiStr << "public extern static " << GenerateStructC::fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " " << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String() : String(GenerateStructC::fixCType(rootStructObj) + " thisHandle")) << ");\n";
              }
              if (hasSet) {
                ss << "\n";
                ss << indentApiStr << "[DllImport(UseDynamicLib, CallingConvention = UseCallingConvention)]\n";
                ss << indentApiStr << "public extern static void " << GenerateStructC::fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String() : String(GenerateStructC::fixCType(rootStructObj) + " thisHandle, ")) << (propertyObj->hasModifier(Modifier_Optional) ? String() : getParamMarshal(propertyObj->mType)) << GenerateStructC::fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << " value);\n";
              }
            }

          }
          if (foundCastRequiredDll) {
            auto &ss = apiFile.structSS_;
            ss << "#endif // !" << getApiCastRequiredDefine(apiFile) << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlers(
                                                        ApiFile &apiFile,
                                                        StructFile &structFile,
                                                        StructPtr structObj
                                                        )
        {
          auto &indentStr = structFile.indent_;

          if (!structObj) return;
          if (!structFile.hasEvents_) return;

          structFile.usingTypedef("callback_event_t", "System.IntPtr");

          processEventHandlersStart(apiFile, structFile, structObj);

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            if (!method->hasModifier(Modifier_Method_EventHandler)) continue;

            {
              auto &ss = structFile.delegateSS_;
              structFile.indentLess();
              ss << indentStr << "public delegate void " << fixCsType(structObj) << "_" << GenerateStructCx::fixName(method->getMappingName()) << "(";
              bool first {true};
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << fixCsPathType(arg->mType) << " " << fixArgumentName(arg->getMappingName());
              }
              ss << ");\n";
              structFile.indentMore();
            }

            {
              auto &ss = structFile.structSS_;

              ss << GenerateHelper::getDocumentation(indentStr + "/// ", method, 80);
              ss << indentStr << "public event " << fixCsType(structObj) << "_" << GenerateStructCx::fixName(method->getMappingName()) << " " << GenerateStructCx::fixName(method->getMappingName()) << ";\n";
            }
          }

          processEventHandlersEnd(apiFile, structFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlersStart(
                                                             ApiFile &apiFile,
                                                             StructFile &structFile,
                                                             StructPtr structObj
                                                             )
        {
          auto &indentStr = structFile.indent_;

          structFile.startRegion("Events");

          structFile.usingTypedef("event_observer_t", "System.IntPtr");

          {
            auto &ss = structFile.structSS_;
            ss << "\n";
            ss << indentStr << "private event_observer_t wrapperEventObserver_ = System.IntPtr.Zero;\n";
            ss << "\n";
            ss << indentStr << "private void WrapperObserveEvents()\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero == this.native_) return;\n";
            ss << indentStr << "    " << getHelperPath(apiFile) << ".ObserveEvents(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath(apiFile) << "." << GenerateStructC::fixType(structObj) << "_wrapperInstanceId(this.native_), (object)this, (object target, string method, callback_event_t handle) => {\n";
            structFile.indentMore();
            structFile.indentMore();

            ss << indentStr << "if (null == target) return;\n";

            for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
              auto method = (*iter);
              if (!method) continue;
              if (!method->hasModifier(Modifier_Method_EventHandler)) continue;
              
              ss << indentStr << "if (\"" << method->getMappingName() << "\" == method) {\n";
              ss << indentStr << "    ((" << fixCsType(structObj) << ")target)." << GenerateStructCx::fixName(method->getMappingName()) << "?.Invoke(";
              bool first {true};
              size_t index = 0;
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << getAdoptFromCMethod(apiFile, method->hasModifier(Modifier_Optional), arg->mType) << "(" << getApiPath(apiFile) << ".callback_event_get_data(handle, " << index << "))";
              }
              ss << ");\n";
              ss << indentStr << "}\n";
            }

            structFile.indentLess();
            structFile.indentLess();

            ss << indentStr << "    });\n";
            ss << indentStr << "    if (System.IntPtr.Zero == this.wrapperEventObserver_) this.wrapperEventObserver_ = " << getApiPath(apiFile) << "." << GenerateStructC::fixType(structObj) << "_wrapperObserveEvents(this.native_);\n";
            ss << indentStr << "}\n";

            ss << "\n";
            ss << indentStr << "private void WrapperObserveEventsCancel()\n";
            ss << indentStr << "{\n";
            ss << indentStr << "    if (System.IntPtr.Zero != this.wrapperEventObserver_)\n";
            ss << indentStr << "    {\n";
            ss << indentStr << "        " << getApiPath(apiFile) << ".callback_wrapperObserverDestroy(this.wrapperEventObserver_);\n";
            ss << indentStr << "        this.wrapperEventObserver_ = System.IntPtr.Zero;\n";
            ss << indentStr << "    }\n";
            ss << indentStr << "    " << getHelperPath(apiFile) << ".ObserveEventsCancel(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath(apiFile) << "." << GenerateStructC::fixType(structObj) << "_wrapperInstanceId(this.native_), (object)this);\n";
            ss << indentStr << "}\n";
            ss << "\n";

          }
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::processEventHandlersEnd(
                                                           ApiFile &apiFile,
                                                           StructFile &structFile,
                                                           StructPtr structObj
                                                           )
        {
          structFile.endRegion("Events");
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructHeader::IIDLCompilerTarget
        #pragma mark

        //---------------------------------------------------------------------
        String GenerateStructDotNet::targetKeyword()
        {
          return String("dotnet");
        }

        //---------------------------------------------------------------------
        String GenerateStructDotNet::targetKeywordHelp()
        {
          return String("Generate C# DotNet wrapper using C API");
        }

        //---------------------------------------------------------------------
        void GenerateStructDotNet::targetOutput(
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

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("dotnet"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          EnumFile enumFile;
          enumFile.project_ = project;
          enumFile.global_ = project->mGlobal;

          ApiFile apiFile;
          apiFile.project_ = project;
          apiFile.global_ = project->mGlobal;

          GenerateStructC::calculateRelations(project->mGlobal, apiFile.derives_);
          GenerateStructC::calculateBoxings(project->mGlobal, apiFile.boxings_);

          enumFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("net_enums.cs"));
          apiFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("net_internal_apis.cs"));

          prepareEnumFile(enumFile);
          prepareApiFile(apiFile);

          processNamespace(apiFile, apiFile.global_);

          finalizeEnumFile(enumFile);
          finalizeApiFile(apiFile);
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
