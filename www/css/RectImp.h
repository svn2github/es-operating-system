// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_RECTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_RECTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/css/Rect.h>

#include <org/w3c/dom/css/CSSPrimitiveValue.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class RectImp : public ObjectMixin<RectImp>
{
public:
    // Rect
    css::CSSPrimitiveValue getTop();
    css::CSSPrimitiveValue getRight();
    css::CSSPrimitiveValue getBottom();
    css::CSSPrimitiveValue getLeft();
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return css::Rect::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return css::Rect::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_RECTIMP_H_INCLUDED
