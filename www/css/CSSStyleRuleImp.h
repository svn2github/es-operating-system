/*
 * Copyright 2010, 2011 Esrille Inc.
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

#ifndef CSSSTYLERULE_IMP_H
#define CSSSTYLERULE_IMP_H

#include <Object.h>
#include <org/w3c/dom/css/CSSStyleRule.h>

#include <org/w3c/dom/css/CSSRule.h>
#include <org/w3c/dom/css/CSSStyleDeclaration.h>

#include "CSSRuleImp.h"
#include "CSSParser.h"
#include "CSSSelector.h"

namespace org { namespace w3c { namespace dom { namespace bootstrap {

class CSSStyleRuleImp : public ObjectMixin<CSSStyleRuleImp, CSSRuleImp>
{
    CSSSelectorsGroup* selectorsGroup;
    css::CSSStyleDeclaration styleDeclaration;
public:
    CSSSelector* match(Element element);
    CSSSpecificity getLastSpecificity() {  // of the last matched selector
        return selectorsGroup->getLastSpecificity();
    }

    // CSSRule
    virtual unsigned short getType();

    // CSSStyleRule
    virtual std::u16string getSelectorText();
    virtual void setSelectorText(std::u16string selectorText);
    virtual css::CSSStyleDeclaration getStyle();

    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv) {
        return css::CSSStyleRule::dispatch(this, selector, id, argc, argv);
    }

    CSSStyleRuleImp(CSSSelectorsGroup* selectorsGroup, css::CSSStyleDeclaration styleDeclaration);
};

}}}}  // org::w3c::dom::bootstrap

#endif  // CSSSTYLERULE_IMP_H
