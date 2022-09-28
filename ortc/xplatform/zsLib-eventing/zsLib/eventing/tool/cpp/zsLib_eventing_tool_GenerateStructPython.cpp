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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateStructPython.h>
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
        #pragma mark GenerateStructPython::BaseFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructPython::BaseFile::BaseFile()
        {
        }

        //---------------------------------------------------------------------
        GenerateStructPython::BaseFile::~BaseFile()
        {
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::usingTypedef(
                                                          const String &usingType,
                                                          const String &originalType,
                                                          const String &asType
                                                          )
        {
          if (alreadyUsing_.end() != alreadyUsing_.find(usingType)) return;
          alreadyUsing_.insert(usingType);

          auto &ss = (originalType.hasData() ? usingAliasSS_ : importSS_);
          if (!originalType.hasData()) {            
            ss << "import " << usingType << "\n";
            return;
          }
          if (originalType == "*") {
            ss << "from " << usingType << " import *\n";
            return;
          }
          if (asType.hasData()) {
            ss << "from " << usingType << " import " << originalType << " as " << asType << "\n";
            return;
          }
          ss << "from ctypes import " << originalType << " as " << usingType << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::usingTypedef(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void: return;

            case PredefinedTypedef_binary: {
              usingTypedef("binary_t", "c_void_p");
              usingTypedef("box_binary_t", "c_void_p");
              return;
            }
            case PredefinedTypedef_string:
            case PredefinedTypedef_astring:
            case PredefinedTypedef_wstring: {
              usingTypedef("string_t", "c_void_p");
              usingTypedef("box_string_t", "c_void_p");
              return;
            }
            default: break;
          }
          usingTypedef(GenerateStructC::fixCType(type), fixCsSystemType(type));
          usingTypedef("box_" + GenerateStructC::fixCType(type), "c_void_p");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::usingTypedef(TypePtr type)
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
              usingTypedef(GenerateStructC::fixCType(type), "c_void_p");
              return;
            }
          }

          {
            auto templatedType = type->toTemplatedStructType();
            if (templatedType) {
              usingTypedef(GenerateStructC::fixCType(type), "c_void_p");
              return;
            }
          }

          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto systemType = fixCsSystemType(enumType->mBaseType);
              if (systemType.hasData()) {
                usingTypedef(GenerateStructC::fixCType(type), systemType);
                usingTypedef(GenerateStructC::fixCType(true, type), "c_void_p");
              }
            }
          }
        }

        //---------------------------------------------------------------------
        bool GenerateStructPython::BaseFile::hasBoxing(const String &namePathStr)
        {
          auto found = boxings_.find(namePathStr);
          return found != boxings_.end();
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::startRegion(const String &region)
        {
          auto dash = GenerateHelper::getDashedComment(indent_);
          dash.replaceAll("//","# ");

          auto &ss = structSS_;
          ss << indent_ << "\n";
          ss << indent_ << "# region " << region << "\n";
          ss << indent_ << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "# " << region << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::endRegion(const String &region)
        {
          auto &ss = structSS_;
          ss << indent_ << "\n";
          ss << indent_ << "# endregion // " << region << "\n";
          ss << indent_ << "\n";
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::startOtherRegion(
                                                              const String &region,
                                                              bool preStruct
                                                              )
        {
          auto dash = GenerateHelper::getDashedComment(indent_);
          dash.replaceAll("//","# ");

          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << indent_ << "\n";
          ss << indent_ << "# region " << region << "\n";
          ss << indent_ << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "# " << region << "\n";
          ss << dash;
          ss << dash;
          ss << indent_ << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::BaseFile::endOtherRegion(
                                                            const String &region,
                                                            bool preStruct
                                                            )
        {
          auto &ss = preStruct ? preStructSS_ : postStructSS_;
          ss << indent_ << "\n";
          ss << indent_ << "#endregion // " << region << "\n";
          ss << indent_ << "\n";
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructPython::StructFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructPython::StructFile::StructFile(
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
        GenerateStructPython::StructFile::~StructFile()
        {
        }
        
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructPython::ApiFile
        #pragma mark

        //---------------------------------------------------------------------
        GenerateStructPython::ApiFile::ApiFile() :
          helpersSS_(postStructSS_),
          helpersEndSS_(postStructEndSS_)
        {
        }

        //---------------------------------------------------------------------
        GenerateStructPython::ApiFile::~ApiFile()
        {
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark GenerateStructPython
        #pragma mark


        //-------------------------------------------------------------------
        GenerateStructPython::GenerateStructPython() : IDLCompiler(Noop{})
        {
        }

        //-------------------------------------------------------------------
        GenerateStructPythonPtr GenerateStructPython::create()
        {
          return make_shared<GenerateStructPython>();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixName(const String &originalName)
        {
          if (originalName.isEmpty()) return String();
          String firstLetter = originalName.substr(0, 1);
          String remaining = originalName.substr(1);
          firstLetter.toUpper();
          String result = firstLetter + remaining;
          if (result == "None") return "_None";
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixNamespace(NamespacePtr namespaceObj)
        {          
          if (!namespaceObj) return String();

          if (namespaceObj->isGlobal()) return String();

          auto result = namespaceObj->getPathName();
          result.replaceAll("::",".");
          result = result.substr(1);
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixNamePath(ContextPtr context)
        {
          ContextList parents;
          while (context) {
            if (context->toProject()) break;
            auto namespaceObj = context->toNamespace();
            if (namespaceObj) {
              if (namespaceObj->mName.isEmpty()) break;
            }
            parents.push_front(context);
            context = context->getParent();
          }

          bool lastWasStruct = false;
          String path;
          for (auto iter = parents.begin(); iter != parents.end(); ++iter)
          {
            context = (*iter);

            auto structObj = context->toStruct();
            if ((structObj) && (lastWasStruct)) {
              path += "_";
            } else {
              path += "::";
            }

            path += structObj ? fixName(context->mName) : context->mName;
            lastWasStruct = ((bool)structObj);
          }
          return path;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::flattenPath(const String &originalName)
        {
          String result(originalName);
          result.replaceAll(".","_");
          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getPythonConvertedCType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "bool";
            case PredefinedTypedef_uchar:       return "int";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "str";
            case PredefinedTypedef_ushort:      
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      
            case PredefinedTypedef_uint:        
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "int";
            case PredefinedTypedef_ulong:       
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       
            case PredefinedTypedef_ulonglong:   
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "long";
            case PredefinedTypedef_uint8:       
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       
            case PredefinedTypedef_uint16:      
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      
            case PredefinedTypedef_uint32:      
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "int";
            case PredefinedTypedef_uint64:      
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "long";

            case PredefinedTypedef_byte:        
            case PredefinedTypedef_word:        
            case PredefinedTypedef_dword:       return "int";
            case PredefinedTypedef_qword:       return "long";

            case PredefinedTypedef_float:       
            case PredefinedTypedef_double:      
            case PredefinedTypedef_ldouble:     
            case PredefinedTypedef_float32:     
            case PredefinedTypedef_float64:     return "float";

            case PredefinedTypedef_pointer:     return "long";

            case PredefinedTypedef_binary:      return "bytes";
            case PredefinedTypedef_size:        return "long";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "str";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getPythonConvertedCType(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return getPythonConvertedCType(basicType->mBaseType);
          }
          return fixPythonType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getPythonCType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "c_bool";
            case PredefinedTypedef_uchar:       return "c_byte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "c_char";
            case PredefinedTypedef_ushort:      return "c_ushort";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "c_short";
            case PredefinedTypedef_uint:        return "c_uint";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "c_int";
            case PredefinedTypedef_ulong:       return "c_ulong";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "c_long";
            case PredefinedTypedef_ulonglong:   return "c_ulonglong";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "c_longlong";
            case PredefinedTypedef_uint8:       return "c_uint8";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "c_int8";
            case PredefinedTypedef_uint16:      return "c_uint16";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "c_int16";
            case PredefinedTypedef_uint32:      return "c_uint32";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "c_int32";
            case PredefinedTypedef_uint64:      return "c_uint64";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "c_int64";

            case PredefinedTypedef_byte:        return "c_uint8";
            case PredefinedTypedef_word:        return "c_uint16";
            case PredefinedTypedef_dword:       return "c_uint32";
            case PredefinedTypedef_qword:       return "c_uint64";

            case PredefinedTypedef_float:       return "c_float";
            case PredefinedTypedef_double:      return "c_double";
            case PredefinedTypedef_ldouble:     return "c_longdouble";
            case PredefinedTypedef_float32:     return "c_float";
            case PredefinedTypedef_float64:     return "c_double";

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
        String GenerateStructPython::getPythonToCType(
                                                      const String &input,
                                                      IEventingTypes::PredefinedTypedefs type
                                                      )
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "c_bool(" + input + ")";
            case PredefinedTypedef_uchar:       return "c_byte(" + input + ")";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "c_char(" + input + ")";
            case PredefinedTypedef_ushort:      return "c_ushort(" + input + ")";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "c_short(" + input + ")";
            case PredefinedTypedef_uint:        return "c_uint(" + input + ")";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "c_int(" + input + ")";
            case PredefinedTypedef_ulong:       return "c_ulong(" + input + ")";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "c_long(" + input + ")";
            case PredefinedTypedef_ulonglong:   return "c_ulonglong(" + input + ")";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "c_longlong(" + input + ")";
            case PredefinedTypedef_uint8:       return "c_uint8(" + input + ")";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "c_int8(" + input + ")";
            case PredefinedTypedef_uint16:      return "c_uint16(" + input + ")";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "c_int16(" + input + ")";
            case PredefinedTypedef_uint32:      return "c_uint32(" + input + ")";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "c_int32(" + input + ")";
            case PredefinedTypedef_uint64:      return "c_uint64(" + input + ")";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "c_int64(" + input + ")";

            case PredefinedTypedef_byte:        return "c_uint8(" + input + ")";
            case PredefinedTypedef_word:        return "c_uint16(" + input + ")";
            case PredefinedTypedef_dword:       return "c_uint32(" + input + ")";
            case PredefinedTypedef_qword:       return "c_uint64(" + input + ")";

            case PredefinedTypedef_float:       return "c_float(" + input + ")";
            case PredefinedTypedef_double:      return "c_double(" + input + ")";
            case PredefinedTypedef_ldouble:     return "c_ldouble(" + input + ")";
            case PredefinedTypedef_float32:     return "c_float(" + input + ")";
            case PredefinedTypedef_float64:     return "c_double(" + input + ")";

            case PredefinedTypedef_pointer:     return "c_void_p(" + input + ")";

            case PredefinedTypedef_binary:      return "c_void_p(" + input + ")";
            case PredefinedTypedef_size:        return "c_size_t(" + input + ")";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "c_char_p(" + input + ")";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getCToPythonType(
                                                      const String &input,
                                                      IEventingTypes::PredefinedTypedefs type
                                                      )
        {
          switch (type) {
            case PredefinedTypedef_void:        break;
            case PredefinedTypedef_bool:        return "(bool(" + input + "))";
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
            case PredefinedTypedef_sint64:      return "((" + input + ").value)";

            case PredefinedTypedef_byte:        
            case PredefinedTypedef_word:        
            case PredefinedTypedef_dword:       
            case PredefinedTypedef_qword:       return "((" + input + ").value)";

            case PredefinedTypedef_float:       
            case PredefinedTypedef_double:      
            case PredefinedTypedef_ldouble:     
            case PredefinedTypedef_float32:     
            case PredefinedTypedef_float64:     return "((" + input + ").value)";

            case PredefinedTypedef_pointer:     return "((" + input + ").value)";

            case PredefinedTypedef_binary:      return "((" + input + ").value)";
            case PredefinedTypedef_size:        return "((" + input + ").value)";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "((" + input + ").value)";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixArgumentName(const String &value)
        {
          //if ("params" == value) return "@params";
          //if ("event" == value) return "@event";
          return value;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixCPythonType(IEventingTypes::PredefinedTypedefs type)
        {
          return getPythonCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixCPythonType(TypePtr type)
        {
          {
            auto basicType = type->toBasicType();
            if (basicType) return fixCPythonType(basicType->mBaseType);
          }
          return GenerateStructC::fixCType(type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixPythonType(
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          if (!type) return String();

          {
            auto basicType = type->toBasicType();
            if (basicType) {
              switch (basicType->mBaseType) {
                case PredefinedTypedef_pointer:     return "c_void_p";

                case PredefinedTypedef_binary:      return "bytes";
                case PredefinedTypedef_size:        return "int";

                case PredefinedTypedef_string:
                case PredefinedTypedef_astring:
                case PredefinedTypedef_wstring:     return "str";
                default:                            break;
              }
              return getPythonConvertedCType(basicType->toBasicType());
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) return fixName(enumType->getMappingName());
          }
          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) {
                String specialName = structType->getPathName();
                if ("::zs::Time" == specialName) return "datetime.datetime";
                if ("::zs::Nanoseconds" == specialName) return "datetime.timedelta";
                if ("::zs::Microseconds" == specialName) return "datetime.timedelta";
                if ("::zs::Milliseconds" == specialName) return "datetime.timedelta";
                if ("::zs::Seconds" == specialName) return "datetime.timedelta";
                if ("::zs::Minutes" == specialName) return "datetime.timedelta";
                if ("::zs::Hours" == specialName) return "datetime.timedelta";
                if ("::zs::Days" == specialName) return "datetime.timedelta";

                if ("::zs::exceptions::Exception" == specialName) return "Exception";
                if ("::zs::exceptions::InvalidArgument" == specialName) return "ValueError";
                if ("::zs::exceptions::BadState" == specialName) return "RuntimeError";
                if ("::zs::exceptions::NotImplemented" == specialName) return "NotImplementedError";
                if ("::zs::exceptions::NotSupported" == specialName) return "StandardError";
                if ("::zs::exceptions::UnexpectedError" == specialName) return "SystemError";

                if ("::zs::Any" == specialName) return "object";
                if ("::zs::Promise" == specialName) return "wrapper.Promise";
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
                  prefixStr = "wrapper.Promise";
                  maxArgs = 1;
                }
                if ("::std::list" == specialName) prefixStr = "list";
                if ("::std::set" == specialName) {
                  prefixStr = "dict";
                  postFixStr = ", object>";
                }
                if ("::std::map" == specialName) prefixStr = "dict";

                size_t index = 0;
                String argsStr;
                for (auto iter = templatedType->mTemplateArguments.begin(); (index < maxArgs) && (iter != templatedType->mTemplateArguments.end()); ++iter, ++index) {
                  auto subType = (*iter);
                  if (argsStr.hasData()) {
                    argsStr += ", ";
                  }
                  argsStr += fixPythonPathType(subType, isInterface);
                }

                if ((">" == postFixStr.substr(0, 1)) &&
                    (argsStr.length() > 0) &&
                    (">" == argsStr.substr(argsStr.length() - 1, 1))) {
                  argsStr += " ";
                }

                return prefixStr;
              }
            }
          }

          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixPythonPathType(
                                                       TypePtr type,
                                                       bool isInterface
                                                       )
        {
          if (!type) return String();

          {
            auto structType = type->toStruct();
            if (structType) {
              if (GenerateHelper::isBuiltInType(structType)) return fixPythonType(type, isInterface);

              if (isInterface) {
                if (hasInterface(structType)) {
                  auto parent = structType->getParent();
                  auto result = fixNamePath(parent);
                  if (result.substr(0, 2) == "::") result = result.substr(2);
                  result.replaceAll("::", ".");
                  if (result.hasData()) {
                    result += ".";
                  }
                  result += "I" + GenerateStructCx::fixStructName(structType);
                  return result;
                }
              }

              auto result = fixNamePath(structType);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::",".");
              return result;
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              auto parent = enumType->getParent();
              auto result = fixNamePath(parent);
              if (result.substr(0, 2) == "::") result = result.substr(2);
              result.replaceAll("::", ".");
              if (result.hasData()) {
                result += ".";
              }
              result += fixName(enumType->getMappingName());
              return result;
            }
          }

          return fixPythonType(type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixPythonType(
                                                   bool isOptional,
                                                   TypePtr type,
                                                   bool isInterface
                                                   )
        {
          auto result = fixPythonType(type, isInterface);
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
              return result;
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              return result;
            }
          }

          return result;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixPythonPathType(
                                                       bool isOptional,
                                                       TypePtr type,
                                                       bool isInterface
                                                       )
        {
          if (!isOptional) return fixPythonPathType(type, isInterface);
          if (type->toStruct()) return fixPythonPathType(type, isInterface);
          return fixPythonType(isOptional, type, isInterface);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::fixCsSystemType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type) {
            case PredefinedTypedef_void:        return String();
            case PredefinedTypedef_bool:        return "c_bool";
            case PredefinedTypedef_uchar:       return "c_ubyte";
            case PredefinedTypedef_char:        
            case PredefinedTypedef_schar:       return "c_char";
            case PredefinedTypedef_ushort:      return "c_ushort";
            case PredefinedTypedef_short:       
            case PredefinedTypedef_sshort:      return "c_short";
            case PredefinedTypedef_uint:        return "c_uint";
            case PredefinedTypedef_int:         
            case PredefinedTypedef_sint:        return "c_int";
            case PredefinedTypedef_ulong:       return "c_ulong";
            case PredefinedTypedef_long:        
            case PredefinedTypedef_slong:       return "c_long";
            case PredefinedTypedef_ulonglong:   return "c_ulonglong";
            case PredefinedTypedef_longlong:    
            case PredefinedTypedef_slonglong:   return "c_longlong";
            case PredefinedTypedef_uint8:       return "c_ubyte";
            case PredefinedTypedef_int8:        
            case PredefinedTypedef_sint8:       return "c_char";
            case PredefinedTypedef_uint16:      return "c_ushort";
            case PredefinedTypedef_int16:       
            case PredefinedTypedef_sint16:      return "c_short";
            case PredefinedTypedef_uint32:      return "c_uint";
            case PredefinedTypedef_int32:       
            case PredefinedTypedef_sint32:      return "c_int";
            case PredefinedTypedef_uint64:      return "c_ulonglong";
            case PredefinedTypedef_int64:       
            case PredefinedTypedef_sint64:      return "c_longlong";

            case PredefinedTypedef_byte:        return "c_ubyte";
            case PredefinedTypedef_word:        return "c_ushort";
            case PredefinedTypedef_dword:       return "c_uint";
            case PredefinedTypedef_qword:       return "c_ulonglong";

            case PredefinedTypedef_float:       return "c_float";
            case PredefinedTypedef_double:      return "c_double";
            case PredefinedTypedef_ldouble:     return "c_longdouble";
            case PredefinedTypedef_float32:     return "c_float";
            case PredefinedTypedef_float64:     return "c_double";

            case PredefinedTypedef_pointer:     return "c_void_p";

            case PredefinedTypedef_binary:      return "c_void_p";
            case PredefinedTypedef_size:        return "c_ulonglong";

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return "c_void_p";
          }
          return String();
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::combineIf(
                                               const String &combinePreStr,
                                               const String &combinePostStr,
                                               const String &ifHasValue
                                               )
        {
          if (ifHasValue.isEmpty()) return String();
          return combinePreStr + ifHasValue + combinePostStr;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getHelpersMethod(
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
                return (useApiHelper ? getApiPath() : getHelperPath()) + ".box_" + cTypeStr + methodName;
              }
              if ("string_t" == cTypeStr) {
                return (useApiHelper ? getApiPath() : getHelperPath()) + "." + GenerateStructC::fixType(type) + methodName;
              }
              if ("binary_t" == cTypeStr) {
                return (useApiHelper ? getApiPath() : getHelperPath()) + "." + GenerateStructC::fixType(type) + methodName;
              }
              return (useApiHelper ? getApiPath() : getHelperPath()) + "." + cTypeStr + methodName;
            }
          }
          {
            auto enumType = type->toEnumType();
            if (enumType) {
              String cTypeStr = GenerateStructC::fixCType(enumType);
              if (isOptional) {
                return (useApiHelper ? getApiPath() : getHelperPath()) + ".box_" + GenerateStructC::fixCType(type) + methodName;
              }
              return (useApiHelper ? getApiPath() : getHelperPath()) + "." + GenerateStructC::fixCType(type) + methodName;
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
                      return getApiPath() + ".zs_Promise" + methodName;
                    }
                  }
                }
              }
            }
          }

          return (useApiHelper ? getApiPath() : getHelperPath()) + "." + GenerateStructC::fixType(type) + methodName;
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getToCMethod(
                                                  BaseFile &baseFile,
                                                  bool isOptional,
                                                  TypePtr type
                                                  )
        {
          return getHelpersMethod(baseFile, false, "_ToC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getFromCMethod(
                                                    BaseFile &baseFile,
                                                    bool isOptional, 
                                                    TypePtr type
                                                    )
        {
          return getHelpersMethod(baseFile, false, "_FromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getAdoptFromCMethod(
                                                         BaseFile &baseFile,
                                                         bool isOptional, 
                                                         TypePtr type
                                                         )
        {
          return getHelpersMethod(baseFile, false, "_AdoptFromC", isOptional, type);
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getDestroyCMethod(
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
        bool GenerateStructPython::hasInterface(StructPtr structObj)
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
        String GenerateStructPython::getApiCastRequiredDefine(BaseFile &baseFile)
        {
          String result = "WRAPPER_C_GENERATED_REQUIRES_CAST";
          if (!baseFile.project_) return result;
          if (baseFile.project_->mName.isEmpty()) return result;

          auto name = baseFile.project_->mName;
          name.toUpper();
          return name + "_WRAPPER_C_GENERATED_REQUIRES_CAST";
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getApiPath()
        {
          return "wrapper.Api";
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::getHelperPath()
        {
          return "wrapper.Helpers";
        }

        //---------------------------------------------------------------------
        bool GenerateStructPython::shouldDeriveFromException(
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
        void GenerateStructPython::finalizeBaseFile(BaseFile &baseFile)
        {
          std::stringstream ss;

          GenerateStructC::appendStream(ss, baseFile.importSS_);
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
        void GenerateStructPython::prepareSetupFile(SetupFile &setupFile)
        {

          {
            auto &ss = setupFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
          }

          {
            auto &ss = setupFile.importSS_;

            ss << "import wrapper\n\n";

            for (auto iter = setupFile.global_->mNamespaces.begin(); iter != setupFile.global_->mNamespaces.end(); ++iter) {
              auto &namespaceObj = (*iter).second;
              if (!namespaceObj) continue;

              auto namePath = namespaceObj->getPathName();
              if ("::zs" == namePath) continue;
              if ("::std" == namePath) continue;

              ss << "import " << namespaceObj->getMappingName() << "\n";
            }

            ss << "\n";
          }       

          {
            auto &ss = setupFile.usingAliasSS_;
            ss << "Promise = wrapper.Promise\n\n";
          }
   
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::finalizeSetupFile(SetupFile &setupFile)
        {
          finalizeBaseFile(setupFile);
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiInitFile(InitFile &apiInitFile)
        {
          {
            auto &ss = apiInitFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "from .wrapper import *\n\n";
            ss << "from promise import Promise\n\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::finalizeApiInitFile(InitFile &apiInitFile)
        {
          finalizeBaseFile(apiInitFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiFile(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          {
            auto &ss = apiFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
            ss << "try:\n";
            ss << "  check_integer_64 = long\n";
            ss << "except NameError:\n";
            ss << "  long = int\n";
            ss << "\n";
            ss << "try:\n";
            ss << "  check_none_type = NoneType\n";
            ss << "except NameError:\n";
            ss << "  NoneType = type(None)\n";
            ss << "\n";

            apiFile.usingTypedef("cdll", "cdll");
            apiFile.usingTypedef("c_char_p", "c_char_p");
            apiFile.usingTypedef("c_void_p", "c_void_p");
            apiFile.usingTypedef("c_bool", "c_bool");
            apiFile.usingTypedef("c_byte", "c_byte");
            apiFile.usingTypedef("c_char", "c_char");
            apiFile.usingTypedef("c_short", "c_short");
            apiFile.usingTypedef("c_ushort", "c_ushort");
            apiFile.usingTypedef("c_int", "c_int");
            apiFile.usingTypedef("c_uint", "c_uint");
            apiFile.usingTypedef("c_long", "c_long");
            apiFile.usingTypedef("c_ulong", "c_ulong");
            apiFile.usingTypedef("c_longlong", "c_longlong");
            apiFile.usingTypedef("c_ulonglong", "c_ulonglong");
            apiFile.usingTypedef("c_int8", "c_int8");
            apiFile.usingTypedef("c_uint8", "c_uint8");
            apiFile.usingTypedef("c_int16", "c_int16");
            apiFile.usingTypedef("c_uint16", "c_uint16");
            apiFile.usingTypedef("c_int32", "c_int32");
            apiFile.usingTypedef("c_uint32", "c_uint32");
            apiFile.usingTypedef("c_int64", "c_int64");
            apiFile.usingTypedef("c_uint64", "c_uint64");
            apiFile.usingTypedef("c_float", "c_float");
            apiFile.usingTypedef("c_double", "c_double");
            apiFile.usingTypedef("c_longdouble", "c_longdouble");
            apiFile.usingTypedef("c_size_t", "c_size_t");
          }

          {
            //auto &ss = apiFile.namespaceSS_;
          }

          {
            auto &ss = apiFile.helpersSS_;

            ss << indentStr << "class Helpers:\n";
          }

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "class IDisposable(object):\n";
            ss << indentStr << "    def dispose(self):\n";
            ss << indentStr << "        pass\n";
            ss << indentStr << "\n";
          }

          {
            auto &ss = apiFile.structSS_;

            String libNameStr = "lib" + fixName(apiFile.project_->getMappingName()) + "lib";
            libNameStr.toLower();
            ss << indentStr << "class Api:\n";
            apiFile.indentMore();
            ss << indentStr << "__libName = \"\"\n";

            ss << indentStr << "if sys.platform.startswith('tvos'):\n";
            ss << indentStr << "    __libName = \"@rpath/" << libNameStr << ".framework/" << libNameStr << "\"\n";
            ss << indentStr << "elif sys.platform.startswith('ios'):\n";
            ss << indentStr << "    __libName = \"" << libNameStr << ".dylib\"\n";
            ss << indentStr << "elif sys.platform.startswith('linux'):\n";
            ss << indentStr << "    __libName = \"./" << libNameStr << ".so\"\n";
            ss << indentStr << "elif sys.platform.startswith('darwin'):\n";
            ss << indentStr << "    __libName = \"" << libNameStr << ".dylib\"\n";
            ss << indentStr << "elif sys.platform.startswith('win'):\n";
            ss << indentStr << "    __libName = \"" << libNameStr << ".dll\"\n";
            ss << indentStr << "\n";
            ss << indentStr << "__lib = cdll.LoadLibrary(__libName)";
            ss << indentStr << "\n";
          }

          apiFile.usingTypedef("binary_t", "c_void_p");
          apiFile.usingTypedef("string_t", "c_void_p");
          apiFile.usingTypedef("raw_pointer_t", "c_void_p");
          apiFile.usingTypedef("binary_size_t", "c_ulonglong");

          prepareApiCallback(apiFile);
          prepareApiExceptions(apiFile);
          prepareApiBasicTypes(apiFile);
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
        void GenerateStructPython::finalizeApiFile(ApiFile &apiFile)
        {
          //auto &indentStr = apiFile.indent_;

          apiFile.endRegion("Struct API helpers");
          apiFile.endHelpersRegion("Struct helpers");
          apiFile.indentLess();

          {
            auto &ss = apiFile.helpersEndSS_;
            ss << "\n";
          }

          {
            auto &ss = apiFile.structEndSS_;
            ss << "\n";
          }

          {
            auto &ss = apiFile.namespaceEndSS_;

            ss << "\n";
          }

          finalizeBaseFile(apiFile);
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiCallback(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("generic_handle_t", "c_void_p");
          apiFile.usingTypedef("instance_id_t", "c_void_p");
          apiFile.usingTypedef("callback_event_t", "c_void_p");
          apiFile.usingTypedef("event_observer_t", "c_void_p");
          apiFile.usingTypedef("const_char_star_t", "c_char_p");
          apiFile.usingTypedef("CFUNCTYPE", "CFUNCTYPE");

          {
            auto &ss = apiFile.structSS_;

            apiFile.startRegion("Callback and Event API helpers");

            static const char *callbackHelpers =
              "# def wrapperCallbackFunction(handle: callback_event_t)\n"
              "\n"
              "__callback_wrapperInstall = __lib.callback_wrapperInstall\n"
              "__callback_wrapperInstall.restype = None\n"
              "\n"
              "@staticmethod\n"
              "def callback_wrapperInstall(function):\n"
              "    callbackDef = CFUNCTYPE(None, callback_event_t)\n"
              "    cCallback = callbackDef(function)\n"
              "    Api.__callback_wrapperInstall(cCallback)\n"
              "\n"
              "__callback_wrapperObserverDestroy = __lib.callback_wrapperObserverDestroy\n"
              "__callback_wrapperObserverDestroy.restype = None\n"
              "__callback_wrapperObserverDestroy.argtypes = [event_observer_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_wrapperObserverDestroy(handle: event_observer_t):\n"
              "    Api.__callback_wrapperObserverDestroy(handle)\n"
              "\n"
              "__callback_event_wrapperDestroy = __lib.callback_event_wrapperDestroy\n"
              "__callback_event_wrapperDestroy.restype = None\n"
              "__callback_event_wrapperDestroy.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_wrapperDestroy(handle: callback_event_t):\n"
              "    Api.__callback_event_wrapperDestroy(handle)\n"
              "\n"
              "__callback_event_wrapperInstanceId = __lib.callback_event_wrapperInstanceId\n"
              "__callback_event_wrapperInstanceId.restype = instance_id_t\n"
              "__callback_event_wrapperInstanceId.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_wrapperInstanceId(handle: callback_event_t) -> instance_id_t:\n"
              "    return Api.__callback_event_wrapperInstanceId(handle)\n"
              "\n"
              "__callback_event_get_observer = __lib.callback_event_get_observer\n"
              "__callback_event_get_observer.restype = event_observer_t\n"
              "__callback_event_get_observer.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_observer(handle: callback_event_t) -> event_observer_t:\n"
              "    return Api.__callback_event_get_observer(handle)\n"
              "\n"
              "__callback_event_get_namespace_actual = __lib.callback_event_get_namespace_actual\n"
              "__callback_event_get_namespace_actual.restype = const_char_star_t\n"
              "__callback_event_get_namespace_actual.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_namespace_actual(handle: callback_event_t) -> const_char_star_t:\n"
              "    return Api.__callback_event_get_namespace_actual(handle)\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_namespace(handle: callback_event_t) -> str:\n"
              "    result = Api.callback_event_get_namespace_actual(handle)\n"
              "    if (result is None):\n"
              "        return None\n"
              "    return result.decode('utf-8')\n"
              "\n"
              "__callback_event_get_class_actual = __lib.callback_event_get_class_actual\n"
              "__callback_event_get_class_actual.restype = const_char_star_t\n"
              "__callback_event_get_class_actual.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_class_actual(handle: callback_event_t) -> const_char_star_t:\n"
              "    return Api.__callback_event_get_class_actual(handle)\n"
              "\n"
              "def callback_event_get_class(handle: callback_event_t) -> str:\n"
              "    result = Api.callback_event_get_class_actual(handle)\n"
              "    if (result is None):\n"
              "        return None\n"
              "    return result.decode('utf-8')\n"
              "\n"
              "__callback_event_get_method_actual = __lib.callback_event_get_method_actual\n"
              "__callback_event_get_method_actual.restype = const_char_star_t\n"
              "__callback_event_get_method_actual.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_method_actual(handle: callback_event_t) -> const_char_star_t:\n"
              "    return Api.__callback_event_get_method_actual(handle)\n"
              "\n"
              "def callback_event_get_method(handle: callback_event_t) -> str:\n"
              "    result =  Api.callback_event_get_method_actual(handle)\n"
              "    if (result is None):\n"
              "        return None\n"
              "    return result.decode('utf-8')\n"
              "\n"
              "__callback_event_get_source = __lib.callback_event_get_source\n"
              "__callback_event_get_source.restype = generic_handle_t\n"
              "__callback_event_get_source.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_source(handle: callback_event_t) -> generic_handle_t:\n"
              "    return Api.__callback_event_get_source(handle)\n"
              "\n"
              "__callback_event_get_source_instance_id = __lib.callback_event_get_source_instance_id\n"
              "__callback_event_get_source_instance_id.restype = generic_handle_t\n"
              "__callback_event_get_source_instance_id.argtypes = [callback_event_t]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_source_instance_id(handle: callback_event_t) -> instance_id_t:\n"
              "    return Api.__callback_event_get_source_instance_id(handle)\n"
              "\n"
              "__callback_event_get_data = __lib.callback_event_get_data\n"
              "__callback_event_get_data.restype = generic_handle_t\n"
              "__callback_event_get_data.argtypes = [callback_event_t, c_int]\n"
              "\n"
              "@staticmethod\n"
              "def callback_event_get_data(handle: callback_event_t, argumentIndex: int) -> generic_handle_t:\n"
              "    return Api.__callback_event_get_data(handle, c_int(argumentIndex))\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, callbackHelpers, true);

            apiFile.endRegion("Callback and Event API helpers");
          }

          {
            auto &ss = apiFile.helpersSS_;

            apiFile.startHelpersRegion("Callback and Event helpers");

            apiFile.usingTypedef("weakref");
            apiFile.usingTypedef("sys");
            apiFile.usingTypedef("queue");

            static const char *callbackHelpers =
              "# delegate FireEventDelegate(target: object, methodName: str, handle: callback_event_t) -> None\n"
              "\n"
              "class EventManager(object):\n"
              "    __singleton = None\n"
              "    \n"
              "    @staticmethod\n"
              "    def getSingleton():\n"
              "        if (Helpers.EventManager.__singleton is None):\n"
              "            Helpers.EventManager.__singleton = Helpers.EventManager()\n"
              "        return Helpers.EventManager.__singleton\n"
              "    \n"
              "    @staticmethod\n"
              "    def getQueue():\n"
              "        return Helpers.EventManager.getSingleton().__queue\n"
              "    \n"
              "    class Target:\n"
              "        def __init__(self, target: object, targetCallback):\n"
              "          self.__target = weakref.ref(target)\n"              
              "          self.__delegate = targetCallback\n"
              "        \n"
              "        def fireEvent(self, methodName: str, handle: callback_event_t):\n"
              "            target = self.__target()\n"
              "            if (target is None):\n"
              "                return\n"
              "            self.__delegate(target, methodName, handle)\n"
              "        \n"
              "        def target(self):\n"
              "            return self.__target()\n"
              "    \n"
              "    class InstanceObservers:\n"
              "        def __init__(self):\n"
              "            self.__targets = []\n"
              "        \n"
              "        def observeEvents(self, target: object, targetCallback):\n"
              "            oldList = self.__targets\n"
              "            newList = []\n"
              "            \n"
              "            for value in oldList:\n"
              "                existingTarget = value.target()\n"
              "                if (existingTarget is None):\n"
              "                    continue\n"
              "                if (id(existingTarget) == id(target)):\n"
              "                    continue\n"
              "                newList.append(value)\n"
              "            \n"
              "            newList.append(Helpers.EventManager.Target(target, targetCallback))\n"
              "            self.__targets = newList\n"
              "        \n"
              "        def observeEventsCancel(self, target: object):\n"
              "            oldList = self.__targets\n"
              "            newList = []\n"
              "            \n"
              "            for value in oldList:\n"
              "                existingTarget = value.target()\n"
              "                if (existingTarget is None):\n"
              "                    continue\n"
              "                if (id(existingTarget) == id(target)):\n"
              "                    continue\n"
              "                newList.append(value)\n"
              "            \n"
              "            self.__targets = newList\n"
              "        \n"
              "        @property\n"
              "        def targets(self):\n"
              "            return self.__targets\n"
              "    \n"
              "    class StructObservers:\n"
              "        def __init__(self):\n"
              "            self.__observers = {}\n"
              "        \n"
              "        def observeEvents(self, source: instance_id_t, target: object, targetCallback):\n"
              "            observer = self.__observers.get(source)\n"
              "            if (observer is None):\n"
              "                observer = Helpers.EventManager.InstanceObservers()\n"
              "                self.__observers[source] = observer\n"
              "            observer.observeEvents(target, targetCallback)\n"
              "        \n"
              "        def observeEventsCancel(self, source: instance_id_t, target: object):\n"
              "            observer = self.__observers.get(source)\n"
              "            if (observer is not None):\n"
              "                    observer.observeEventsCancel(target)\n"
              "        \n"
              "        def getTargets(self, source: instance_id_t) -> list:\n"
              "            observer = self.__observers.get(source)\n"
              "            if (observer is None):\n"
              "                return None\n"
              "            return observer.targets\n"
              "    \n"
              "    class Structs:\n"
              "        def __init__(self):\n"
              "            self.__observers = {}\n"
              "        \n"
              "        def observeEvents(self, className: str, source: instance_id_t, target: object, targetCallback):\n"
              "            observer = self.__observers.get(className)\n"
              "            if (observer is None):\n"
              "                observer = Helpers.EventManager.StructObservers()\n"
              "                self.__observers[className] = observer\n"
              "            observer.observeEvents(source, target, targetCallback)\n"
              "        \n"
              "        def observeEventsCancel(self, className: str, source: instance_id_t, target: object):\n"
              "            observer = self.__observers.get(className)\n"
              "            if (observer is None):\n"
              "                return\n"
              "            observer.observeEventsCancel(source, target)\n"
              "        \n"
              "        def getTargets(self, className: str, source: instance_id_t) -> list:\n"
              "            observer = self.__observers.get(className)\n"
              "            if (observer is None):\n"
              "                return None\n"
              "            return observer.getTargets(source)\n"
              "    \n"
              "    def __init__(self):\n"
              "        self.__observers = {}\n"
              "        self.__queue = queue.Queue(maxsize=0)\n"
              "        self.__notifyCallback = None\n"
              "        $APINAMESPACE$.callback_wrapperInstall(Helpers.EventManager.handleEvent)\n"
              "    \n"
              "    def observeEvents(self, namespaceName: str, className: str, source: object, target: object, targetCallback):\n"
              "        observer = self.__observers.get(namespaceName)\n"
              "        if (observer is None):\n"
              "            observer = Helpers.EventManager.Structs()\n"
              "            self.__observers[namespaceName] = observer\n"
              "        observer.observeEvents(className, source, target, targetCallback)\n"
              "    \n"
              "    def observeEventsCancel(self, namespaceName: str, className: str, source: instance_id_t, target: object):\n"
              "        observer = self.__observers.get(namespaceName)\n"
              "        if (observer is None):\n"
              "            return\n"
              "        observer.observeEventsCancel(className, source, target)\n"
              "    \n"
              "    def __handleEvent(self, handle: callback_event_t, namespaceName: str, className: str, methodName: str, source: instance_id_t):\n"
              "        observer = self.__observers.get(namespaceName)\n"
              "        if (observer is None):\n"
              "            return\n"
              "        targets = observer.getTargets(className, source)\n"
              "        if (targets is None):\n"
              "            return\n"
              "        for target in targets:\n"
              "            target.fireEvent(methodName, handle)\n"
              "    \n"
              "    @staticmethod\n"
              "    def setNotifyCallback(notifyCallback):\n"
              "        singleton = Helpers.EventManager.getSingleton()\n"
              "        singleton.__notifyCallback = notifyCallback\n"
              "    \n"
              "    @staticmethod\n"
              "    def handleEvent(handle: callback_event_t):\n"
              "        if (handle is None):\n"
              "            return\n"
              "        singleton = Helpers.EventManager.getSingleton()\n"
              "        singleton.__queue.put(handle)\n"
              "        if (singleton.__notifyCallback is not None):\n"
              "            singleton.__notifyCallback()\n"
              "    \n"
              "    def processEvent(block=True):\n"
              "        queue = Helpers.EventManager.getQueue()\n"
              "        #try:\n"
              "        handle = queue.get(block)\n"
              "        #except Exception:\n"
              "        #    return\n"
              "        namespaceName = $APINAMESPACE$.callback_event_get_namespace(handle)\n"
              "        className = $APINAMESPACE$.callback_event_get_class(handle)\n"
              "        methodName = $APINAMESPACE$.callback_event_get_method(handle)\n"
              "        instanceId = $APINAMESPACE$.callback_event_get_source_instance_id(handle)\n"
              "        Helpers.EventManager.getSingleton().__handleEvent(handle, namespaceName, className, methodName, instanceId)\n"
              "        $APINAMESPACE$.callback_event_wrapperDestroy(handle)\n"
              "\n"
              "@staticmethod\n"
              "def observeEvents(namespaceName: str, className: str, source: instance_id_t, target: object, targetCallback):\n"
              "    Helpers.EventManager.getSingleton().observeEvents(namespaceName, className, source, target, targetCallback)\n"
              "\n"
              "@staticmethod\n"
              "def observeEventsCancel(namespaceName: str, className: str, source: instance_id_t, target: object):\n"
              "    Helpers.EventManager.getSingleton().observeEventsCancel(namespaceName, className, source, target)\n"
              "\n"
              "@staticmethod\n"
              "def processEvent(block=True):\n"
              "    Helpers.EventManager.processEvent(block)\n"
              "\n"
              "@staticmethod\n"
              "def setNotifyCallback(notifyCallback):\n"
              "    Helpers.EventManager.setNotifyCallback(notifyCallback)\n"
              "\n"
              "@staticmethod\n"
              "def getQueue():\n"
              "    Helpers.EventManager.getQueue()\n"
              "\n"
              ;

              String apiNamespaceStr = "Api";

              String callbackHelpersStr(callbackHelpers);
              callbackHelpersStr.replaceAll("$APINAMESPACE$", apiNamespaceStr);

              GenerateHelper::insertBlob(ss, indentStr, callbackHelpersStr, true);

              apiFile.endHelpersRegion("Callback and Event helpers");
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiExceptions(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("exception_handle_t", "c_void_p");
          apiFile.usingTypedef("instance_id_t", "c_void_p");
          apiFile.usingTypedef("const_char_star_t", "c_void_p");

          {
            apiFile.startRegion("Exception API helpers");

            auto &ss = apiFile.structSS_;

            static const char *exceptionHelpers =
              "\n"
              "__exception_wrapperCreate_exception = __lib.exception_wrapperCreate_exception\n"
              "__exception_wrapperCreate_exception.restype = exception_handle_t\n"
              "__exception_wrapperCreate_exception.argtypes = []\n"
              "\n"
              "@staticmethod\n"
              "def exception_wrapperCreate_exception() -> exception_handle_t:\n"
              "    return Api.__exception_wrapperCreate_exception()\n"
              "\n"
              "__exception_wrapperDestroy = __lib.exception_wrapperDestroy\n"
              "__exception_wrapperDestroy.restype = None\n"
              "__exception_wrapperDestroy.argtypes = [exception_handle_t]\n"
              "\n"
              "@staticmethod\n"
              "def exception_wrapperDestroy(handle: exception_handle_t):\n"
              "    Api.__exception_wrapperDestroy(handle)\n"
              "\n"
              "__exception_wrapperInstanceId = __lib.exception_wrapperInstanceId\n"
              "__exception_wrapperInstanceId.restype = instance_id_t\n"
              "__exception_wrapperInstanceId.argtypes = [exception_handle_t]\n"
              "\n"
              "@staticmethod\n"
              "def exception_wrapperInstanceId(handle: exception_handle_t) -> instance_id_t:\n"
              "    return Api.__exception_wrapperInstanceId(handle)\n"
              "\n"
              "__exception_hasException = __lib.exception_hasException\n"
              "__exception_hasException.restype = c_bool\n"
              "__exception_hasException.argtypes = [exception_handle_t]\n"
              "\n"
              "@staticmethod\n"
              "def exception_hasException(handle: exception_handle_t) -> bool:\n"
              "    return bool(Api.__exception_hasException(handle))\n"
              "\n"
              "__exception_what_actual = __lib.exception_what_actual\n"
              "__exception_what_actual.restype = c_char_p\n"
              "__exception_what_actual.argtypes = [exception_handle_t]\n"
              "\n"
              "@staticmethod\n"
              "def exception_what_actual(handle: exception_handle_t) -> c_char_p:\n"
              "    return Api.__exception_what_actual(handle)\n"
              "\n"
              "@staticmethod\n"
              "def exception_what(handle: exception_handle_t) -> str:\n"
              "    return exception_what_actual(handle).value\n"
              "\n"
              ;
            GenerateHelper::insertBlob(ss, indentStr, exceptionHelpers, true);
          }

          {
            apiFile.startHelpersRegion("Exception helpers");

            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def exception_FromC(handle: exception_handle_t) -> BaseException:\n";
            ss << indentStr << "    if (not Api.exception_hasException(handle)):\n";
            ss << indentStr << "      return None\n";
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
            ss << indentStr << "    return Exception(" << getApiPath() << ".exception_what(handle))\n";
            ss << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def exception_AdoptFromC(handle: exception_handle_t) -> BaseException:\n";
            ss << indentStr << "    result = Helpers.exception_FromC(handle)\n";
            ss << indentStr << "    Api.exception_wrapperDestroy(handle)\n";
            ss << indentStr << "    return result\n";

            apiFile.endHelpersRegion("Exception helpers");
          }

          {
            apiFile.endRegion("Exception API helpers");
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiExceptions(
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
            ss << indentStr << "\n";
            ss << indentStr << "__exception_is_" << exceptionName << " = __lib.exception_is_" << exceptionName << "\n";
            ss << indentStr << "__exception_is_" << exceptionName << ".restype = c_bool\n";
            ss << indentStr << "__exception_is_" << exceptionName << ".argtypes = [exception_handle_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def exception_is_" << exceptionName << "(handle: exception_handle_t) -> bool:\n";
            ss << indentStr << "    return bool(Api.__exception_is_" << exceptionName << "(handle))\n";
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "    if (Api.exception_is_" << exceptionName << "(handle)):\n";
            ss << indentStr << "        return " << fixPythonType(contextStruct) << "(" << getApiPath() << ".exception_what(handle)";
            ss << ")\n";
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiBasicTypes(ApiFile &apiFile)
        {
          apiFile.startRegion("Basic Types API helpers");
          apiFile.startHelpersRegion("Basic Types helpers");

          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_bool);

          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uchar);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_schar);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_ushort);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sshort);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uint);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sint);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_ulong);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_slong);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_ulonglong);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_slonglong);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uint8);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sint8);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uint16);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sint16);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uint32);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sint32);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_uint64);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_sint64);

          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_byte);
          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_word);
          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_dword);
          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_qword);

          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_float);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_double);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_ldouble);
          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_float32);
          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_float64);

          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_pointer);

          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_binary);
          prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_size);

          //prepareApiBasicTypes(apiFile, IEventingTypes::PredefinedTypedef_string);

          apiFile.endRegion("Basic Types API helpers");
          apiFile.endHelpersRegion("Basic Types helpers");
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiBasicTypes(
                                                        ApiFile &apiFile,
                                                        const IEventingTypes::PredefinedTypedefs basicType
                                                        )
        {
          auto &indentStr = apiFile.indent_;

          String cTypeStr = GenerateStructC::fixCType(basicType);

          auto pathStr = GenerateStructC::fixBasicType(basicType);

          apiFile.usingTypedef(cTypeStr, getPythonCType(basicType));

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_wrapperDestroy(handle: " << cTypeStr << "):\n";
            ss << indentStr << "    pass\n";
            ss << indentStr << "\n";
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";

            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_FromC(handle: " << cTypeStr << ") -> " << getPythonConvertedCType(basicType) << ":\n";
            ss << indentStr << "    if (handle is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getCToPythonType("handle", basicType) << "\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_AdoptFromC(handle: " << cTypeStr << ") -> " << getPythonConvertedCType(basicType) << ":\n";
            ss << indentStr << "    if (handle is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getCToPythonType("handle", basicType) << "\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_ToC(value: " << getPythonConvertedCType(basicType) << ") -> " << cTypeStr << ":\n";
            ss << indentStr << "    if (value is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getPythonToCType("value", basicType) << "\n";
            ss << indentStr << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiBoxing(ApiFile &apiFile)
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
        void GenerateStructPython::prepareApiBoxing(
                                                    ApiFile &apiFile,
                                                    const IEventingTypes::PredefinedTypedefs basicType
                                                    )
        {
          auto &indentStr = apiFile.indent_;

          String cTypeStr = GenerateStructC::fixCType(basicType);
          String boxedTypeStr = "box_" + cTypeStr;

          apiFile.usingTypedef(boxedTypeStr, "c_void_p");

          auto pathStr = GenerateStructC::fixBasicType(basicType);
          if (!apiFile.hasBoxing(pathStr)) return;

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << " = __lib.box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << ".restype = " << boxedTypeStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << ".argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "() -> " << boxedTypeStr << ":\n";
            ss << indentStr << "    return Api.__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "()\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue = __lib.box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue.restype = " << boxedTypeStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue.argtypes = [" << cTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(value: " << cTypeStr << ") -> " << boxedTypeStr << ":\n";
            ss << indentStr << "    return Api.__box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(value)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperDestroy = __lib.box_" << cTypeStr << "_wrapperDestroy\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperDestroy.restype = None\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperDestroy.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_wrapperDestroy(handle: " << boxedTypeStr << "):\n";
            ss << indentStr << "    return Api.__box_" << cTypeStr << "_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperInstanceId = __lib.box_" << cTypeStr << "_wrapperInstanceId\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperInstanceId.restype = instance_id_t\n";
            ss << indentStr << "__box_" << cTypeStr << "_wrapperInstanceId.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_wrapperInstanceId(handle: " << boxedTypeStr << ") -> instance_id_t:\n";
            ss << indentStr << "    return Api.__box_" << cTypeStr << "_wrapperInstanceId(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_has_value = __lib.box_" << cTypeStr << "_has_value\n";
            ss << indentStr << "__box_" << cTypeStr << "_has_value.restype = c_bool\n";
            ss << indentStr << "__box_" << cTypeStr << "_has_value.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_has_value(handle: " << boxedTypeStr << " ) -> bool:\n";
            ss << indentStr << "    return bool(Api.__box_" << cTypeStr << "_has_value(handle))\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_get_value = __lib.box_" << cTypeStr << "_get_value\n";
            ss << indentStr << "__box_" << cTypeStr << "_get_value.restype = " << cTypeStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_get_value.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def box_" << cTypeStr << "_get_value(handle: " << boxedTypeStr << ") -> " << cTypeStr << ":\n";
            ss << indentStr << "    return Api.__box_" + cTypeStr + "_get_value(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__box_" << cTypeStr << "_set_value = __lib.box_" << cTypeStr << "_set_value\n";
            ss << indentStr << "__box_" << cTypeStr << "_set_value.restype = None\n";
            ss << indentStr << "__box_" << cTypeStr << "_set_value.argtypes = [" << boxedTypeStr << ", " << cTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << "box_" << cTypeStr << "_set_value(handle: " << boxedTypeStr << ", value: " << cTypeStr << "):\n";
            ss << indentStr << "    Api.__box_" << cTypeStr << "_set_value(handle, value)\n";
            ss << indentStr << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";
            if (IEventingTypes::PredefinedTypedef_binary == basicType) {
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_FromC(handle: " << boxedTypeStr << ") -> bytearray:\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    if (not " << getApiPath() << ".box_" << cTypeStr << "_has_value(handle)):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    binaryHandle = " << getApiPath() << ".box_" << cTypeStr << "_get_value(handle)\n";
              ss << indentStr << "    result = " << cTypeStr << "_FromC(binaryHandle)\n";
              ss << indentStr << "    " << getApiPath() << "." << cTypeStr << "_wrapperDestroy(binaryHandle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_AdoptFromC(handle: " << boxedTypeStr << ") -> bytearray:\n";
              ss << indentStr << "    result = box_" << cTypeStr << "_FromC(handle)\n";
              ss << indentStr << "    " << getApiPath() << ".box_" << cTypeStr << "_wrapperDestroy(handle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_ToC(value: bytes) -> " << boxedTypeStr << ":\n";
              ss << indentStr << "    if (value is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    tempHandle = " << cTypeStr << "_ToC(value)\n";
              ss << indentStr << "    result = " << getApiPath() << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle)\n";
              ss << indentStr << "    " << getApiPath() << "." << cTypeStr << "_wrapperDestroy(tempHandle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
          } else if (IEventingTypes::getBaseType(basicType) == IEventingTypes::BaseType_String) {
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_FromC(handle: " << boxedTypeStr << ") -> str:\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    if (not " << getApiPath() << ".box_" << cTypeStr << "_has_value(handle)):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    tempHandle = " << getApiPath() << ".box_" << cTypeStr << "_get_value(handle)\n";
              ss << indentStr << "    result = " << cTypeStr << "_FromC(tempHandle)\n";
              ss << indentStr << "    " << getApiPath() << "." << cTypeStr << "_wrapperDestroy(tempHandle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_AdoptFromC(handle: " << boxedTypeStr << ") -> str:\n";
              ss << indentStr << "    result = box_" << cTypeStr << "_FromC(handle)\n";
              ss << indentStr << "    " << getApiPath() << ".box_" << cTypeStr << "_wrapperDestroy(handle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_ToC(value: str) -> " << boxedTypeStr << ":\n";
              ss << indentStr << "    if (value is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    tempHandle = " << cTypeStr << "_ToC(value)\n";
              ss << indentStr << "    result = " << getApiPath() << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr << "WithValue(tempHandle)\n";
              ss << indentStr << "    " << getApiPath() << "." << cTypeStr << "_wrapperDestroy(tempHandle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
          } else {
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_FromC(handle: " << boxedTypeStr << ") -> " << getPythonConvertedCType(basicType) << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    if (not " << getApiPath() << ".box_" << cTypeStr << "_has_value(handle)):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << getCToPythonType(getApiPath() + ".box_" + cTypeStr + "_get_value(handle)", basicType) << "\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_AdoptFromC(handle: " << boxedTypeStr << ") -> " << getPythonConvertedCType(basicType) << ":\n";
              ss << indentStr << "    result = box_" << cTypeStr << "_FromC(handle)\n";
              ss << indentStr << "    " << getApiPath() << ".box_" << cTypeStr << "_wrapperDestroy(handle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def box_" << cTypeStr << "_ToC(value: " << getPythonConvertedCType(basicType) << ") -> " << boxedTypeStr << ":\n";
              ss << indentStr << "    if (value is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << getApiPath() << ".box_" << cTypeStr << "_wrapperCreate_" << cTypeStr <<"WithValue(" << getPythonToCType("value", basicType) << ")\n";
              ss << indentStr << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiString(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.usingTypedef("string_t", "c_void_p");
          apiFile.usingTypedef("const_char_star_t", "c_void_p");

          apiFile.startRegion("String API helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_wrapperCreate_string = __lib.string_t_wrapperCreate_string\n";
            ss << indentStr << "__string_t_wrapperCreate_string.restype = string_t\n";
            ss << indentStr << "__string_t_wrapperCreate_string.argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_wrapperCreate_string() -> string_t:\n";
            ss << indentStr << "    return Api.__string_t_wrapperCreate_string()\n";
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_wrapperCreate_stringWithValue = __lib.string_t_wrapperCreate_stringWithValue\n";
            ss << indentStr << "__string_t_wrapperCreate_stringWithValue.restype = string_t\n";
            ss << indentStr << "__string_t_wrapperCreate_stringWithValue.argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_wrapperCreate_stringWithValue(value: str) -> string_t:\n";
            ss << indentStr << "    return Api.__string_t_wrapperCreate_stringWithValue(c_char_p(value))\n";
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_wrapperDestroy = __lib.string_t_wrapperDestroy\n";
            ss << indentStr << "__string_t_wrapperDestroy.restype = None\n";
            ss << indentStr << "__string_t_wrapperDestroy.argtypes = [string_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_wrapperDestroy(handle: string_t):\n";
            ss << indentStr << "    Api.__string_t_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_wrapperInstanceId = __lib.string_t_wrapperInstanceId\n";
            ss << indentStr << "__string_t_wrapperInstanceId.restype = None\n";
            ss << indentStr << "__string_t_wrapperInstanceId.argtypes = [instance_id_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_wrapperInstanceId(handle: string_t) -> instance_id_t:\n";
            ss << indentStr << "    return Api.__string_t_wrapperInstanceId(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_get_value_actual = __lib.string_t_get_value_actual\n";
            ss << indentStr << "__string_t_get_value_actual.restype = const_char_star_t\n";
            ss << indentStr << "__string_t_get_value_actual.argtypes = [string_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_get_value_actual(handle: string_t) -> const_char_star_t:\n";
            ss << indentStr << "    return Api.__string_t_get_value_actual(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_get_value(handle: string_t) -> str:\n";
            ss << indentStr << "    result = string_t_get_value_actual(handle)\n";
            ss << indentStr << "    if (result is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return result.decode('utf-8')\n";
            ss << indentStr << "\n";
            ss << indentStr << "__string_t_set_value = __lib.string_t_set_value\n";
            ss << indentStr << "__string_t_set_value.restype = None\n";
            ss << indentStr << "__string_t_set_value.argtypes = [string_t, c_char_p]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_set_value(handle: string_t, value: str):\n";
            ss << indentStr << "    Api.__string_t_set_value(handle, c_char_p(value))\n";
            ss << indentStr << "\n";
          }
          apiFile.endRegion("String API helpers");

          apiFile.startHelpersRegion("String helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_FromC(handle: string_t) -> str:\n";
            ss << indentStr << "    if (handle is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getApiPath() << ".string_t_get_value(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_AdoptFromC(handle: string_t) -> str:\n";
            ss << indentStr << "    result = string_t_FromC(handle)\n";
            ss << indentStr << "    " << getApiPath() << ".string_t_wrapperDestroy(handle)\n";
            ss << indentStr << "    return result\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def string_t_ToC(value: str) -> string_t:\n";
            ss << indentStr << "    if (value is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getApiPath() << ".string_t_wrapperCreate_stringWithValue(value)\n";
          }

          apiFile.endHelpersRegion("String helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiBinary(ApiFile &apiFile)
        {
          auto &indentStr = apiFile.indent_;

          apiFile.startRegion("Binary API helpers");

          apiFile.usingTypedef("binary_t", "c_void_p");
          apiFile.usingTypedef("ConstBytePtr", "c_void_p");

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_t = __lib.binary_t_wrapperCreate_binary_t\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_t.restype = binary_t\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_t.argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_wrapperCreate_binary_t() -> binary_t:\n";
            ss << indentStr << "    return Api.__binary_t_wrapperCreate_binary_t()\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_tWithValue = __lib.binary_t_wrapperCreate_binary_tWithValue\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_tWithValue.restype = binary_t\n";
            ss << indentStr << "__binary_t_wrapperCreate_binary_tWithValue.argtypes = [c_char_p, binary_size_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_wrapperCreate_binary_tWithValue(value: bytes, size: long) -> binary_t:\n";
            ss << indentStr << "    return Api.__binary_t_wrapperCreate_binary_tWithValue(ctypes.create_string_buffer(value, size), binary_size_t(size))\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_wrapperDestroy = __lib.binary_t_wrapperDestroy\n";
            ss << indentStr << "__binary_t_wrapperDestroy.restype = None\n";
            ss << indentStr << "__binary_t_wrapperDestroy.argtypes = [binary_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_wrapperDestroy(handle: binary_t):\n";
            ss << indentStr << "    Api.__binary_t_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_wrapperInstanceId = __lib.binary_t_wrapperInstanceId\n";
            ss << indentStr << "__binary_t_wrapperInstanceId.restype = instance_id_t\n";
            ss << indentStr << "__binary_t_wrapperInstanceId.argtypes = [binary_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_wrapperInstanceId(handle: binary_t) -> instance_id_t:\n";
            ss << indentStr << "    return Api.__binary_t_wrapperInstanceId(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_get_value = __lib.binary_t_get_value\n";
            ss << indentStr << "__binary_t_get_value.restype = c_void_p\n";
            ss << indentStr << "__binary_t_get_value.argtypes = [binary_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_get_value(handle: binary_t) -> c_void_p:\n";
            ss << indentStr << "    return Api.__binary_t_get_value(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_get_size = __lib.binary_t_get_size\n";
            ss << indentStr << "__binary_t_get_size.restype = binary_size_t\n";
            ss << indentStr << "__binary_t_get_size.argtypes = [binary_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_get_size(handle: binary_t) -> long:\n";
            ss << indentStr << "    return Api.__binary_t_get_size(handle).value\n";
            ss << indentStr << "\n";
            ss << indentStr << "__binary_t_set_value = __lib.binary_t_set_value\n";
            ss << indentStr << "__binary_t_set_value.restype = None\n";
            ss << indentStr << "__binary_t_set_value.argtypes = [binary_t, c_char_p, c_size_t]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_set_value(handle: binary_t, value: bytes, size: long):\n";
            ss << indentStr << "    Api.__binary_t_set_value(handle, ctypes.create_string_buffer(value, size), c_size_t(size))\n";
            ss << indentStr << "\n";
          }
          apiFile.endRegion("Binary API helpers");

          apiFile.startHelpersRegion("Binary helpers");

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_FromC(handle: binary_t) -> bytearray:\n";
            ss << indentStr << "    if (handle is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    length = " << getApiPath() << ".binary_t_get_size(handle)\n";
            ss << indentStr << "    buffer = bytearray()\n";
            ss << indentStr << "    buffer.extend(ctypes.create_string_buffer(" << getApiPath() << ".binary_t_get_value(handle), length))\n";
            ss << indentStr << "    return buffer\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_AdoptFromC(handle: binary_t) -> bytearray:\n";
            ss << indentStr << "    result = binary_t_FromC(handle)\n";
            ss << indentStr << "    " << getApiPath() << ".binary_t_wrapperDestroy(handle)\n";
            ss << indentStr << "    return result\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def binary_t_ToC(value: bytes) -> binary_t:\n";
            ss << indentStr << "    if (value is None):\n";
            ss << indentStr << "        return None\n";
            ss << indentStr << "    return " << getApiPath() << ".binary_t_wrapperCreate_binary_tWithValue(value, len(value))\n";
            ss << indentStr << "\n";
          }

          apiFile.endHelpersRegion("Binary helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiDuration(ApiFile &apiFile)
        {
          apiFile.startRegion("Time API helpers");
          apiFile.startHelpersRegion("Time helpers");

          prepareApiDuration(apiFile, "Time");
          prepareApiDuration(apiFile, "Days");
          prepareApiDuration(apiFile, "Hours");
          prepareApiDuration(apiFile, "Minutes");
          prepareApiDuration(apiFile, "Seconds");
          prepareApiDuration(apiFile, "Milliseconds");
          prepareApiDuration(apiFile, "Microseconds");
          prepareApiDuration(apiFile, "Nanoseconds");

          apiFile.endRegion("Time API helpers");
          apiFile.endHelpersRegion("Time helpers");
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiDuration(
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

          apiFile.usingTypedef(cTypeStr, "c_void_p");
          apiFile.usingTypedef("datetime");

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << " = __lib.zs_" << durationType << "_wrapperCreate_" << durationType << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << ".restype = " << cTypeStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << ".argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_wrapperCreate_" << durationType << "() -> " << cTypeStr << ":\n";
            ss << indentStr << "    return Api.__zs_" << durationType << "_wrapperCreate_" << durationType << "()\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue = __lib.zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue.restype = " << cTypeStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue.argtypes = [c_long]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value: long) -> " << cTypeStr << ":\n";
            ss << indentStr << "    return Api.__zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(value)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperDestroy = __lib.zs_" << durationType << "_wrapperDestroy\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperDestroy.restype = None\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperDestroy.argtypes = [" << cTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_wrapperDestroy(handle: " << cTypeStr << "):\n";
            ss << indentStr << "    Api.__zs_" << durationType << "_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperInstanceId = __lib.zs_" << durationType << "_wrapperInstanceId\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperInstanceId.restype = instance_id_t\n";
            ss << indentStr << "__zs_" << durationType << "_wrapperInstanceId.argtypes = [" << cTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_wrapperInstanceId(handle: " << cTypeStr << ") -> instance_id_t:\n";
            ss << indentStr << "    return Api.__zs_" << durationType << "_wrapperInstanceId(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_get_value = __lib.zs_" << durationType << "_get_value\n";
            ss << indentStr << "__zs_" << durationType << "_get_value.restype = c_long\n";
            ss << indentStr << "__zs_" << durationType << "_get_value.argtypes = [" << cTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_get_value(handle: " << cTypeStr << ") -> long:\n";
            ss << indentStr << "    return Api.__zs_" << durationType << "_get_value(handle).value\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << durationType << "_set_value = __lib.zs_" << durationType << "_get_value\n";
            ss << indentStr << "__zs_" << durationType << "_set_value.restype = None\n";
            ss << indentStr << "__zs_" << durationType << "_set_value.argtypes = [" << cTypeStr << ", c_long]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_set_value(handle: " << cTypeStr << ", value: long):\n";
            ss << indentStr << "    Api.__zs_" << durationType << "_set_value(handle, c_long(value))\n";
            ss << indentStr << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_FromC(handle: " << cTypeStr << ") -> " << fixPythonPathType(durationContext) << ":\n";
            ss << indentStr << "    ";
            if ("Time" == durationType) {
              ss << "micros = (" << getApiPath() << ".zs_" << durationType << "_get_value(handle) - 116444736000000000) // 10\n";
              ss << indentStr << "    return datetime.datetime(1970, 1, 1) + timedelta(microseconds = micros)\n";
            } else if ("Nanoseconds" == durationType) {
              ss << "return datetime.timedelta(microseconds = " << getApiPath() << ".zs_" << durationType << "_get_value(handle) // 1000)\n";
            } else if ("Microseconds" == durationType) {
              ss << "return datetime.timedelta(microseconds = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            } else if ("Milliseconds" == durationType) {
              ss << "return datetime.timedelta(milliseconds = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            } else if ("Seconds" == durationType) {
              ss << "return datetime.timedelta(seconds = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            } else if ("Minutes" == durationType) {
              ss << "return datetime.timedelta(minutes = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            } else if ("Hours" == durationType) {
              ss << "return datetime.timedelta(hours = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            } else if ("Days" == durationType) {
              ss << "return datetime.timedelta(days = " << getApiPath() << ".zs_" << durationType << "_get_value(handle))\n";
            }
            ss << indentStr << "\n";

            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_AdoptFromC(handle: " << cTypeStr << ") -> " << fixPythonPathType(durationContext) << ":\n";
            ss << indentStr << "  result = zs_" << durationType << "_FromC(handle)\n";
            ss << indentStr << "  " << getApiPath() << ".zs_" << durationType << "_wrapperDestroy(handle)\n";
            ss << indentStr << "  return result\n";
            ss << indentStr << "\n";

            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << durationType << "_ToC(value: " << fixPythonPathType(durationContext) << ") -> " << cTypeStr << ":\n";
            if ("Time" == durationType) {
              ss << indentStr << "    offset = value - datetime.datetime(1970, 1, 1)\n";
              ss << indentStr << "    nanosoffset = (((offset.days * 24 * 60 * 1000 * 1000) + (offset.seconds * 1000 * 1000) + (offset.microseconds)) * 10) + 116444736000000000\n";
              ss << indentStr << "    return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(nanosoffset)\n";
            } else {
              ss << indentStr << "    micros = (value.days * 24 * 60 * 1000 * 1000) + (value.seconds * 1000 * 1000) + (value.microseconds)\n";
              ss << indentStr << "    ";
              if ("Nanoseconds" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros * 1000)\n";
              } else if ("Microseconds" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros)\n";
              } else if ("Milliseconds" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros // 1000)\n";
              } else if ("Seconds" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros // 1000000)\n";
              } else if ("Minutes" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros // 60000000)\n";
              } else if ("Hours" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros // 3600000000)\n";
              } else if ("Days" == durationType) {
                ss << "return " << getApiPath() << ".zs_" << durationType << "_wrapperCreate_" << durationType << "WithValue(micros // 86400000000)\n";
              }
            }
            ss << indentStr << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiList(
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
            
            apiFile.usingTypedef("iterator_handle_t", "c_void_p");
            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "c_void_p");
            apiFile.usingTypedef(keyType);
            apiFile.usingTypedef(listType);

            {
              auto &ss = apiFile.structSS_;
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << " = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << ".restype = " << GenerateStructC::fixCType(templatedStructType) << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << ".argtypes = []\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "() -> " << GenerateStructC::fixCType(templatedStructType) << ":\n";
              ss << indentStr << "    return Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << structType->getMappingName() << "\n";
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy.restype = None\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy(handle: " << GenerateStructC::fixCType(templatedStructType) << "):\n";
              ss << indentStr << "    Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperDestroy(handle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId.restype = instance_id_t\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId(handle: " << GenerateStructC::fixCType(templatedStructType) << ") -> instance_id_t:\n";
              ss << indentStr << "    return Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId(handle)\n";
              if (isMap) {
                ss << indentStr << "\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert = __lib." << GenerateStructC::fixType(templatedStructType) << "_insert\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert.restype = None\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << ", " << fixCPythonType(keyType) << ", " << fixCPythonType(listType) <<  "]\n";
                ss << indentStr << "\n";
                ss << indentStr << "@staticmethod\n";
                ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_insert(handle: " << GenerateStructC::fixCType(templatedStructType) << ", key: " << fixCPythonType(keyType) << ", value: " << fixCPythonType(listType) << "):\n";
                ss << indentStr << "    Api.__" << GenerateStructC::fixType(templatedStructType) << "_insert(handle, key, value)\n";
              } else {
                ss << indentStr << "\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert = __lib." << GenerateStructC::fixType(templatedStructType) << "_insert\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert.restype = None\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_insert.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << ", " << fixCPythonType(listType) <<  "]\n";
                ss << indentStr << "\n";
                ss << indentStr << "@staticmethod\n";
                ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_insert(handle: " << GenerateStructC::fixCType(templatedStructType) << " , value: " << fixCPythonType(listType) << "):\n";
                ss << indentStr << "     Api.__" << GenerateStructC::fixType(templatedStructType) << "_insert(handle, value)\n";
              }

              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin.restype = iterator_handle_t\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin(handle: " << GenerateStructC::fixCType(templatedStructType) << ") -> iterator_handle_t:\n";
              ss << indentStr << "    return Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperInstanceId(handle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext.restype = None\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterHandle: iterator_handle_t):\n";
              ss << indentStr << "    Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterHandle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd.restype = c_bool\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd.argtypes = [" << GenerateStructC::fixCType(templatedStructType) << " , iterator_handle_t]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(handle: " << GenerateStructC::fixCType(templatedStructType) << ", iterHandle: iterator_handle_t) -> bool:\n";
              ss << indentStr << "    return Api.__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(handle, iterHandle).value\n";
              if (isMap) {
                ss << indentStr << "\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey.restype = " << fixCPythonType(keyType) << "\n";
                ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey.argtypes = [iterator_handle_t]\n";
                ss << indentStr << "\n";
                ss << indentStr << "@staticmethod\n";
                ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey(iterHandle: iterator_handle_t) -> " << fixCPythonType(keyType) << ":\n";
                ss << indentStr << "    return Api.__" + GenerateStructC::fixType(templatedStructType) + "_wrapperIterKey(iterHandle)\n";
              }
              ss << indentStr << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue = __lib." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue.restype = " << fixCPythonType(listType) << "\n";
              ss << indentStr << "__" << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue.argtypes = [iterator_handle_t]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue(iterHandle: iterator_handle_t) -> " << fixCPythonType(listType) << " :\n";
              ss << indentStr << "    return Api.__" + GenerateStructC::fixType(templatedStructType) + "_wrapperIterValue(iterHandle)\n";
              ss << indentStr << "\n";
            }
            {
              auto &ss = apiFile.helpersSS_;

              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_FromC(handle: " << GenerateStructC::fixCType(templatedStructType) << ") -> ";
              if (isList) {
                ss << "list";
              } else if (isMap) {
                ss << "dict";
              } else {
                ss << "dict";
              }
              ss << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    result = ";
              if (isList) {
                ss << "[]";
              } else if (isMap) {
                ss << "{}";
              } else {
                ss << "{}";
              }
              ss << "\n";
              ss << indentStr << "    iterHandle = " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterBegin(handle)\n";
              ss << indentStr << "    while (not " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterIsEnd(handle, iterHandle)):\n";
              if (isMap) {
                ss << indentStr << "        cKey = " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterKey(iterHandle)\n";
              }
              ss << indentStr << "        cValue = " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterValue(iterHandle)\n";

              if (isMap) {
                ss << indentStr << "        pyKey = " << getAdoptFromCMethod(apiFile, false, listType) << "(cKey)\n";
              }
              ss << indentStr << "        pyValue = " << getAdoptFromCMethod(apiFile, false, listType) << "(cValue)\n";
              if (isList) {
                ss << indentStr << "        result.append(pyValue)\n";
              } else if (isMap) {
                ss << indentStr << "        result[pyKey] = pyValue\n";
              } else {
                ss << indentStr << "        result[pyValue] = None\n";
              }
              ss << indentStr << "        " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperIterNext(iterHandle)\n";
              ss << indentStr << "    return result\n";

              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_AdoptFromC(handle: " << GenerateStructC::fixCType(templatedStructType) << ") -> ";
              if (isList) {
                ss << "list";
              } else if (isMap) {
                ss << "dict";
              } else {
                ss << "dict";
              }
              ss << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    result = " << GenerateStructC::fixType(templatedStructType) << "_FromC(handle)\n";
              ss << indentStr << "    " << getDestroyCMethod(apiFile, false, templatedStructType) << "(handle)\n";
              ss << indentStr << "    return result\n";

              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(templatedStructType) << "_ToC(values: " << fixPythonPathType(templatedStructType) << ") -> " << GenerateStructC::fixCType(templatedStructType) << ":\n";
              ss << indentStr << "    if (values is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    handle = " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_wrapperCreate_" << listOrSetStr << "()\n";
              if (isList) {
                ss << indentStr << "    for value in values:\n";
              } else if (isMap) {
                ss << indentStr << "    for key, value in values:\n";
              } else {
                ss << indentStr << "    for key, value in values:\n";
              }
              if (isList) {
                ss << indentStr << "        cValue = " << getToCMethod(apiFile, false, listType) << "(value)\n";
                ss << indentStr << "        " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_insert(handle, cValue)\n";
              } else if (isMap) {
                ss << indentStr << "        cKey = " << getToCMethod(apiFile, false, keyType) << "(key)\n";
                ss << indentStr << "        cValue = " << getToCMethod(apiFile, false, listType) << "(value)\n";
                ss << indentStr << "        " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) <<  "_insert(handle, cKey, cValue)\n";
                auto destroyStr = getDestroyCMethod(apiFile, false, keyType);
                if (destroyStr.hasData()) {
                  ss << indentStr << "        " << destroyStr << "(cKey)\n";
                }
              } else {
                ss << indentStr << "        cValue = " << getToCMethod(apiFile, false, listType) << "(key)\n";
                ss << indentStr << "        " << getApiPath() << "." << GenerateStructC::fixType(templatedStructType) << "_insert(handle, cValue)\n";
              }
              {
                auto destroyStr = getDestroyCMethod(apiFile, false, listType);
                if (destroyStr.hasData()) {
                  ss << indentStr << "        " << destroyStr << "(cValue)\n";
                }
              }
              ss << indentStr << "    return handle\n";
              ss << indentStr << "\n";

            }
          }
          apiFile.endRegion(listOrSetStr + " API helpers");
          apiFile.endHelpersRegion(listOrSetStr + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiSpecial(
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

          apiFile.usingTypedef(GenerateStructC::fixCType(contextStruct), "c_void_p");

          apiFile.startRegion(specialName + " API helpers");
          apiFile.startHelpersRegion(specialName + " helpers");

          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperDestroy = __lib.zs_" << specialName << "_wrapperDestroy\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperDestroy.restype = None\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperDestroy.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << specialName << "_wrapperDestroy(handle: " << GenerateStructC::fixCType(contextStruct) << "):\n";
            ss << indentStr << "    Api.__zs_" << specialName << "_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperInstanceId = __lib.zs_" << specialName << "_wrapperInstanceId\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperInstanceId.restype = instance_id_t\n";
            ss << indentStr << "__zs_" << specialName << "_wrapperInstanceId.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_" << specialName << "_wrapperInstanceId(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> instance_id_t:\n";
            ss << indentStr << "    return Api.__zs_" << specialName << "_wrapperInstanceId(handle)\n";
            if (isPromise) {
              ss << indentStr << "\n";
              ss << indentStr << "__zs_" << specialName << "_wrapperObserveEvents = __lib.zs_" << specialName << "_wrapperObserveEvents\n";
              ss << indentStr << "__zs_" << specialName << "_wrapperObserveEvents.restype = event_observer_t\n";
              ss << indentStr << "__zs_" << specialName << "_wrapperObserveEvents.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_" << specialName << "_wrapperObserveEvents(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> event_observer_t:\n";
              ss << indentStr << "    return Api.__zs_" << specialName << "_wrapperObserveEvents(handle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "__zs_" << specialName << "_get_id = __lib.zs_" << specialName << "_get_id\n";
              ss << indentStr << "__zs_" << specialName << "_get_id.restype = event_observer_t\n";
              ss << indentStr << "__zs_" << specialName << "_get_id.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_" << specialName << "_get_id(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> c_uint64:\n";
              ss << indentStr << "    return Api.__zs_" << specialName << "_get_id(handle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "__zs_" << specialName << "_isSettled = __lib.zs_" << specialName << "_isSettled\n";
              ss << indentStr << "__zs_" << specialName << "_isSettled.restype = c_bool\n";
              ss << indentStr << "__zs_" << specialName << "_isSettled.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_" << specialName << "_isSettled(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> bool:\n";
              ss << indentStr << "    return Api.__zs_" << specialName << "_isSettled(handle).value\n";
              ss << indentStr << "\n";
              ss << indentStr << "__zs_" << specialName << "_isResolved = __lib.zs_" << specialName << "_isResolved\n";
              ss << indentStr << "__zs_" << specialName << "_isResolved.restype = c_bool\n";
              ss << indentStr << "__zs_" << specialName << "_isResolved.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_" << specialName << "_isResolved(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> bool:\n";
              ss << indentStr << "    return Api.__zs_" << specialName << "_isResolved(handle).value\n";
              ss << indentStr << "\n";
              ss << indentStr << "__zs_" << specialName << "_isRejected = __lib.zs_" << specialName << "_isRejected\n";
              ss << indentStr << "__zs_" << specialName << "_isRejected.restype = c_bool\n";
              ss << indentStr << "__zs_" << specialName << "_isRejected.argtypes = [" << GenerateStructC::fixCType(contextStruct) << "]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_" << specialName << "_isRejected(handle: " << GenerateStructC::fixCType(contextStruct) << ") -> bool:\n";
              ss << indentStr << "    return Api.__zs_" << specialName << "_isRejected(handle).value\n";
            }
            ss << indentStr << "\n";
          }

          if (isPromise) {
            auto &ss = apiFile.helpersSS_;
            ss << "\n";

            static const char *promiseAdopt =
              "@staticmethod\n"
              "def zs_Promise_AdoptFromC(handle: zs_Promise_t): # -> wrapper.Promise:\n"
              "    if (handle is None):\n"
              "        return None\n"
              "    \n"
              "    result = wrapper.Promise()\n"
              "    result.handleHolder = handle\n"
              "    \n"
              "    instanceId = $API$.zs_Promise_wrapperInstanceId(handle)\n"
              "    def __promiseCallback(target: wrapper.Promise, methodName, callbackHandle: callback_event_t):\n"
              "        handle = target.handleHolder\n"
              "        resolved = $API$.zs_Promise_isResolved(handle)\n"
              "        if (resolved):\n"
              "            promise.resolve(None)\n"
              "        else:\n"
              "            promise.reject(zs_PromiseRejectionReason_FromC(handle))\n"
              "        \n"
              "        $API$.callback_wrapperObserverDestroy(eventObserverHandle)\n"
              "        $API$.zs_Promise_wrapperDestroy(handle)\n"
              "    \n"
              "    observeEvents(\"::zs\", \"Promise\", instanceId, result, __promiseCallback)\n"
              "    return result\n"
              "\n"
              ;
            String promiseAdoptStr(promiseAdopt);
            promiseAdoptStr.replaceAll("$API$", getApiPath());
            GenerateHelper::insertBlob(ss, indentStr, promiseAdoptStr, true);
            ss << indentStr << "\n";
          } else if (isAny) {
            {
              auto &ss = apiFile.helpersSS_;

              String cTypeStr = GenerateStructC::fixCType(contextStruct);

              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(contextStruct) << "_FromC(handle: " << cTypeStr << "): # -> " << fixPythonPathType(contextStruct) << ":\n";
              ss << indentStr << "    return handle\n";
              ss << indentStr << "\n";

              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(contextStruct) << "_AdoptFromC(handle: " << cTypeStr << "): # -> " << fixPythonPathType(contextStruct) << ":\n";
              ss << indentStr << "    result = " << GenerateStructC::fixType(contextStruct) << "_FromC(handle)\n";
              ss << indentStr << "    " << getDestroyCMethod(apiFile, false, contextStruct) << "(handle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";

              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << GenerateStructC::fixType(contextStruct) << "_ToC(value: " << fixPythonPathType(contextStruct) << ") -> " << cTypeStr << ":\n";
              ss << indentStr << "    raise NotImplementedError(\"Promise to C type not implemented\")\n";
              ss << indentStr << "\n";
            }
          }

          apiFile.endRegion(specialName + " API helpers");
          apiFile.endHelpersRegion(specialName + " helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::preparePromiseWithValue(ApiFile &apiFile)
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

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "c_void_p");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << indentStr << "\n";
              ss << indentStr << "__zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << " = __lib.zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << "\n";
              ss << indentStr << "__zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << ".restype = " << GenerateStructC::fixCType(promiseType) << "\n";
              ss << indentStr << "__zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << ".argtypes = [zs_Promise_t]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << "(handle: zs_Promise_t) -> " << GenerateStructC::fixCType(promiseType) << ":\n";
              ss << indentStr << "    return Api.__zs_PromiseWith_resolveValue_" << GenerateStructC::fixType(promiseType) << "(handle)\n";
              ss << indentStr << "\n";
            }
            {
              auto &ss = apiFile.helpersSS_;
              ss << "\n";

              static const char *promiseAdopt =
                "@staticmethod\n"
                "def zs_Promise_AdoptFromC(handle: zs_Promise_t): # -> wrapper.Promise:\n"
                "    if (handle is None):\n"
                "        return None\n"
                "    \n"
                "    result = wrapper.Promise()\n"
                "    result.handleHolder = handle\n"
                "    \n"
                "    instanceId = $API$.zs_Promise_wrapperInstanceId(handle)\n"
                "    def __promiseCallback(target: wrapper.Promise, methodName, callbackHandle: callback_event_t):\n"
                "        handle = target.handleHolder\n"
                "        resolved = $API$.zs_Promise_isResolved(handle)\n"
                "        if (resolved):\n"
                "            promise.resolve($ADOPTMETHOD$($RESOLVEMETHOD$(handle)))\n"
                "        else:\n"
                "            promise.reject(zs_PromiseRejectionReason_FromC(handle))\n"
                "        \n"
                "        $API$.callback_wrapperObserverDestroy(eventObserverHandle)\n"
                "        $API$.zs_Promise_wrapperDestroy(handle)\n"
                "    \n"
                "    observeEvents(\"::zs\", \"Promise\", instanceId, result, __promiseCallback)\n"
                "    return result\n"
                "\n"
                ;
              String promiseAdoptStr(promiseAdopt);
              promiseAdoptStr.replaceAll("$API$", getApiPath());
              promiseAdoptStr.replaceAll("$ZSPROMISEWITHTYPE$", GenerateStructC::fixType(templatedStructType));
              promiseAdoptStr.replaceAll("$ZSPROMISEWITHCTYPE$", GenerateStructC::fixCType(templatedStructType));
              promiseAdoptStr.replaceAll("$RESOLVECSTYPE$", fixPythonPathType(promiseType));
              promiseAdoptStr.replaceAll("$ADOPTMETHOD$", getAdoptFromCMethod(apiFile, false, promiseType));
              promiseAdoptStr.replaceAll("$RESOLVEMETHOD$", String(getApiPath() + ".zs_PromiseWith_resolveValue_" + GenerateStructC::fixType(promiseType)));

              GenerateHelper::insertBlob(ss, indentStr, promiseAdoptStr, true);
              ss << indentStr << "\n";
            }
          }

          apiFile.endRegion("PromiseWith API helpers");
          apiFile.endHelpersRegion("PromiseWith helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::preparePromiseWithRejectionReason(ApiFile &apiFile)
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
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def zs_PromiseRejectionReason_FromC(handle: zs_Promise_t) -> BaseException:\n";
            ss << indentStr << "    if (handle is None):\n";
            ss << indentStr << "        return None\n";
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

            apiFile.usingTypedef(GenerateStructC::fixCType(templatedStructType), "c_void_p");
            apiFile.usingTypedef(promiseType);

            {
              auto &ss = apiFile.structSS_;
              ss << indentStr << "\n";
              ss << indentStr << "__zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << " = __lib.zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "\n";
              ss << indentStr << "__zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << ".restype = " << GenerateStructC::fixCType(promiseType) << "\n";
              ss << indentStr << "__zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << ".argtypes = [zs_Promise_t]\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(handle: zs_Promise_t) -> " << GenerateStructC::fixCType(promiseType) << ":\n";
              ss << indentStr << "    return Api.__zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(handle)\n";
            }
            {
              auto &ss = apiFile.helpersSS_;
              ss << indentStr << "    result = " << getAdoptFromCMethod(apiFile, false, promiseType) << "(" << getApiPath() <<  ".zs_PromiseWith_rejectReason_" << GenerateStructC::fixType(promiseType) << "(handle))\n";
              ss << indentStr << "    if (result is not None):\n";
              ss << indentStr << "        return result\n";
            }
          }

          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "    return None\n";
            ss << indentStr << "\n";
          }

          apiFile.endRegion("PromiseWithRejectionReason API helpers");
          apiFile.endHelpersRegion("PromiseWithRejectionReason helpers");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareApiNamespace(
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
        void GenerateStructPython::prepareApiStruct(
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
        void GenerateStructPython::prepareApiEnum(
                                                  ApiFile &apiFile,
                                                  EnumTypePtr enumObj
                                                  )
        {
          auto &indentStr = apiFile.indent_;

          if (!enumObj) return;
          bool hasBoxing = apiFile.hasBoxing(enumObj->getPathName());

          String cTypeStr = GenerateStructC::fixCType(enumObj);
          String boxedTypeStr = "box_" + cTypeStr;
          String csTypeStr = fixPythonPathType(enumObj);
          String csEnumName = fixName(enumObj->getMappingName());

          auto parent = enumObj->getParent();
          String fromStr;
          String importNameStr;
          String asStr;
          String usageStr;
          auto parentNamespace = parent->toNamespace();
          if (parentNamespace) {
            fromStr = fixNamespace(parentNamespace) + ".enums";
            importNameStr = csEnumName;
            asStr = csEnumName;
            usageStr = importNameStr;
          }
          auto parentStruct = parent->toStruct();
          if (parentStruct) {
            fromStr = fixNamePath(parentStruct);
            importNameStr = fixPythonPathType(parentStruct);
            asStr = importNameStr;
            usageStr = importNameStr + "." + csEnumName;
          }

          apiFile.usingTypedef(cTypeStr, fixCsSystemType(enumObj->mBaseType));
          apiFile.usingTypedef(boxedTypeStr, "c_void_p");

          if (hasBoxing)
          {
            auto &ss = apiFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << " = __lib." << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << ".restype = " << boxedTypeStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << ".argtypes = []\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "() -> " << boxedTypeStr << ":\n";
            ss << indentStr << "    return Api.__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "()\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue = __lib." << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue.restype = " << boxedTypeStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue.argtypes = [" << fixCPythonType(enumObj->mBaseType) << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(value: " << fixCPythonType(enumObj->mBaseType) << ") -> " << boxedTypeStr << ":\n";
            ss << indentStr << "    return Api.__" << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(value)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperDestroy = __lib." << boxedTypeStr << "_wrapperDestroy\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperDestroy.restype = None\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperDestroy.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_wrapperDestroy(handle: " << boxedTypeStr << "):\n";
            ss << indentStr << "    Api.__" << boxedTypeStr << "_wrapperDestroy(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperInstanceId = __lib." << boxedTypeStr << "_wrapperInstanceId\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperInstanceId.restype = instance_id_t\n";
            ss << indentStr << "__" << boxedTypeStr << "_wrapperInstanceId.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_wrapperInstanceId(handle: " << boxedTypeStr << ") -> instance_id_t:\n";
            ss << indentStr << "    return Api.__" << boxedTypeStr << "_wrapperInstanceId(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_has_value = __lib." << boxedTypeStr << "_has_value\n";
            ss << indentStr << "__" << boxedTypeStr << "_has_value.restype = c_bool\n";
            ss << indentStr << "__" << boxedTypeStr << "_has_value.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_has_value(handle: " << boxedTypeStr << ") -> bool:\n";
            ss << indentStr << "    return Api.__" << boxedTypeStr << "_has_value(handle).value\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_get_value = __lib." << boxedTypeStr << "_get_value\n";
            ss << indentStr << "__" << boxedTypeStr << "_get_value.restype = " << fixCPythonType(enumObj->mBaseType) << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_get_value.argtypes = [" << boxedTypeStr << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_get_value(handle: " << boxedTypeStr << ") -> " << fixCPythonType(enumObj->mBaseType) << ":\n";
            ss << indentStr << "    return Api.__" << boxedTypeStr << "_get_value(handle)\n";
            ss << indentStr << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_set_value = __lib." << boxedTypeStr << "_set_value\n";
            ss << indentStr << "__" << boxedTypeStr << "_set_value.restype = " << fixCPythonType(enumObj->mBaseType) << "\n";
            ss << indentStr << "__" << boxedTypeStr << "_set_value.argtypes = [" << boxedTypeStr << ", " << fixCPythonType(enumObj->mBaseType) << "]\n";
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << boxedTypeStr << "_set_value(handle: " << boxedTypeStr << ", value: " << fixCPythonType(enumObj->mBaseType) << "):\n";
            ss << indentStr << "    Api.__" << boxedTypeStr << "_set_value(handle, value)\n";
            ss << indentStr << "\n";
          }
          {
            auto &ss = apiFile.helpersSS_;
            ss << indentStr << "\n";
            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_FromC(value: " << cTypeStr << "): # -> " << csTypeStr << ":\n";
            ss << indentStr << "    from " << fromStr << " import " << importNameStr << " as " << asStr << "\n";
            ss << indentStr << "    return " << usageStr << "(value)\n";
            ss << indentStr << "\n";

            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "def " << cTypeStr << "_AdoptFromC(value: " << cTypeStr << "): # -> " << csTypeStr << ":\n";
            ss << indentStr << "    from " << fromStr << " import " << importNameStr << " as " << asStr << "\n";
            ss << indentStr << "    return " << usageStr << "(value)\n";
            ss << indentStr << "\n";

            ss << indentStr << "@staticmethod\n";
            ss << indentStr << "# def " << cTypeStr << "_ToC(value: " << csTypeStr << ") -> " << cTypeStr << ":\n";
            ss << indentStr << "def " << cTypeStr << "_ToC(value) -> " << cTypeStr << ":\n";
            ss << indentStr << "    return " << getPythonToCType("value.value", enumObj->mBaseType) << "\n";

            if (hasBoxing) {
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << boxedTypeStr << "_FromC(handle: " << boxedTypeStr << "): # -> " << csTypeStr << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    if (not " << getApiPath() << "." << boxedTypeStr << "_has_value(handle)):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << cTypeStr << "(" << getCToPythonType(getApiPath() + "." + boxedTypeStr + "_get_value(handle)", enumObj->mBaseType) << ")\n";
              ss << indentStr << "\n";

              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << boxedTypeStr << "_AdoptFromC(handle: " << boxedTypeStr << "): # -> " << csTypeStr << ":\n";
              ss << indentStr << "    result = " << boxedTypeStr << "_FromC(handle)\n";
              ss << indentStr << "    " << getApiPath() << "." << boxedTypeStr << "_wrapperDestroy(handle)\n";
              ss << indentStr << "    return result\n";
              ss << indentStr << "\n";

              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "# def " << boxedTypeStr << "_ToC(value: " << csTypeStr << ") -> " << boxedTypeStr << ":\n";
              ss << indentStr << "def " << boxedTypeStr << "_ToC(value) -> " << boxedTypeStr << ":\n";
              ss << indentStr << "    if (value is not None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << getApiPath() << "." << boxedTypeStr << "_wrapperCreate_" << boxedTypeStr << "WithValue(" << getPythonToCType("value.value", enumObj->mBaseType) <<  ")\n";
              ss << indentStr << "\n";
            }
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareInitFile(
                                                   InitFile &initFile,
                                                   NamespacePtr namespaceObj
                                                   )
        {
          {
            auto &ss = initFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";

            if (namespaceObj->mEnums.size() > 0) {
              ss << "from .enums import *\n\n";
            }

            for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
              auto &subNamespaceObj = (*iter).second;
              if (!subNamespaceObj) continue;

              ss << "import " << fixNamespace(subNamespaceObj) << "\n";
            }
          }
        }


        //---------------------------------------------------------------------
        void GenerateStructPython::prepareEnumFile(
                                                   EnumFile &enumFile,
                                                   NamespacePtr namespaceObj
                                                   )
        {
          {
            auto &ss = enumFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";
            ss << "\n";
            ss << "from aenum import Enum\n\n";
            ss << "from aenum import auto\n\n";
          }

          prepareEnumNamespace(enumFile, namespaceObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::prepareEnumNamespace(
                                                        EnumFile &enumFile,
                                                        NamespacePtr namespaceObj
                                                        )
        {
          auto &indentStr = enumFile.indent_;
          auto &ss = enumFile.namespaceSS_;

          if (!namespaceObj) return;

          for (auto iter = namespaceObj->mEnums.begin(); iter != namespaceObj->mEnums.end(); ++iter) {
            auto enumObj = (*iter).second;
            if (!enumObj) continue;

            ss << "\n";
            ss << GenerateHelper::getDocumentation(indentStr + "# ", enumObj, 80);            
            ss << indentStr << "class " << fixName(enumObj->getMappingName()) << "(Enum):\n";

            enumFile.indentMore();

            bool first = true;
            for (auto iterVal = enumObj->mValues.begin(); iterVal != enumObj->mValues.end(); ++iterVal) {
              auto valueObj = (*iterVal);

              ss << GenerateHelper::getDocumentation(indentStr + "# ", valueObj, 80);
              ss << indentStr << fixName(valueObj->getMappingName());
              if (valueObj->mValue.hasData()) {
                ss << " = " << valueObj->mValue;
              } else {
                if (!first) {
                  ss << " = auto()";
                } else {
                  ss << " = 0";
                }
              }
              first = false;
              ss << "\n";
            }

            enumFile.indentLess();
            ss << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processNamespace(
                                                    const String &inPathStr,
                                                    SetupFile &setupFile,
                                                    ApiFile &apiFile,
                                                    NamespacePtr namespaceObj
                                                    )
        {
          if (!namespaceObj) return;

          auto pathName = namespaceObj->getPathName();
          if ("::std" == pathName) return;
          if ("::zs" == pathName) return;
          if ("::zs::exceptions" == pathName) return;

          String pathStr = inPathStr;

          if (!namespaceObj->isGlobal()) {
            pathStr = UseHelper::fixRelativeFilePath(pathStr, namespaceObj->getMappingName());
            try {
              UseHelper::mkdir(pathStr);
            } catch (const StdError &e) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
            }
            pathStr += "/";

            {
              auto &ss = setupFile.usingAliasSS_;

              ss << "setattr(" << fixNamespace(namespaceObj) << ", \"wrapper\", wrapper)\n";
              for (auto iter = setupFile.global_->mNamespaces.begin(); iter != setupFile.global_->mNamespaces.end(); ++iter) {
                auto &rootNamespaceObj = (*iter).second;
                if (!rootNamespaceObj) continue;

                auto rootPathName = rootNamespaceObj->getPathName();
                if ("::zs" == rootPathName) continue;
                if ("::std" == rootPathName) continue;

                ss << "setattr("<< fixNamespace(namespaceObj) << ", \"" << fixNamespace(rootNamespaceObj) << "\", " << fixNamespace(rootNamespaceObj) << ")\n";
              }
            }
          }

          InitFile initFile;
          initFile.project_ = apiFile.project_;
          initFile.global_ = apiFile.global_;

          initFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("__init__.py"));
          prepareInitFile(initFile, namespaceObj);

          if (namespaceObj->mEnums.size() > 0) {
            EnumFile enumFile;
            enumFile.project_ = apiFile.project_;
            enumFile.global_ = apiFile.global_;

            enumFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("enums.py"));
            prepareEnumFile(enumFile, namespaceObj);
            finalizeEnumFile(enumFile);
          }

          for (auto iter = namespaceObj->mNamespaces.begin(); iter != namespaceObj->mNamespaces.end(); ++iter) {
            auto subNamespaceObj = (*iter).second;
            processNamespace(pathStr, setupFile, apiFile, subNamespaceObj);
          }

          for (auto iter = namespaceObj->mStructs.begin(); iter != namespaceObj->mStructs.end(); ++iter) {
            auto subStructObj = (*iter).second;
            processStruct(pathStr, apiFile, initFile, subStructObj);
          }

          if (!namespaceObj->isGlobal()) {
            finalizeInitFile(initFile);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processStruct(
                                                 const String &pathStr,
                                                 ApiFile &apiFile,
                                                 InitFile &initFile,
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
          structFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, fixPythonType(structObj) + ".py");

          StructFile interfaceFile(apiFile, structObj);
          interfaceFile.project_ = apiFile.project_;
          interfaceFile.global_ = apiFile.global_;
          interfaceFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, "I" + fixPythonType(structObj) + ".py");

          {
            auto &ss = initFile.importSS_;

            ss << "from ." << fixPythonType(structObj) << " import *\n";
          }

          {
            auto &ss = structFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";

            structFile.usingTypedef("wrapper");
          }

          {
            auto &ss = interfaceFile.importSS_;
            ss << "# " ZS_EVENTING_GENERATED_BY "\n\n";

            interfaceFile.usingTypedef("wrapper");
          }

          auto &indentStr = structFile.indent_;

          if (interfaceFile.shouldDefineInterface_) {
            auto &ss = interfaceFile.interfaceSS_;
            ss << GenerateHelper::getDocumentation(indentStr + "# ", structObj, 80);
            ss << indentStr << "class I" << GenerateStructCx::fixStructName(structObj) << "(object):\n";
          }

          String fixedTypeStr = GenerateStructC::fixType(structObj);
          String cTypeStr = GenerateStructC::fixCType(structObj);
          String csTypeStr = GenerateStructCx::fixStructName(structObj);

          {
            auto &ss = structFile.structSS_;
            ss << GenerateHelper::getDocumentation(indentStr + "# ", structObj, 80);
            ss << indentStr << "class " << csTypeStr << "(";

            bool first = true;
            if (structFile.shouldInheritException_) {
              ss << "Exception";
              first = false;
            }

            if (!structFile.isStaticOnly_) {
              if (!first) ss << ", ";
              ss << "wrapper.IDisposable";
              first = false;

              processInheritance(apiFile, structFile, interfaceFile, structObj, structObj, first);
            }
            ss << "):\n";
          }

          apiFile.startRegion(fixPythonPathType(structObj));

          structFile.indentMore();

          if (!structFile.isStaticOnly_)
          {
            //structFile.usingTypedef(structObj);
            structFile.usingTypedef("instance_id_t", "c_void_p");
            structFile.usingTypedef(cTypeStr, "c_void_p");

            {
              auto &ss = structFile.structSS_;
              structFile.startRegion("To / From C routines");
              ss << indentStr << "class __WrapperMakePrivate:\n";
              ss << indentStr << "    pass\n";
              ss << indentStr << "\n";
              ss << indentStr << "def __init__(self, ignored: __WrapperMakePrivate, handle: " << cTypeStr << "):\n";
              ss << indentStr << "    self.__disposed = False\n";
              ss << indentStr << "    self.__native = handle\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    self.__wrapperEventObserver = None\n";
                ss << indentStr << "    self.__wrapperObserverInstallEventHandlers()\n";
                ss << indentStr << "    self.__wrapperObserveEvents()\n";
              }
              ss << indentStr << "\n";
              ss << indentStr << "def dispose(self):\n";
              ss << indentStr << "    self.doDispose(True)\n";
              ss << indentStr << "\n";
              ss << indentStr << "def doDispose(self, disposing: bool):\n";
              ss << indentStr << "    if (self.__disposed == True):\n";
              ss << indentStr << "        return\n";
              ss << indentStr << "    if (self.__native is None):\n";
              ss << indentStr << "        return\n";
              if (structFile.hasEvents_) {
                ss << indentStr << "    self.__wrapperObserveEventsCancel()\n";
              }
              ss << indentStr << "    " << getApiPath() << "." << fixedTypeStr << "_wrapperDestroy(self.__native)\n";
              ss << indentStr << "    self.__native = None\n";
              ss << indentStr << "\n";
              ss << indentStr << "def __del__(self):\n";
              ss << indentStr << "    self.doDispose(False)\n";
              ss << indentStr << "\n";
              ss << indentStr << "def internalGetNativeHandle(self):\n";
              ss << indentStr << "    return self.__native\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << fixedTypeStr << "_FromC(handle: " << cTypeStr << "): # -> " << csTypeStr << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << csTypeStr << "(None, " << getApiPath() << "." << fixedTypeStr << "_wrapperClone(handle))\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "def " << fixedTypeStr << "_AdoptFromC(handle: " << cTypeStr << "): # -> " << csTypeStr << ":\n";
              ss << indentStr << "    if (handle is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << csTypeStr << "(None, handle)\n";
              ss << indentStr << "\n";
              ss << indentStr << "@staticmethod\n";
              ss << indentStr << "# def " << fixedTypeStr << "_ToC(value: " << csTypeStr << ") -> " << cTypeStr << ":\n";
              ss << indentStr << "def " << fixedTypeStr << "_ToC(value) -> " << cTypeStr << ":\n";
              ss << indentStr << "    if (value is None):\n";
              ss << indentStr << "        return None\n";
              ss << indentStr << "    return " << getApiPath() << "." << fixedTypeStr << "_wrapperClone(value.internalGetNativeHandle())\n";
              structFile.endRegion("To / From C routines");
            }
            {
              apiFile.usingTypedef(structObj);
              apiFile.usingTypedef("instance_id_t", "c_void_p");

              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.structSS_;
              ss << indentApiStr << "\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperClone = __lib." << fixedTypeStr << "_wrapperClone\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperClone.restype = " << cTypeStr << "\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperClone.argtypes = [" << cTypeStr << "]\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "# def " << fixedTypeStr << "_wrapperClone(handle: " << cTypeStr << ") -> " << cTypeStr << ":\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_wrapperClone(handle) -> " << cTypeStr << ":\n";
              ss << indentApiStr << "    return Api.__" << fixedTypeStr << "_wrapperClone(handle)\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperDestroy = __lib." << fixedTypeStr << "_wrapperDestroy\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperDestroy.restype = None\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperDestroy.argtypes = [" << cTypeStr << "]\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_wrapperDestroy(handle: " << cTypeStr << "):\n";
              ss << indentApiStr << "    Api.__" << fixedTypeStr << "_wrapperDestroy(handle)\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperInstanceId = __lib." << fixedTypeStr << "_wrapperInstanceId\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperInstanceId.restype = instance_id_t\n";
              ss << indentApiStr << "__" << fixedTypeStr << "_wrapperInstanceId.argtypes = [ " << cTypeStr << " ]\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_wrapperInstanceId(handle: " << cTypeStr << ") -> instance_id_t:\n";
              ss << indentApiStr << "    return Api.__" << fixedTypeStr << "_wrapperInstanceId(handle)\n";
              if (structFile.hasEvents_) {
                apiFile.usingTypedef("event_observer_t", "c_void_p");
                ss << indentApiStr << "\n";
                ss << indentApiStr << "__" << fixedTypeStr << "_wrapperObserveEvents = __lib." << fixedTypeStr << "_wrapperObserveEvents\n";
                ss << indentApiStr << "__" << fixedTypeStr << "_wrapperObserveEvents.restype = event_observer_t\n";
                ss << indentApiStr << "__" << fixedTypeStr << "_wrapperObserveEvents.argtypes = [" << cTypeStr << "]\n";
                ss << indentApiStr << "\n";
                ss << indentApiStr << "@staticmethod\n";
                ss << indentApiStr << "def " << fixedTypeStr << "_wrapperObserveEvents(handle: " << cTypeStr << ") -> event_observer_t:\n";
                ss << indentApiStr << "    return Api.__" << fixedTypeStr << "_wrapperObserveEvents(handle)\n";
              }
            }
            {
              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.helpersSS_;
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_FromC(handle: " << cTypeStr << "): # -> " << fixPythonPathType(structObj) << ":\n";
              ss << indentApiStr << "    from " << fixPythonPathType(structObj) << " import " << fixPythonType(structObj) << " as " << fixedTypeStr << "\n";
              ss << indentApiStr << "    return " << fixedTypeStr << "." << fixedTypeStr << "_FromC(handle)\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_AdoptFromC(handle: " << cTypeStr << "): # -> " << fixPythonPathType(structObj) <<  ":\n";
              ss << indentApiStr << "    from " << fixPythonPathType(structObj) << " import " << fixPythonType(structObj) << " as " << fixedTypeStr << "\n";
              ss << indentApiStr << "    return " << fixedTypeStr << "." << fixedTypeStr << "_AdoptFromC(handle)\n";
              ss << indentApiStr << "\n";
              ss << indentApiStr << "@staticmethod\n";
              ss << indentApiStr << "# def " << fixedTypeStr << "_ToC(value: " << fixPythonPathType(structObj) << ") -> " << cTypeStr << ":\n";
              ss << indentApiStr << "def " << fixedTypeStr << "_ToC(value) -> " << cTypeStr << ":\n";
              ss << indentApiStr << "    from " << fixPythonPathType(structObj) << " import " << fixPythonType(structObj) << " as " << fixedTypeStr << "\n";
              ss << indentApiStr << "    return " << fixedTypeStr << "." << fixedTypeStr << "_ToC(value)\n";
              ss << indentApiStr << "\n";
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
                  ss << indentApiStr << "\n";
                  ss << indentApiStr << "__" << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << " = __lib." << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "\n";
                  ss << indentApiStr << "__" << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << ".restype = " << GenerateStructC::fixCType(structObj) << "\n";
                  ss << indentApiStr << "__" << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << ".argtypes = [ " << GenerateStructC::fixCType(structObj) << " ]\n";
                  ss << indentApiStr << "\n";
                  ss << indentApiStr << "@staticmethod\n";
                  ss << indentApiStr << "def " << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "(handle: " << GenerateStructC::fixCType(structObj) << ") -> " << GenerateStructC::fixCType(relatedStruct) << ":\n";
                  ss << indentApiStr << "    return Api.__" << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "(handle)\n";
                }
                {
                  auto &indentStructStr = structFile.indent_;
                  auto &ss = relStructSS;
                  ss << indentStructStr << "\n";
                  ss << indentStructStr << "@staticmethod\n";
                  ss << indentStructStr << "# def cast_" << GenerateStructC::fixType(relatedStruct) << "(value: " << fixPythonPathType(relatedStruct) << " ): -> " << fixPythonType(structObj) << ":\n";
                  ss << indentStructStr << "def cast_" << GenerateStructC::fixType(relatedStruct) << "(value):\n";
                  ss << indentStructStr << "    if (value is None):\n";
                  ss << indentStructStr << "        return None\n";
                  ss << indentStructStr << "    castHandle = " << getToCMethod(apiFile, false, relatedStruct) << "(value)\n";
                  ss << indentStructStr << "    if (castHandle is None):\n";
                  ss << indentStructStr << "        return None\n";
                  ss << indentStructStr << "    originalHandle = " << getApiPath() << "." << GenerateStructC::fixType(structObj) << "_wrapperCastAs_" << GenerateStructC::fixType(relatedStruct) << "(castHandle)\n";
                  ss << indentStructStr << "    if (originalHandle is None):\n";
                  ss << indentStructStr << "        return None\n";
                  ss << indentStructStr << "    return " << getAdoptFromCMethod(apiFile, false, structObj) << "(originalHandle)\n";
                }
              }

              if (foundRelated) {
                structFile.startRegion("Casting methods");
                structFile.structSS_ << relStructSS.str();
                structFile.endRegion("Casting methods");
              }
            }
          }

          processStruct(pathStr, apiFile, initFile, structFile, interfaceFile, structObj, structObj);

          apiFile.endRegion(fixPythonPathType(structObj));


          if (interfaceFile.shouldDefineInterface_) {
            auto &ss = interfaceFile.interfaceEndSS_;
            ss << indentStr << "pass\n\n";
          }
          {
            auto &ss = structFile.structEndSS_;
            ss << indentStr << "pass\n\n\n";
          }

          structFile.indentLess();

          finalizeBaseFile(structFile);
          if (interfaceFile.shouldDefineInterface_) {
            finalizeBaseFile(interfaceFile);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processInheritance(
                                                      ApiFile &apiFile,
                                                      StructFile &structFile,
                                                      StructFile &interfaceFile,
                                                      StructPtr rootStructObj,
                                                      StructPtr structObj,
                                                      bool &first
                                                      )
        {
          if (!structObj) return;

          bool includeInterface = hasInterface(structObj);

          String interfaceNamePath = fixPythonPathType(structObj, includeInterface);
          String interfaceName = (includeInterface ? "I" : "") + GenerateStructCx::fixStructName(structObj);
          String flattenedInterfaceNamePath = flattenPath(interfaceNamePath);

          bool addTypedef {true};

          if (!includeInterface) {
            addTypedef = (rootStructObj != structObj);
          }
          if (addTypedef) {
            structFile.usingTypedef(interfaceNamePath, interfaceName, flattenedInterfaceNamePath);
          }

          if (hasInterface(structObj)) {
            auto &ss = structFile.structSS_;
            if (!first) ss << ", ";
            first = false;
            ss << flattenedInterfaceNamePath;
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter)
          {
            auto relatedObj = (*iter).second;
            if (!relatedObj) continue;
            processInheritance(apiFile, structFile, interfaceFile, rootStructObj, relatedObj->toStruct(), first);
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processStruct(
                                                 const String &pathStr,
                                                 ApiFile &apiFile,
                                                 InitFile &initFile,
                                                 StructFile &structFile,
                                                 StructFile &interfaceFile,
                                                 StructPtr rootStructObj,
                                                 StructPtr structObj
                                                 )
        {
          if (!structObj) return;

          if (rootStructObj == structObj) {
            for (auto iter = structObj->mStructs.begin(); iter != structObj->mStructs.end(); ++iter) {
              auto subStructObj = (*iter).second;
              processStruct(pathStr, apiFile, initFile, subStructObj);
            }
            for (auto iter = structObj->mEnums.begin(); iter != structObj->mEnums.end(); ++iter) {
              auto enumObj = (*iter).second;
              processEnum(apiFile, structFile, structObj, enumObj);
            }
          }

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter) {
            auto relatedTypeObj = (*iter).second;
            if (!relatedTypeObj) continue;
            processStruct(pathStr, apiFile, initFile, structFile, interfaceFile, rootStructObj, relatedTypeObj->toStruct());
          }

          structFile.startRegion(fixPythonPathType(structObj));

          processMethods(apiFile, structFile, interfaceFile, rootStructObj, structObj);
          processProperties(apiFile, structFile, interfaceFile, rootStructObj, structObj);
          if (rootStructObj == structObj) {
            processEventHandlers(apiFile, structFile, interfaceFile, structObj);
          }

          structFile.endRegion(fixPythonPathType(structObj));
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processEnum(
                                               ApiFile &apiFile,
                                               StructFile &structFile,
                                               StructPtr structObj,
                                               EnumTypePtr enumObj
                                               )
        {
          if (!enumObj) return;

          auto &indentStr = structFile.indent_;
          auto &ss = structFile.structSS_;

          ss << indentStr << "\n";
          ss << indentStr << "class " << fixName(enumObj->getMappingName()) << "(Enum)\n";
          //if (enumObj->mBaseType != PredefinedTypedef_int) {
          //  ss << " : " << fixCPythonType(enumObj->mBaseType);
          //}
          structFile.indentMore();

          bool first = true;
          for (auto iter = enumObj->mValues.begin(); iter != enumObj->mValues.end(); ++iter) {
            auto valueObj = (*iter);

            ss << indentStr << fixName(valueObj->getMappingName());
            if (valueObj->mValue.hasData()) {
              ss << " = " << valueObj->mValue;
            } else {
              if (!first) {
                ss << " = auto()";
              } else {
                ss << " = 0";
              }
            }
            first = false;
            ss << "\n";
          }

          structFile.indentLess();
          ss << indentStr << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processMethods(
                                                  ApiFile &apiFile,
                                                  StructFile &structFile,
                                                  StructFile &interfaceFile,
                                                  StructPtr rootStructObj,
                                                  StructPtr structObj
                                                  )
        {
          auto &indentStr = structFile.indent_;
          String structName = GenerateStructCx::fixStructName(structObj);

          bool foundDllMethod {};

          if (rootStructObj == structObj) {
            if (GenerateHelper::needsDefaultConstructor(rootStructObj)) {
              {
                auto &indentApiStr = apiFile.indent_;
                auto &&ss = apiFile.structSS_;
                ss << indentApiStr << "\n";
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << " = __lib." << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << "\n";
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << ".restype = " << GenerateStructC::fixCType(structObj) << "\n";
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << ".argtypes = None\n";
                ss << indentApiStr << "\n";
                ss << indentApiStr << "@staticmethod\n";
                apiFile.usingTypedef(rootStructObj);
                ss << indentApiStr << "def " << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << "() -> " << GenerateStructC::fixCType(structObj) << ":\n";
                ss << indentApiStr << "    return Api.__" << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << rootStructObj->getMappingName() << "()\n";
                ss << indentApiStr << "\n";
              }
              {
                auto &ss = structFile.structSS_;
                ss << indentStr << "\n";

                String altNameStr = fixName(rootStructObj->getMappingName());

                ss << indentStr << "def " << altNameStr << "(self):\n";

                structFile.indentMore();

                structFile.usingTypedef(structObj);
                ss << indentStr << "self.__native = " << getApiPath() << "." << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << altNameStr << "()\n";

                if (structFile.hasEvents_) {
                  ss << indentStr << "self.__wrapperObserveEvents()\n";
                }

                structFile.indentLess();
                ss << indentStr << "\n";
              }
            }
          }

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (!method) continue;
            if (method->hasModifier(Modifier_Method_EventHandler)) continue;

            bool isConstructor = method->hasModifier(Modifier_Method_Ctor);
            bool isStatic = method->hasModifier(Modifier_Static);

            bool generateInterface = (!isStatic) && (!isConstructor) && (rootStructObj == structObj) && (interfaceFile.shouldDefineInterface_);

            if (rootStructObj != structObj) {
              if ((isStatic) || (isConstructor)) continue;
            }
            if (method->hasModifier(Modifier_Method_Delete)) continue;

            auto resultCsStr = fixPythonPathType(method->hasModifier(Modifier_Optional), method->mResult);
            bool hasResult = ("void" != resultCsStr) && ("NoneType" != resultCsStr) && (resultCsStr.hasData()) && (!isConstructor);

            String altNameStr = method->getModifierValue(Modifier_AltName);
            if (altNameStr.isEmpty()) {
              altNameStr = method->getMappingName();
            }

            {
              auto &indentApiStr = apiFile.indent_;
              auto &&ss = apiFile.structSS_;

              String cMethodName = GenerateStructC::fixType(rootStructObj);
              if (isConstructor) {
                cMethodName += "_wrapperCreate_";
              } else {
                cMethodName += "_";
              }
              cMethodName += altNameStr;

              if (!foundDllMethod) {
                if (rootStructObj != structObj) {
                  ss << indentApiStr << "\n";
                  ss << indentApiStr << "#if !" << getApiCastRequiredDefine(apiFile) << ":\n";
                }
                foundDllMethod = true;
              }
              ss << indentApiStr << "\n";
              ss << indentApiStr << "__" << cMethodName << " = __lib." << cMethodName << "\n";
              ss << indentApiStr << "__" << cMethodName << ".restype = ";
              if (isConstructor) {
                ss << GenerateStructC::fixCType(structObj);
              } else {
                if (hasResult) {
                  ss << "" << GenerateStructC::fixCType(method->hasModifier(Modifier_Optional), method->mResult) << "";
                } else {
                  ss << "None";
                }
              }
              ss << "\n";
              ss << indentApiStr << "__" << cMethodName << ".argtypes = [";
              bool first {true};
              if (method->mThrows.size() > 0) {
                ss << "exception_handle_t";
                first = false;
              }
              if ((!isConstructor) &&
                  (!isStatic)) {
                if (!first) ss << ", ";
                first = false;
                ss << GenerateStructC::fixCType(rootStructObj);
              }

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << GenerateStructC::fixCType(arg->hasModifier(Modifier_Optional), arg->mType);
              }

              ss << "]\n";
              ss << indentApiStr << "\n";

              ss << indentApiStr << "@staticmethod\n";
              if (isConstructor) {
                apiFile.usingTypedef(structObj);
              } else {
                apiFile.usingTypedef(method->mResult);
              }
              ss << indentApiStr << "def " << cMethodName << "(";
              first  = true;
              if (method->mThrows.size() > 0) {
                apiFile.usingTypedef("exception_handle_t", "c_void_p");
                ss << "wrapperExceptionHandle: exception_handle_t";
                first = false;
              }
              if ((!isConstructor) &&
                  (!isStatic)) {
                if (!first) ss << ", ";
                first = false;
                ss << "thisHandle: " << GenerateStructC::fixCType(rootStructObj) << "";
              }
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                apiFile.usingTypedef(arg->mType);
                ss << fixArgumentName(arg->getMappingName()) << ": " << GenerateStructC::fixCType(arg->hasModifier(Modifier_Optional), arg->mType);
              }
              ss << ")";
              if (isConstructor) {
                ss << ": #  -> " << GenerateStructC::fixCType(structObj) << ":\n";
              } else {
                if (hasResult) {
                  ss << ": # -> ";
                  if (method->hasModifier(Modifier_Optional)) {
                    ss << " ";
                  }
                  ss << "" << GenerateStructC::fixCType(method->hasModifier(Modifier_Optional), method->mResult) << "";
                  if (method->hasModifier(Modifier_Optional)) {
                    ss << "";
                  }
                }
                ss << ":\n";
              }
              ss << indentApiStr << "    ";
              if ((isConstructor) || (hasResult)) {
                ss << "return ";
              }
              ss << "Api.__" << cMethodName << "(";

              first = true;
              if (method->mThrows.size() > 0) {
                ss << "wrapperExceptionHandle";
                first = false;
              }
              if ((!isConstructor) &&
                  (!isStatic)) {
                if (!first) ss << ", ";
                first = false;
                ss << "thisHandle";
              }
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << fixArgumentName(arg->getMappingName());
              }
              ss << ")\n";

              ss << indentApiStr << "\n";
            }

            for (int loop = 0; loop < 2; ++loop)
            {
              auto &iSS = interfaceFile.interfaceSS_;
              auto &sSS = structFile.structSS_;

              String altNameStr = method->getModifierValue(Modifier_AltName);
              if (altNameStr.isEmpty()) {
                altNameStr = method->getMappingName();
              }

              if (isConstructor) {
                altNameStr.replaceAll(structName, "create");
              }

              if (generateInterface) {
                if (0 == loop) {
                  iSS << indentStr << "\n";
                  iSS << GenerateHelper::getDocumentation(indentStr + "# ", method, 80);
                }
                iSS << indentStr << (0 == loop ? "# ": "") << "def " << altNameStr << "(";
              }

              if (0 == loop) {
                sSS << indentStr << "\n";
                sSS << GenerateHelper::getDocumentation(indentStr + "# ", method, 80);
                if (isStatic) {
                  sSS << indentStr << "@staticmethod\n";
                }
              }
              sSS << indentStr  << (0 == loop ? "# ": "") << "def " << (isConstructor ? altNameStr : altNameStr) << "(";

              bool first {true};

              if ((!isStatic) && 
                  (!isConstructor)) {
                if (generateInterface) {
                  iSS << "self";
                }
                sSS << "self";
                first = false;
              }

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) {
                  if (generateInterface) {
                    iSS << ", ";
                  }
                  sSS << ", ";
                }
                first = false;
                if (generateInterface) {
                  iSS << fixArgumentName(arg->getMappingName());
                  if (0 == loop) {
                    iSS << ": " << fixPythonPathType(arg->hasModifier(Modifier_Optional), arg->mType);
                  }
                }
                sSS << fixArgumentName(arg->getMappingName());
                if (0 == loop) {
                  sSS << ": " << fixPythonPathType(arg->hasModifier(Modifier_Optional), arg->mType);
                }
              }

              if (generateInterface) {
                iSS << ")";
                if (hasResult) {
                  if (0 == loop) {
                    iSS << " -> " << resultCsStr;
                  }
                }
                iSS << ":\n";
                if (0 != loop) {
                  iSS << indentStr << "    pass\n";
                  iSS << indentStr << "\n";
                }
              }
              sSS << ")";
              if (hasResult) {
                if (0 == loop) {
                  sSS << ": -> " << resultCsStr;
                }
              }
              sSS << ":\n";
            }
            {
              auto &ss = structFile.structSS_;
              structFile.indentMore();

              if (rootStructObj != structObj) {
                ss << indentStr << "if (\'" << getApiCastRequiredDefine(apiFile) << "\' in globals()):\n";

                structFile.indentMore();

                ss << indentStr << "cast = " << fixPythonPathType(structObj) << ".cast_" << GenerateStructC::fixType(structObj) <<"(self)\n";
                ss << indentStr << "if (cast is None):\n";
                ss << indentStr << "    raise ReferenceError(\"self \\\"" << fixPythonPathType(structObj) << "\\\" casted from \\\"" << fixPythonPathType(rootStructObj) << "\\\" becomes None.\")\n";
                ss << indentStr << (hasResult ? "return " : "" ) << "cast." << method->getMappingName() << "(";
                bool first {true};
                for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                  auto arg = (*iterArgs);
                  if (!arg) continue;
                  if (!first) ss << ", ";
                  first = false;
                  ss << fixArgumentName(arg->getMappingName());
                }
                ss << ")\n";

                structFile.indentLess();
                ss << indentStr << "else:\n";
                structFile.indentMore();
              }

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                structFile.usingTypedef(arg->mType);
                ss << indentStr << "wrapper_c_" << arg->getMappingName() << " = " << getToCMethod(apiFile, arg->hasModifier(Modifier_Optional), arg->mType) << "(" << fixArgumentName(arg->getMappingName()) << ")\n";
              }
              if (method->mThrows.size() > 0) {
                structFile.usingTypedef("exception_handle_t", "c_void_p");
                ss << indentStr << "wrapperException = " << getApiPath() << ".exception_wrapperCreate_exception()\n";
              }
              if (hasResult) {
                structFile.usingTypedef(method->mResult);
                ss << indentStr << "wrapper_c_wrapper_result = ";
              }

              if (isConstructor) {
                structFile.usingTypedef(structObj);
                ss << indentStr << "self = " << structName << "(None, None)\n";
                ss << indentStr << "self.__native = ";
              }

              if ((!hasResult) &&
                  (!isConstructor)) {
                ss << indentStr;
              }

              if (isConstructor) {
                ss << getApiPath() << "." << GenerateStructC::fixType(rootStructObj) << "_wrapperCreate_" << altNameStr << "(";
              } else {
                ss << getApiPath() << "." << GenerateStructC::fixType(rootStructObj) << "_" << altNameStr << "(";
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
                ss << "self.__native";
              }
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << "wrapper_c_" << arg->getMappingName();
              }
              ss << ")\n";

              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs)
              {
                auto arg = (*iterArgs);
                if (!arg) continue;
                auto wrapperDestroyStr = getDestroyCMethod(apiFile, arg->hasModifier(Modifier_Optional), arg->mType);
                if (wrapperDestroyStr.hasData()) {
                  ss << indentStr << wrapperDestroyStr << "(wrapper_c_" << arg->getMappingName() << ")\n";
                }
              }

              if (method->mThrows.size() > 0) {
                ss << indentStr << "wrapperPyException = " << getHelperPath() << ".exception_AdoptFromC(wrapperException)\n";
                ss << indentStr << "if (wrapperPyException is not None):\n";
                if (hasResult) {
                  auto destroyStr = getDestroyCMethod(apiFile, method->hasModifier(Modifier_Optional), method->mResult);
                  if (destroyStr.hasData()) {
                    ss << indentStr << "    " << destroyStr << "(wrapper_c_wrapper_result)\n";
                  }
                }
                ss << indentStr << "    raise wrapperPyException\n";
              }

              if (hasResult) {
                ss << indentStr << "return " << getAdoptFromCMethod(apiFile, method->hasModifier(Modifier_Optional), method->mResult) << "(wrapper_c_wrapper_result)\n";
              }
              if (isConstructor) {
                if (structFile.hasEvents_) {
                  ss << indentStr << "self.__wrapperObserveEvents()\n";
                }
                ss << indentStr << "return self";
              }

              if (rootStructObj != structObj) {
                structFile.indentLess();
              }

              structFile.indentLess();
              ss << indentStr << "\n";
            }
          }

          if (foundDllMethod) {
            if (rootStructObj != structObj) {
              auto &&ss = apiFile.structSS_;
              ss << indentStr << "\n";
              ss << indentStr << "#endif // !" << getApiCastRequiredDefine(apiFile) << "\n";
              ss << indentStr << "\n";
            }
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processProperties(
                                                     ApiFile &apiFile,
                                                     StructFile &structFile,
                                                     StructFile &interfaceFile,
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
              auto &ss = interfaceFile.interfaceSS_;
              ss << GenerateHelper::getDocumentation(indentStr + "# ", propertyObj, 80);
              ss << indentStr << propertyObj->getMappingName() << " = property(None)\n";
              ss << indentStr << "\n";
            }

            {
              auto &ss = structFile.structSS_;
              ss << GenerateHelper::getDocumentation(indentStr + "# ", propertyObj, 80);

              ss << indentStr << "\n";

              if (rootStructObj != structObj) {
                ss << indentStr << "if (\'" << getApiCastRequiredDefine(apiFile) << "\' in globals()):\n";
                structFile.indentMore();
                if (hasGet) {
                  if (isStatic) {
                    ss << indentStr << "@staticmethod\n";
                  }
                  for (int loop = 0; loop < 2; ++loop) {
                    ss << indentStr << (loop == 0 ? "# " : "") << "def __get_" << propertyObj->getMappingName() << "(";
                    if (!isStatic) {
                      ss << "self";
                    }
                    if (0 == loop) {
                      ss << "): -> " << fixPythonPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << ":\n";
                    } else {
                      ss << "):\n";
                    }
                  }
                  if (!isStatic) {
                    ss << indentStr << "    cast = " << fixPythonPathType(structObj) << ".cast_" << GenerateStructC::fixType(structObj) << "(self)\n";
                    ss << indentStr << "    if (cast is None):\n";
                    ss << indentStr << "        raise ReferenceError(\"self \\\"" << fixPythonPathType(structObj) << "\\\" casted from \\\"" << fixPythonPathType(rootStructObj) << "\\\" becomes null.\")\n";
                    ss << indentStr << "    return cast.";
                  } else {
                    ss << indentStr << "    return " << fixPythonPathType(structObj) << ".";
                  }
                  ss << fixName(propertyObj->getMappingName()) << "\n";
                  ss << indentStr << "\n";
                }
                if (hasSet) {
                  if (isStatic) {
                    ss << indentStr << "@staticmethod\n";
                  }
                  for (int loop = 0; loop < 2; ++loop) {
                    ss << indentStr << (0 == loop ? "# " : "") << "def __set_" << propertyObj->getMappingName() << "(";
                    if (!isStatic) {
                      ss << "self, ";
                    }
                    if (0 == loop) {
                      ss << "value: " << fixPythonPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "):\n";
                    } else {
                      ss << "value):\n";
                    }
                  }
                  if (!isStatic) {
                    ss << indentStr << "    cast = " << fixPythonPathType(structObj) << ".cast_" << GenerateStructC::fixType(structObj) << "(self)\n";
                    ss << indentStr << "    if (case is None):\n";
                    ss << indentStr << "        raise ReferenceError(\"self \\\"" << fixPythonPathType(structObj) << "\\\" casted from \\\"" << fixPythonPathType(rootStructObj) << "\\\" becomes null.\")\n";
                    ss << indentStr << "    cast.";
                  } else {
                    ss << indentStr << "    " << fixPythonPathType(structObj) << ".";
                  }
                  ss << fixName(propertyObj->getMappingName()) << " = value\n";
                  ss << indentStr << "\n";
                }
                structFile.indentLess();
                ss << indentStr << "else:\n";
                structFile.indentMore();
              }

              if (hasGet) {
                if (isStatic) {
                  ss << indentStr << "@staticmethod\n";
                }
                for (int loop = 0; loop < 2; ++loop) {
                  ss << indentStr << (0 == loop ? "# " : "") << "def __get_" << propertyObj->getMappingName() << "(";
                  if (!isStatic) {
                    ss << "self";
                  }
                  if (0 == loop) {
                    ss << "): -> " << fixPythonPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << ":\n";
                  } else {
                    ss << "):\n";
                  }
                }
                ss << indentStr << "    result = " << getApiPath() << "." << GenerateStructC::fixType(structObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "self.__native") << ")\n";
                ss << indentStr << "    return " << getAdoptFromCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(result)\n";
                ss << indentStr << "\n";
              }
              if (hasSet) {
                if (isStatic) {
                  ss << indentStr << "@staticmethod\n";
                }
                for (int loop = 0; loop < 2; ++loop) {
                  ss << indentStr << (0 == loop ? "# " : "") << "def __set_" << propertyObj->getMappingName() << "(";
                  if (!isStatic) {
                    ss << "self, ";
                  }
                  if (0 == loop) {
                    ss << "value: " << fixPythonPathType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "):\n";
                  } else {
                    ss << "value):\n";
                  }
                }
                ss << indentStr << "    cValue = " << getToCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(value)\n";
                ss << indentStr << "    " << getApiPath() << "." << GenerateStructC::fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "self.__native, ") << "cValue)\n";
                auto destroyRoutine = getDestroyCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType);
                if (destroyRoutine.hasData()) {
                  ss << indentStr << "    " << getDestroyCMethod(apiFile, propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "(cValue)\n";
                }
                ss << indentStr << "\n";
              }

              if (rootStructObj != structObj) {
                structFile.indentLess();
              }

              if (hasGet || hasSet) {
                ss << indentStr << propertyObj->getMappingName() << " = property(";
                if (hasGet) {
                  ss << "__get_" << propertyObj->getMappingName();
                } else {
                  ss << "None";
                }
                ss << ", ";
                if (hasSet) {
                  ss << "__set_" << propertyObj->getMappingName();
                } else {
                  ss << "None";
                }
                ss << ")\n";
                ss << indentStr << "\n";
              }

              ss << indentStr << "\n";
            }

            {
              auto &indentApiStr = apiFile.indent_;
              auto &ss = apiFile.structSS_;
              if (rootStructObj != structObj) {
                if (!foundCastRequiredDll) {
                  ss << indentApiStr << "#if !" << getApiCastRequiredDefine(apiFile) << "\n";
                  foundCastRequiredDll = true;
                }
              }
              if (hasGet) {
                ss << indentApiStr << "\n";
                //if (!propertyObj->hasModifier(Modifier_Optional)) {
                //  ss << getReturnMarshal(propertyObj->mType, indentApiStr);
                //}
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << " = __lib." << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "\n";
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << ".restype = " << GenerateStructC::fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "\n";
                ss << indentApiStr << "__" << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << ".argtypes = [" << (isStatic ? String() : String(GenerateStructC::fixCType(rootStructObj))) << "]\n";
                ss << indentApiStr << "\n";
                ss << indentApiStr << "@staticmethod\n";
                ss << indentApiStr << "def " << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? String() : "thisHandle: " + String(GenerateStructC::fixCType(rootStructObj))) << ") -> " << GenerateStructC::fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << ":\n";
                ss << indentApiStr << "    return Api.__" << GenerateStructC::fixType(rootStructObj) << "_get_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "thisHandle") << ")\n";
              }
              if (hasSet) {
                ss << indentApiStr << "\n";
                ss << indentApiStr << "\n";
                ss << indentApiStr << "\n";
                ss << indentApiStr << "@staticmethod\n";
                ss << indentApiStr << "def " << GenerateStructC::fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? String() : "thisHandle: " + String(GenerateStructC::fixCType(rootStructObj) + ", ")) << "value: " << (propertyObj->hasModifier(Modifier_Optional) ? String() : String()) << GenerateStructC::fixCType(propertyObj->hasModifier(Modifier_Optional), propertyObj->mType) << "):\n";
                ss << indentApiStr << "    return Api.__" << GenerateStructC::fixType(rootStructObj) << "_set_" << propertyObj->getMappingName() << "(" << (isStatic ? "" : "thisHandle, ") << "value)\n";
              }
            }

          }
          if (foundCastRequiredDll) {
            auto &ss = apiFile.structSS_;
            ss << apiFile.indent_ << "#endif // !" << getApiCastRequiredDefine(apiFile) << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processEventHandlers(
                                                        ApiFile &apiFile,
                                                        StructFile &structFile,
                                                        StructFile &interfaceFile,
                                                        StructPtr structObj
                                                        )
        {
          auto &indentStr = structFile.indent_;

          if (!structObj) return;
          if (!structFile.hasEvents_) return;

          structFile.usingTypedef("callback_event_t", "c_void_p");

          processEventHandlersStart(apiFile, structFile, interfaceFile, structObj);

          auto &ss = structFile.structSS_;

          ss << indentStr << "def __wrapperObserverInstallEventHandlers(self):\n";

          structFile.indentMore();

          ss << indentStr << "\n";
          ss << indentStr << "def __noop(farg, *args):\n";
          ss << indentStr << "    pass\n";
          ss << indentStr << "\n";

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            if (!method->hasModifier(Modifier_Method_EventHandler)) continue;

            {
              ss << GenerateHelper::getDocumentation(indentStr + "# ", method, 80);
              ss << indentStr << "self." << method->getMappingName() << " = __noop\n";
              ss << indentStr << "\n";
            }

            {
              ss << indentStr << "# delegate def " << fixPythonType(structObj) << "_" << fixName(method->getMappingName()) << "(";
              bool first {true};
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << fixArgumentName(arg->getMappingName()) << ": " << fixPythonPathType(arg->mType);
              }
              ss << ")\n";
              ss << indentStr << "\n";
            }

          }

          structFile.indentLess();

          processEventHandlersEnd(apiFile, structFile, interfaceFile, structObj);
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processEventHandlersStart(
                                                             ApiFile &apiFile,
                                                             StructFile &structFile,
                                                             StructFile &interfaceFile,
                                                             StructPtr structObj
                                                             )
        {
          auto &indentStr = structFile.indent_;

          structFile.startRegion("Events");

          structFile.usingTypedef("event_observer_t", "c_void_p");

          {
            auto &ss = structFile.structSS_;
            ss << indentStr << "\n";
            ss << indentStr << "def __wrapperObserveEvents(self):\n";
            ss << indentStr << "    if (self.__native is None):\n";
            ss << indentStr << "        return\n";
            ss << indentStr << "    \n";
            ss << indentStr << "    def __eventCallback(target, method: str, handle: callback_event_t):\n";
            structFile.indentMore();
            structFile.indentMore();
            ss << indentStr << "if (target is None):\n";
            ss << indentStr << "    return\n";
            ss << indentStr << "\n";

            for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
              auto method = (*iter);
              if (!method) continue;
              if (!method->hasModifier(Modifier_Method_EventHandler)) continue;
              
              ss << indentStr << "if (\"" << method->getMappingName() << "\" == method):\n";
              ss << indentStr << "    target." << method->getMappingName() << "(";
              bool first {true};
              size_t index = 0;
              for (auto iterArgs = method->mArguments.begin(); iterArgs != method->mArguments.end(); ++iterArgs, ++index) {
                auto arg = (*iterArgs);
                if (!arg) continue;
                if (!first) ss << ", ";
                first = false;
                ss << getAdoptFromCMethod(apiFile, method->hasModifier(Modifier_Optional), arg->mType) << "(" << getApiPath() << ".callback_event_get_data(handle, " << index << "))";
              }
              ss << ")\n";
            }
            structFile.indentLess();
            structFile.indentLess();
            ss << indentStr << "    \n";
            ss << indentStr << "    " << getHelperPath() << ".observeEvents(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath() << "." << GenerateStructC::fixType(structObj) << "_wrapperInstanceId(self.__native), self, __eventCallback)\n";

            ss << indentStr << "    if (self.__wrapperEventObserver is None):\n";
            ss << indentStr << "        self.__wrapperEventObserver = " << getApiPath() << "." << GenerateStructC::fixType(structObj) << "_wrapperObserveEvents(self.__native)\n";
            ss << indentStr << "\n";

            ss << indentStr << "def __wrapperObserveEventsCancel(self):\n";
            ss << indentStr << "    if (self.__wrapperEventObserver is not None):\n";
            ss << indentStr << "        " << getApiPath() << ".callback_wrapperObserverDestroy(self.__wrapperEventObserver)\n";
            ss << indentStr << "        self.__wrapperEventObserver = None\n";
            ss << indentStr << "    \n";
            ss << indentStr << "    " << getHelperPath() << ".observeEventsCancel(\"" << structObj->getPath() << "\", \"" << structObj->getMappingName() << "\", " << getApiPath() << "." << GenerateStructC::fixType(structObj) << "_wrapperInstanceId(self.__native), self)\n";
            ss << indentStr << "\n";
          }
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::processEventHandlersEnd(
                                                           ApiFile &apiFile,
                                                           StructFile &structFile,
                                                           StructFile &interfaceFile,
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
        String GenerateStructPython::targetKeyword()
        {
          return String("python");
        }

        //---------------------------------------------------------------------
        String GenerateStructPython::targetKeywordHelp()
        {
          return String("Generate Python wrapper using C API");
        }

        //---------------------------------------------------------------------
        void GenerateStructPython::targetOutput(
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

          pathStr = UseHelper::fixRelativeFilePath(pathStr, String("python"));
          try {
            UseHelper::mkdir(pathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          pathStr += "/";

          String apisPathStr = UseHelper::fixRelativeFilePath(pathStr, String("wrapper"));
          try {
            UseHelper::mkdir(apisPathStr);
          } catch (const StdError &e) {
            ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_SYSTEM_ERROR, "Failed to create path \"" + pathStr + "\": " + " error=" + string(e.result()) + ", reason=" + e.message());
          }
          apisPathStr += "/";

          const ProjectPtr &project = config.mProject;
          if (!project) return;
          if (!project->mGlobal) return;

          SetupFile setupFile;
          setupFile.project_ = project;
          setupFile.global_ = project->mGlobal;

          setupFile.fileName_ = UseHelper::fixRelativeFilePath(pathStr, String("setup.py"));

          InitFile apiInitFile;

          apiInitFile.project_ = project;
          apiInitFile.global_ = project->mGlobal;

          apiInitFile.fileName_ = UseHelper::fixRelativeFilePath(apisPathStr, String("__init__.py"));

          ApiFile apiFile;
          apiFile.project_ = project;
          apiFile.global_ = project->mGlobal;

          GenerateStructC::calculateRelations(project->mGlobal, apiFile.derives_);
          GenerateStructC::calculateBoxings(project->mGlobal, apiFile.boxings_);

          apiFile.fileName_ = UseHelper::fixRelativeFilePath(apisPathStr, String("wrapper.py"));

          prepareSetupFile(setupFile);
          prepareApiInitFile(apiInitFile);
          prepareApiFile(apiFile);

          processNamespace(pathStr, setupFile, apiFile, apiFile.global_);

          finalizeApiFile(apiFile);
          finalizeApiInitFile(apiInitFile);
          finalizeSetupFile(setupFile);
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
