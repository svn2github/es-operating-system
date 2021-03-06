/*
 * Copyright 2010-2012 Esrille Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ElementImp.h"

#include <algorithm>
#include <new>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "AttrImp.h"
#include "DocumentImp.h"
#include "DOMTokenListImp.h"
#include "NodeListImp.h"
#include "MutationEventImp.h"
#include "css/CSSSerialize.h"
#include "html/HTMLTokenizer.h"

namespace org { namespace w3c { namespace dom { namespace bootstrap {

class AttrArray : public Object
{
    ElementImp* element;
public:
    virtual unsigned int getLength() {
        return element->attributes.size();
    }
    virtual void setLength(unsigned int length) {
    }
    virtual Attr getElement(unsigned int index) {
        if (element->attributes.size() <= index)
            return 0;
        return element->attributes[index];
    }
    virtual void setElement(unsigned int index, Attr value) {
    }
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv) {
        return ObjectArray<Attr>::dispatch(this, selector, id, argc, argv);
    }
    AttrArray(ElementImp* element) :
        Object(this),
        element(element) {
    }
};

void ElementImp::setAttributes(const std::deque<Attr>& attributes)
{
    if (this->attributes.empty()) {
        this->attributes = attributes;
        return;
    }
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        setAttributeNS(attr.getNamespaceURI(), attr.getName(), attr.getValue());
    }
}

ElementImp* ElementImp::getNextElement(ElementImp* root)
{
    NodeImp* n = this;
    for (auto i = n->firstChild; i; i = i->nextSibling) {
        if (i->getNodeType() == Node::ELEMENT_NODE)
            return dynamic_cast<ElementImp*>(i);
    }
    while (n != root) {
        while (n->nextSibling) {
            n = n->nextSibling;
            if (n->getNodeType() == Node::ELEMENT_NODE)
                return dynamic_cast<ElementImp*>(n);
        }
        n = n->parentNode;
    }
    return 0;
}

// Node

unsigned short ElementImp::getNodeType()
{
    return Node::ELEMENT_NODE;
}

Node ElementImp::cloneNode(bool deep)
{
    return new(std::nothrow) ElementImp(this, deep);
}

Nullable<std::u16string> ElementImp::getTextContent()
{
    std::u16string content;
    for (Node child = getFirstChild(); child; child = child.getNextSibling()) {
        if (child.getNodeType() == Node::COMMENT_NODE)
            continue;
        Nullable<std::u16string> text = child.getTextContent();
        if (text.hasValue())
            content += text.value();
    }
    return content;
}

void ElementImp::setTextContent(const std::u16string& textContent)
{
    while (hasChildNodes())
        removeChild(getFirstChild());
    if (0 < textContent.length()) {
        org::w3c::dom::Text text = ownerDocument->createTextNode(textContent);
        appendChild(text);
    }
}

bool ElementImp::isEqualNode(Node arg)
{
    ElementImp* element = dynamic_cast<ElementImp*>(arg.self());
    if (this == element)
        return true;
    if (!element)
        return false;
    if (namespaceURI != element->namespaceURI)
        return false;
    if (prefix != element->prefix)
        return false;
    if (localName != element->localName)
        return false;
    if (attributes.size() != element->attributes.size())
        return false;
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        bool found = false;
        for (auto j = element->attributes.begin(); j != element->attributes.end(); ++j) {
            if (i->getLocalName() == j->getLocalName()) {
                if (i->getNamespaceURI() != j->getNamespaceURI())
                    break;
                if (i->getValue() != j->getValue())
                    break;
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return NodeImp::isEqualNode(arg);
}

// Element
Nullable<std::u16string> ElementImp::getNamespaceURI()
{
    return namespaceURI;
}

Nullable<std::u16string> ElementImp::getPrefix()
{
    return prefix;
}

std::u16string ElementImp::getLocalName()
{
    return localName;
}

std::u16string ElementImp::getTagName()
{
    if (0 < prefix.length())
        return prefix + u':' + localName;
    return localName;
}

std::u16string ElementImp::getId()
{
    return getAttribute(u"id");
}

void ElementImp::setId(const std::u16string& id)
{
    setAttribute(u"id", id);
}

std::u16string ElementImp::getClassName()
{
    return getAttribute(u"class");
}

void ElementImp::setClassName(const std::u16string& className)
{
    setAttribute(u"class", className);
}

DOMTokenList ElementImp::getClassList()
{
    return new(std::nothrow) DOMTokenListImp(this, u"class");
}

dom::ObjectArray<Attr> ElementImp:: getAttributes()
{
    return new(std::nothrow) AttrArray(this);
}

Nullable<std::u16string> ElementImp::getAttribute(const std::u16string& name)
{
    // TODO: If the context node is in the HTML namespace and its ownerDocument is an HTML document
    std::u16string n(name);
        toLower(n);
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getName() == n)
            return attr.getValue();
    }
    return Nullable<std::u16string>();
}

Nullable<std::u16string> ElementImp::getAttributeNS(const std::u16string& namespaceURI, const std::u16string& localName)
{
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getNamespaceURI().hasValue() && attr.getNamespaceURI().value() == namespaceURI && attr.getLocalName() == localName)
            return attr.getValue();
    }
    return Nullable<std::u16string>();
}

void ElementImp::setAttribute(const std::u16string& name, const std::u16string& value)
{
    // TODO: If qualifiedName does not match the Name production in XML, raise an INVALID_CHARACTER_ERR exception and terminate these steps.
    // TODO: If the context node is in the HTML namespace and its ownerDocument is an HTML document
    std::u16string n(name);
        toLower(n);
    // TODO: If qualifiedName starts with "xmlns", raise a NAMESPACE_ERR and terminate these steps.
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getName() == n) {
            std::u16string prevValue = attr.getValue();
            attr.setValue(value);

            events::MutationEvent event = new(std::nothrow) MutationEventImp;
            event.initMutationEvent(u"DOMAttrModified",
                                    true, false, attr, prevValue, value, n, events::MutationEvent::MODIFICATION);
            this->dispatchEvent(event);
            return;
        }
    }
    Attr attr = new(std::nothrow) AttrImp(Nullable<std::u16string>(), Nullable<std::u16string>(), n, value);
    if (attr)
        attributes.push_back(attr);
}

void ElementImp::setAttributeNS(const std::u16string& namespaceURI, const std::u16string& qualifiedName, const std::u16string& value)
{
    // TODO:
    // TODO:
    Nullable<std::u16string> prefix;
    std::u16string localName;
    size_t pos = qualifiedName.find(u':');
    if (pos != std::u16string::npos) {
        prefix = qualifiedName.substr(0, pos);
        localName = qualifiedName.substr(pos + 1);
    } else
        localName = qualifiedName;
    if (prefix.hasValue()) {
        if (namespaceURI.length() == 0)
            throw DOMException{DOMException::NAMESPACE_ERR};
        if (prefix.value() == u"xml" && namespaceURI != u"http://www.w3.org/XML/1998/namespace")
            throw DOMException{DOMException::NAMESPACE_ERR};
    }
    if ((qualifiedName == u"xmlns" || prefix.hasValue() && prefix.value() == u"xmlns") &&
        namespaceURI != u"http://www.w3.org/2000/xmlns") {
        throw DOMException{DOMException::NAMESPACE_ERR};
    }
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getNamespaceURI().hasValue() && attr.getNamespaceURI().value() == namespaceURI && attr.getLocalName() == localName) {
            attr.setValue(value);
            // TODO: set prefix, too.
            return;
        }
    }
    Attr attr = new(std::nothrow) AttrImp(namespaceURI, prefix, localName, value);
    if (attr)
        attributes.push_back(attr);
}

void ElementImp::removeAttribute(const std::u16string& name)
{
    // TODO: If the context node is in the HTML namespace and its ownerDocument is an HTML document
    std::u16string n(name);
        toLower(n);
    for (auto i = attributes.begin(); i != attributes.end();) {
        Attr attr = *i;
        if (attr.getName() == n)
            i = attributes.erase(i);
        else
            ++i;
    }
}

void ElementImp::removeAttributeNS(const std::u16string& namespaceURI, const std::u16string& localName)
{
    for (auto i = attributes.begin(); i != attributes.end();) {
        Attr attr = *i;
        if (attr.getNamespaceURI().hasValue() && attr.getNamespaceURI().value() == namespaceURI && attr.getLocalName() == localName)
            i = attributes.erase(i);
        else
            ++i;
    }
}

bool ElementImp::hasAttribute(const std::u16string& name)
{
    // TODO: If the context node is in the HTML namespace and its ownerDocument is an HTML document
    std::u16string n(name);
        toLower(n);
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getName() == n)
            return true;
    }
    return false;
}

bool ElementImp::hasAttributeNS(const std::u16string& namespaceURI, const std::u16string& localName)
{
    for (auto i = attributes.begin(); i != attributes.end(); ++i) {
        Attr attr = *i;
        if (attr.getNamespaceURI().hasValue() && attr.getNamespaceURI().value() == namespaceURI && attr.getLocalName() == localName)
            return true;
    }
    return false;
}

html::HTMLCollection ElementImp::getChildren()
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

NodeListImp* ElementImp::getElementsByTagName(ElementImp* element, const std::u16string& qualifiedName)
{
    NodeListImp* nodeList = new(std::nothrow) NodeListImp;
    if (!nodeList)
        return 0;

    if (qualifiedName == u"*") {
        for (ElementImp* e = element; e; e = e->getNextElement())
            nodeList->addItem(e);
    } else {
        // TODO: Support non HTML document
        for (ElementImp* e = element; e; e = e->getNextElement()) {
            if (e->getLocalName() == qualifiedName)
                nodeList->addItem(e);
        }
    }
    return nodeList;
}

NodeList ElementImp::getElementsByTagName(const std::u16string& qualifiedName)
{
    return getElementsByTagName(this, qualifiedName);
}

NodeList ElementImp::getElementsByTagNameNS(const std::u16string& _namespace, const std::u16string& localName)
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

NodeListImp* ElementImp::getElementsByClassName(ElementImp* element, const std::u16string& classNames)
{
    NodeListImp* nodeList = new(std::nothrow) NodeListImp;
    if (!nodeList)
        return 0;

    std::vector<std::u16string> classes;
    boost::algorithm::split(classes, classNames, isSpace);
    for (ElementImp* e = element; e; e = e->getNextElement()) {
        std::u16string c = e->getClassName();
        std::vector<std::u16string> v;
        boost::algorithm::split(v, c, isSpace);
        bool notFound = false;
        for (auto i = classes.begin(); i != classes.end(); ++i) {
            if (std::find(v.begin(), v.end(), *i) == v.end()) {
                notFound = true;
                break;
            }
        }
        if (!notFound)
            nodeList->addItem(e);
    }
    return nodeList;
}

NodeList ElementImp::getElementsByClassName(const std::u16string& classNames)
{
    return getElementsByClassName(this, classNames);
}

Element ElementImp::getFirstElementChild()
{
    for (NodeImp* n = this->firstChild; n; n = n->nextSibling) {
        if (dynamic_cast<ElementImp*>(n))
            return n;
    }
    return 0;
}

Element ElementImp::getLastElementChild()
{
    for (NodeImp* n = this->lastChild; n; n = n->previousSibling) {
        if (dynamic_cast<ElementImp*>(n))
            return n;
    }
    return 0;
}

Element ElementImp::getPreviousElementSibling()
{
    NodeImp* n = this;
    while (n = n->previousSibling) {
        if (dynamic_cast<ElementImp*>(n))
            return n;
    }
    return 0;
}

Element ElementImp::getNextElementSibling()
{
    NodeImp* n = this;
    while (n = n->nextSibling) {
        if (dynamic_cast<ElementImp*>(n))
            return n;
    }
    return 0;
}

unsigned int ElementImp::getChildElementCount()
{
    unsigned int count = 0;
    for (NodeImp* n = this->firstChild; n; n = n->nextSibling) {
        if (dynamic_cast<ElementImp*>(n))
            ++count;
    }
    return count;
}

views::ClientRectList ElementImp::getClientRects()
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

views::ClientRect ElementImp::getBoundingClientRect()
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

void ElementImp::scrollIntoView(bool top)
{
    // TODO: implement me!
}

int ElementImp::getScrollTop()
{
    // TODO: implement me!
    return 0;
}

void ElementImp::setScrollTop(int scrollTop)
{
    // TODO: implement me!
}

int ElementImp::getScrollLeft()
{
    // TODO: implement me!
    return 0;
}

void ElementImp::setScrollLeft(int scrollLeft)
{
    // TODO: implement me!
}

int ElementImp::getScrollWidth()
{
    // TODO: implement me!
    return 0;
}

int ElementImp::getScrollHeight()
{
    // TODO: implement me!
    return 0;
}

int ElementImp::getClientTop()
{
    // TODO: implement me!
    return 0;
}

int ElementImp::getClientLeft()
{
    // TODO: implement me!
    return 0;
}

int ElementImp::getClientWidth()
{
    // TODO: implement me!
    return 0;
}

int ElementImp::getClientHeight()
{
    // TODO: implement me!
    return 0;
}

Element ElementImp::querySelector(const std::u16string& selectors)
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

NodeList ElementImp::querySelectorAll(const std::u16string& selectors)
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

xbl2::XBLImplementationList ElementImp::getXblImplementations()
{
    // TODO: implement me!
    return static_cast<Object*>(0);
}

void ElementImp::addBinding(const std::u16string& bindingURI)
{
    // TODO: implement me!
}

void ElementImp::removeBinding(const std::u16string& bindingURI)
{
    // TODO: implement me!
}

bool ElementImp::hasBinding(const std::u16string& bindingURI)
{
    // TODO: implement me!
    return false;
}

ElementImp::ElementImp(DocumentImp* ownerDocument, const std::u16string& localName, const std::u16string& namespaceURI, const std::u16string& prefix) :
    ObjectMixin(ownerDocument),
    namespaceURI(namespaceURI),
    prefix(prefix),
    localName(localName)
{
    nodeName = getTagName();
}

ElementImp::ElementImp(ElementImp* org, bool deep) :
    ObjectMixin(org, deep)
{
    namespaceURI = org->namespaceURI;
    prefix = org->prefix;
    localName = org->localName;
    for (auto i = org->attributes.begin(); i != org->attributes.end(); ++i) {
        if (Attr attr = new(std::nothrow) AttrImp(*dynamic_cast<AttrImp*>((*i).self())))
            attributes.push_back(attr);
    }
}

}}}}  // org::w3c::dom::bootstrap
