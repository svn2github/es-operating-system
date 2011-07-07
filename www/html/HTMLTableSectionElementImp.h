// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_HTMLTABLESECTIONELEMENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_HTMLTABLESECTIONELEMENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/HTMLTableSectionElement.h>
#include "HTMLElementImp.h"

#include <org/w3c/dom/html/HTMLElement.h>
#include <org/w3c/dom/html/HTMLCollection.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class HTMLTableSectionElementImp : public ObjectMixin<HTMLTableSectionElementImp, HTMLElementImp>
{
public:
    // HTMLTableSectionElement
    html::HTMLCollection getRows();
    html::HTMLElement insertRow();
    html::HTMLElement insertRow(int index);
    void deleteRow(int index);
    // HTMLTableSectionElement-31
    std::u16string getAlign();
    void setAlign(std::u16string align);
    std::u16string getCh();
    void setCh(std::u16string ch);
    std::u16string getChOff();
    void setChOff(std::u16string chOff);
    std::u16string getVAlign();
    void setVAlign(std::u16string vAlign);
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::HTMLTableSectionElement::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::HTMLTableSectionElement::getMetaData();
    }
    HTMLTableSectionElementImp(DocumentImp* ownerDocument, const std::u16string& localName) :
        ObjectMixin(ownerDocument, localName) {
    }
    HTMLTableSectionElementImp(HTMLTableSectionElementImp* org, bool deep) :
        ObjectMixin(org, deep) {
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_HTMLTABLESECTIONELEMENTIMP_H_INCLUDED
