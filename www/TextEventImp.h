// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_TEXTEVENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_TEXTEVENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/events/TextEvent.h>
#include "UIEventImp.h"

#include <org/w3c/dom/events/UIEvent.h>
#include <org/w3c/dom/html/Window.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class TextEventImp : public ObjectMixin<TextEventImp, UIEventImp>
{
public:
    // TextEvent
    std::u16string getData() __attribute__((weak));
    unsigned int getInputMethod() __attribute__((weak));
    std::u16string getLocale() __attribute__((weak));
    void initTextEvent(std::u16string typeArg, bool canBubbleArg, bool cancelableArg, html::Window viewArg, std::u16string dataArg, unsigned int inputMethod, std::u16string localeArg) __attribute__((weak));
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return events::TextEvent::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return events::TextEvent::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_TEXTEVENTIMP_H_INCLUDED
