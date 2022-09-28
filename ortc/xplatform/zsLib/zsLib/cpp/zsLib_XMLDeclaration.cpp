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

#pragma warning(push)
#pragma warning(disable: 4290)

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
      #pragma mark XML::internal::Declaration
      #pragma mark

      //-----------------------------------------------------------------------
      Declaration::Declaration()
      {
      }

      //-----------------------------------------------------------------------
      Declaration::~Declaration()
      {
      }

      //-----------------------------------------------------------------------
      void Declaration::parse(XML::ParserPos &ioPos)
      {
        Parser::AutoStack stack(ioPos);

        // this must be a <?xml
        ZS_THROW_BAD_STATE_IF(!ioPos.isString("<?xml", false))

        ioPos += strlen("<?xml");

        while ((*ioPos) &&
               (!ioPos.isString("?>")))
        {
          if (Parser::skipWhiteSpace(ioPos))
            continue;

          if (('>' == (*ioPos)) ||
              (ioPos.isString("/>")))
          {
            // this not a proper end to the tag but it must be an end tag
            (ioPos.getParser())->addWarning(ParserWarningType_NotProperEndDeclaration, ioPos);
            if (ioPos.isString("/>"))
              ioPos += strlen("/>");
            else
              ++ioPos;

            // stop processing this attribute
            return;
          }

          if (!Parser::isLegalName(*ioPos, true))
          {
            // this isn't a legal name for an attribute, skip it
            (ioPos.getParser())->addWarning(ParserWarningType_IllegalAttributeName, ioPos);

            while ((*ioPos) &&
                   (!ioPos.isString("?>")) &&
                   ('>' != (*ioPos)) &&
                   (!Parser::isWhiteSpace(ioPos)))
            {
              ++ioPos;
            }
            continue;
          }

          AttributePtr attribute = XML::Attribute::create();
          if (!attribute->parse(ioPos))
            continue;

          // attribute was found, add it to the attribute list
          bool duplicateAttribute = (mThis.lock())->setAttribute(attribute);
          if (duplicateAttribute)
          {
            (ioPos.getParser())->addWarning(ParserWarningType_DuplicateAttribute);
          }
        }

        if (!ioPos.isString("?>"))
        {
          (ioPos.getParser())->addWarning(ParserWarningType_NoEndDeclarationFound);
          return;
        }

        ioPos += strlen("?>");
      }

      //-----------------------------------------------------------------------
      size_t Declaration::getOutputSizeXML(const GeneratorPtr &inGenerator) const
      {
        size_t result = 0;
        result += strlen("<?xml");

        if (mFirstAttribute)
        {
          NodePtr child = mFirstAttribute;
          while (child)
          {
            AttributePtr attribute = child->toAttribute();
            result += strlen(" ");
            result += Generator::getOutputSize(inGenerator, attribute);
            child = child->getNextSibling();
          }
        }

        result += strlen(" ?>");
        return result;
      }

      //-----------------------------------------------------------------------
      void Declaration::writeBufferXML(const GeneratorPtr &inGenerator, char * &ioPos) const
      {
        Generator::writeBuffer(ioPos, "<?xml");

        if (mFirstAttribute)
        {
          NodePtr child = mFirstAttribute;
          while (child)
          {
            AttributePtr attribute = child->toAttribute();
            Generator::writeBuffer(ioPos, " ");
            Generator::writeBuffer(inGenerator, attribute, ioPos);
            child = child->getNextSibling();
          }
        }
        Generator::writeBuffer(ioPos, " ?>");
      }

      //-----------------------------------------------------------------------
      size_t Declaration::getOutputSizeJSON(const GeneratorPtr &inGenerator) const
      {
        size_t result = 0;
        return result;
      }

      //-----------------------------------------------------------------------
      void Declaration::writeBufferJSON(const GeneratorPtr &inGenerator, char * &ioPos) const
      {
      }

      //-----------------------------------------------------------------------
      NodePtr Declaration::cloneAssignParent(NodePtr inParent) const
      {
        XML::DeclarationPtr newObject(XML::Declaration::create());
        Parser::safeAdoptAsLastChild(inParent, newObject);

        NodePtr child = mFirstAttribute;
        while (child)
        {
          NodePtr newChild = Parser::cloneAssignParent(newObject, child);
          child = child->getNextSibling();
        }
        (mThis.lock())->cloneChildren(mThis.lock(), newObject);
        return newObject;
      }

    } // namespace internal


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark XML::Declaration
    #pragma mark

    //-------------------------------------------------------------------------
    Declaration::Declaration(const make_private &) :
      internal::Declaration()
    {
    }

    //-------------------------------------------------------------------------
    DeclarationPtr Declaration::create()
    {
      DeclarationPtr newObject(make_shared<Declaration>(make_private {}));
      newObject->mThis = newObject;
      return newObject;
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::findAttribute(String inName) const
    {
      DocumentPtr document(getDocument());

      bool caseSensative = true;
      if (document)
        caseSensative = document->isAttributeNameIsCaseSensative();

      NodePtr child = mFirstAttribute;
      while (child)
      {
        AttributePtr attribute(child->toAttribute());
        if (caseSensative)
        {
          if (inName == attribute->getName())
            return attribute;
        }
        else
        {
          if (0 == inName.compareNoCase(attribute->getName()))
            return attribute;
        }
        child = child->getNextSibling();
      }
      return AttributePtr();
    }

    //-------------------------------------------------------------------------
    String Declaration::getAttributeValue(String inName) const
    {
      AttributePtr attribute = findAttribute(inName);
      if (attribute)
        return attribute->getValue();

      return String();
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::findAttributeChecked(String inName) const throw(Exceptions::CheckFailed)
    {
      AttributePtr result = findAttribute(inName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    String Declaration::getAttributeValueChecked(String inName) const throw(Exceptions::CheckFailed)
    {
      AttributePtr result = findAttribute(inName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result->getValue();
    }

    //-------------------------------------------------------------------------
    bool Declaration::setAttribute(String inName, String inValue)
    {
      AttributePtr attribute = Attribute::create();
      attribute->setName(inName);
      attribute->setValue(inValue);
      return setAttribute(attribute);
    }

    //-------------------------------------------------------------------------
    bool Declaration::setAttribute(AttributePtr inAttribute)
    {
      ZS_THROW_INVALID_USAGE_IF(!inAttribute)

      bool replaced = deleteAttribute(inAttribute->getName());
      mThis.lock()->adoptAsLastChild(inAttribute);
      return replaced;
    }

    //-------------------------------------------------------------------------
    bool Declaration::deleteAttribute(String inName)
    {
      AttributePtr attribute = findAttribute(inName);
      if (!attribute)
        return false;

      attribute->orphan();
      return true;
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::getFirstAttribute() const
    {
      return mFirstAttribute;
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::getLastAttribute() const
    {
      return mLastAttribute;
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::getFirstAttributeChecked() const throw(Exceptions::CheckFailed)
    {
      AttributePtr result = getFirstAttribute();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    AttributePtr Declaration::getLastAttributeChecked() const throw(Exceptions::CheckFailed)
    {
      AttributePtr result = getLastAttribute();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    void Declaration::adoptAsFirstChild(NodePtr inNode)
    {
      if (!inNode)
        return;

      if (!inNode->isAttribute())
      {
        ZS_THROW_INVALID_USAGE("declarations can only have attributes as children")
        return;
      }

      AttributePtr attribute = inNode->toAttribute();

      // first the node has to be orphaned
      inNode->orphan();

      // ensure this attribute does not already exist
      deleteAttribute(attribute->getName());

      // adopt this as the parent
      inNode->mParent = toNode();

      // point existing first child's previous sibling to new node
      if (mFirstAttribute)
        mFirstAttribute->mPreviousSibling = inNode;

      // point new node's next sibling to current first child
      inNode->mNextSibling = mFirstAttribute;

      // first child is now new node
      mFirstAttribute = attribute;

      // if there wasn't a last child then it is now pointing to the new node
      if (!mLastAttribute)
        mLastAttribute = attribute;
    }

    //-------------------------------------------------------------------------
    void Declaration::adoptAsLastChild(NodePtr inNode)
    {
      if (!inNode)
        return;

      if (!inNode->isAttribute())
      {
        ZS_THROW_INVALID_USAGE("declarations can only have attributes as children")
        return;
      }

      AttributePtr attribute = inNode->toAttribute();

      // first the node has to be orphaned
      inNode->orphan();

      // ensure this attribute does not already exist
      deleteAttribute(attribute->getName());

      // adopt this as the parent
      inNode->mParent = toNode();

      // point existing last child's next sibling to new node
      if (mLastAttribute)
        mLastAttribute->mNextSibling = inNode;

      // point new node's next sibling to current first child
      inNode->mPreviousSibling = mLastAttribute;

      // first child is now new node
      mLastAttribute = attribute;

      // if there wasn't a last child then it is now pointing to the new node
      if (!mFirstAttribute)
        mFirstAttribute = attribute;
    }

    //-------------------------------------------------------------------------
    NodePtr Declaration::clone() const
    {
      return cloneAssignParent(NodePtr());
    }

    //-------------------------------------------------------------------------
    void Declaration::clear()
    {
      NodePtr child = mFirstAttribute;
      while (child)
      {
        NodePtr tempNext = child->getNextSibling();
        child->orphan();
        child->clear();
        child = tempNext;
      }
      Node::clear();
    }

    //-------------------------------------------------------------------------
    Node::NodeType::Type Declaration::getNodeType()
    {
      return NodeType::Declaration;
    }

    //-------------------------------------------------------------------------
    bool Declaration::isDeclaration() const
    {
      return true;
    }

    //-------------------------------------------------------------------------
    NodePtr Declaration::toNode() const
    {
      return mThis.lock();
    }

    //-------------------------------------------------------------------------
    DeclarationPtr Declaration::toDeclaration() const
    {
      return mThis.lock();
    }

  } // namespace XML

} // namespace zsLib

#pragma warning(pop)
