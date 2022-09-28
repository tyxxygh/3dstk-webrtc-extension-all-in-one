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

#include <zsLib/XML.h>
#include <zsLib/Exception.h>

namespace zsLib {ZS_DECLARE_SUBSYSTEM(zslib)}

namespace zsLib
{

  namespace XML
  {

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark XML::intenral::Text
      #pragma mark

      //-----------------------------------------------------------------------
      Text::Text() :
        mFormat(XML::Text::Format_EntityEncoded),
        mOutputFormat(XML::Text::Format_EntityEncoded)
      {
      }

      //-----------------------------------------------------------------------
      Text::~Text()
      {
      }

      //-----------------------------------------------------------------------
      void Text::parse(XML::ParserPos &ioPos)
      {
        Parser::AutoStack stack(ioPos);

        mValue.clear();

        mFormat = (ioPos.isString("<![CDATA[") ? XML::Text::Format_CDATA : XML::Text::Format_EntityEncoded);
        mOutputFormat = mFormat;

        if (XML::Text::Format_CDATA == mFormat)
        {
          // skip over the CDATA tag
          ioPos += strlen("<![CDATA[");

          while ((*ioPos) &&
                 (!ioPos.isString("]]>")))
          {
            mValue += *ioPos;
            ++ioPos;
          }

          if (ioPos.isEOF())
          {
            // did not find end to text element
            (ioPos.getParser())->addWarning(ParserWarningType_CDATAMissingEndTag);
            return;
          }

          // done parsing the CDATA
          ioPos += strlen("]]>");
          return;
        }

        while (*ioPos)
        {
          // can only store '<' if at the start of the text block, otherwise it will think '<' is a new element
          if ((!mValue.isEmpty()) &&
              ('<' == *ioPos))
            break;

          mValue += *ioPos;
          ++ioPos;
        }
      }

      //-----------------------------------------------------------------------
      size_t Text::getOutputSizeXML(const GeneratorPtr &inGenerator) const
      {
        bool normalizeCDATA = (0 != (XML::Generator::XMLWriteFlag_NormalizeCDATA & inGenerator->getXMLWriteFlags()));
        bool encode0xDCharactersInText = (0 != (XML::Generator::XMLWriteFlag_EntityEncode0xDInText & inGenerator->getXMLWriteFlags()));
        if (encode0xDCharactersInText) {
          normalizeCDATA = true;
        }
        XML::Text::Formats outputFormat = static_cast<XML::Text::Formats>(mOutputFormat);
        if (normalizeCDATA) {
          if (XML::Text::Format_CDATA == outputFormat) {
            outputFormat = XML::Text::Format_EntityEncoded;
          }
        }

        if ((XML::Text::Format_JSONStringEncoded == outputFormat) ||
            (XML::Text::Format_JSONNumberEncoded == outputFormat)) {
          outputFormat = XML::Text::Format_EntityEncoded;
        }

        String value = getValueInFormat(outputFormat, normalizeCDATA, encode0xDCharactersInText);

        size_t result = 0;

        if (XML::Text::Format_CDATA == outputFormat)
          result += strlen("<![CDATA[");

        result += value.length();

        if (XML::Text::Format_CDATA == outputFormat)
          result += strlen("]]>");

        return result;
      }

      //-----------------------------------------------------------------------
      void Text::writeBufferXML(const GeneratorPtr &inGenerator, char * &ioPos) const
      {
        bool normalizeCDATA = (0 != (XML::Generator::XMLWriteFlag_NormalizeCDATA & inGenerator->getXMLWriteFlags()));
        bool encode0xDCharactersInText = (0 != (XML::Generator::XMLWriteFlag_EntityEncode0xDInText & inGenerator->getXMLWriteFlags()));
        if (encode0xDCharactersInText) {
          normalizeCDATA = true;
        }
        XML::Text::Formats outputFormat = static_cast<XML::Text::Formats>(mOutputFormat);
        if (normalizeCDATA) {
          if (XML::Text::Format_CDATA == outputFormat) {
            outputFormat = XML::Text::Format_EntityEncoded;
          }
        }

        if ((XML::Text::Format_JSONStringEncoded == outputFormat) ||
            (XML::Text::Format_JSONNumberEncoded == outputFormat)) {
          outputFormat = XML::Text::Format_EntityEncoded;
        }

        String value = getValueInFormat(outputFormat, normalizeCDATA, encode0xDCharactersInText);

        if (XML::Text::Format_CDATA == outputFormat)
          Generator::writeBuffer(ioPos, "<![CDATA[");

        Generator::writeBuffer(ioPos, value);

        if (XML::Text::Format_CDATA == outputFormat)
          Generator::writeBuffer(ioPos, "]]>");
      }

      //-----------------------------------------------------------------------
      size_t Text::getOutputSizeJSON(const GeneratorPtr &inGenerator) const
      {
        bool normalizeCDATA = (0 != (XML::Generator::XMLWriteFlag_NormalizeCDATA & inGenerator->getXMLWriteFlags()));
        String value = getValueInFormat(XML::Text::Format_JSONNumberEncoded == mFormat ? XML::Text::Format_JSONNumberEncoded : XML::Text::Format_JSONStringEncoded, normalizeCDATA);

        size_t result = 0;
        result += value.length();
        return result;
      }

      //-----------------------------------------------------------------------
      void Text::writeBufferJSON(const GeneratorPtr &inGenerator, char * &ioPos) const
      {
        bool normalizeCDATA = (0 != (XML::Generator::XMLWriteFlag_NormalizeCDATA & inGenerator->getXMLWriteFlags()));

        String value = getValueInFormat(XML::Text::Format_JSONNumberEncoded == mFormat ? XML::Text::Format_JSONNumberEncoded : XML::Text::Format_JSONStringEncoded, normalizeCDATA);
        Generator::writeBuffer(ioPos, value);
      }

      //-----------------------------------------------------------------------
      NodePtr Text::cloneAssignParent(NodePtr inParent) const
      {
        TextPtr newObject(XML::Text::create());
        Parser::safeAdoptAsLastChild(inParent, newObject);
        newObject->mValue = mValue;
        newObject->mFormat = mFormat;
        newObject->mOutputFormat = mOutputFormat;
        (mThis.lock())->cloneChildren(mThis.lock(), newObject);    // should do noop since text nodes aren't allowed children
        return newObject;
      }

      //-----------------------------------------------------------------------
      String Text::getValueInFormat(
                                    UINT outputFormat,
                                    bool normalize,
                                    bool encode0xDCharactersInText
                                    ) const
      {
        String value = mValue;

        XML::Text::Formats inFormat = static_cast<XML::Text::Formats>(mFormat);

        if (normalize) {
          value = getValueInFormat(XML::Text::Format_CDATA, false);
          inFormat = XML::Text::Format_CDATA;
        }

        switch (static_cast<XML::Text::Formats>(outputFormat))
        {
          case XML::Text::Format_EntityEncoded:
          {
            switch (inFormat)
            {
              case XML::Text::Format_EntityEncoded:       return value;
              case XML::Text::Format_CDATA:               return XML::Parser::makeTextEntitySafe(value, encode0xDCharactersInText);
              case XML::Text::Format_JSONStringEncoded:   return XML::Parser::makeTextEntitySafe(XML::Parser::convertFromJSONEncoding(value));
              case XML::Text::Format_JSONNumberEncoded:   return XML::Parser::makeTextEntitySafe(XML::Parser::convertFromJSONEncoding(value));
            }
            break;
          }
          case XML::Text::Format_CDATA:
          {
            switch (inFormat)
            {
              case XML::Text::Format_EntityEncoded:       return XML::Parser::convertFromEntities(value);
              case XML::Text::Format_CDATA:               return value;
              case XML::Text::Format_JSONStringEncoded:   return XML::Parser::convertFromJSONEncoding(value);
              case XML::Text::Format_JSONNumberEncoded:   return XML::Parser::convertFromJSONEncoding(value);
            }
            break;
          }
          case XML::Text::Format_JSONStringEncoded:
          case XML::Text::Format_JSONNumberEncoded:
          {
            switch (inFormat)
            {
              case XML::Text::Format_EntityEncoded:       return XML::Parser::convertToJSONEncoding(XML::Parser::convertFromEntities(value));
              case XML::Text::Format_CDATA:               return XML::Parser::convertToJSONEncoding(value);
              case XML::Text::Format_JSONStringEncoded:   return value;
              case XML::Text::Format_JSONNumberEncoded:   return value;
            }
            break;
          }
        }
        return value;
      }

    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark XML::Text
    #pragma mark

    //-------------------------------------------------------------------------
    TextPtr Text::create()
    {
      TextPtr object(make_shared<Text>(make_private{}));
      object->mThis = object;
      return object;
    }

    //-------------------------------------------------------------------------
    Text::Text(const make_private &) :
      internal::Text()
    {
    }

    //-------------------------------------------------------------------------
    void Text::setValue(const String &inText, Formats format)
    {
      mFormat = format;
      mOutputFormat = format;

      mValue = inText;
    }

    //-------------------------------------------------------------------------
    void Text::setValueAndEntityEncode(const String &inText)
    {
      mFormat = Format_EntityEncoded;
      mOutputFormat = Format_EntityEncoded;

      mValue = Parser::makeTextEntitySafe(inText);
    }

    //-------------------------------------------------------------------------
    void Text::setValueAndJSONEncode(const String &inText)
    {
      mFormat = Format_JSONStringEncoded;
      mOutputFormat = Format_JSONStringEncoded;

      mValue = Parser::convertToJSONEncoding(inText);
    }

    //-------------------------------------------------------------------------
    Text::Formats Text::getFormat() const
    {
      return static_cast<Text::Formats>(mFormat);
    }

    //-------------------------------------------------------------------------
    Text::Formats Text::getOutputFormat() const
    {
      return static_cast<Text::Formats>(mOutputFormat);
    }

    //-------------------------------------------------------------------------
    void Text::setOutputFormat(Formats format)
    {
      mOutputFormat = format;
    }

    //-------------------------------------------------------------------------
    bool Text::hasChildren()
    {
      return false;
    }

    //-------------------------------------------------------------------------
    void Text::removeChildren()
    {      
    }

    //-------------------------------------------------------------------------
    NodePtr Text::clone() const
    {
      return cloneAssignParent(NodePtr());
    }

    //-------------------------------------------------------------------------
    void Text::clear()
    {
      mValue.clear();
      mFormat = Format_EntityEncoded;
      mOutputFormat = Format_EntityEncoded;
    }

    //-------------------------------------------------------------------------
    String Text::getValue() const
    {
      return mValue;
    }

    //-------------------------------------------------------------------------
    String Text::getValueDecoded() const
    {
      if (Format_EntityEncoded == mFormat)
        return Parser::convertFromEntities(getValue());

      if ((Format_JSONStringEncoded == mFormat) ||
          (Format_JSONNumberEncoded == mFormat))
        return Parser::convertFromJSONEncoding(getValue());

      return mValue;
    }

    //-------------------------------------------------------------------------
    String Text::getValueInFormat(
                                  Formats outputFormat,
                                  bool normalize,
                                  bool encode0xDCharactersInText
                                  ) const
    {
      return internal::Text::getValueInFormat((UINT)outputFormat, normalize, encode0xDCharactersInText);
    }

    //-------------------------------------------------------------------------
    void Text::adoptAsFirstChild(NodePtr inNode)
    {
      ZS_THROW_INVALID_USAGE("text blocks cannot have children")
    }

    //-------------------------------------------------------------------------
    void Text::adoptAsLastChild(NodePtr inNode)
    {
      ZS_THROW_INVALID_USAGE("text blocks cannot have children")
    }

    //-------------------------------------------------------------------------
    Node::NodeType::Type Text::getNodeType()
    {
      return NodeType::Text;
    }

    //-------------------------------------------------------------------------
    bool Text::isText() const
    {
      return true;
    }

    //-------------------------------------------------------------------------
    NodePtr Text::toNode() const
    {
      return mThis.lock();
    }

    //-------------------------------------------------------------------------
    TextPtr Text::toText() const
    {
      return mThis.lock();
    }


  } // namespace XML

} // namespace zsLib
