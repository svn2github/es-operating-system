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

#include "Box.h"

#include <unicode/uchar.h>

#include <algorithm>
#include <new>
#include <iostream>

#include <Object.h>
#include <org/w3c/dom/Document.h>
#include <org/w3c/dom/Element.h>
#include <org/w3c/dom/Text.h>
#include <org/w3c/dom/html/HTMLIFrameElement.h>
#include <org/w3c/dom/html/HTMLImageElement.h>
#include <org/w3c/dom/html/HTMLDivElement.h>

#include "CSSSerialize.h"
#include "CSSStyleDeclarationImp.h"
#include "CSSTokenizer.h"
#include "FormattingContext.h"
#include "StackingContext.h"
#include "ViewCSSImp.h"
#include "WindowImp.h"

#include "Test.util.h"

namespace org { namespace w3c { namespace dom { namespace bootstrap {

namespace {

CSSStyleDeclarationImp* setActiveStyle(ViewCSSImp* view, CSSStyleDeclarationImp* style, FontTexture*& font, float& point)
{
    font = style->getFontTexture();
    point = view->getPointFromPx(style->fontSize.getPx());
    return style;
}

size_t getfirstLetterLength(const std::u16string& data, size_t position)
{
    size_t fitLength = data.size() - position;
    if (0 < fitLength) {
        fitLength = 0;
        while (u_ispunct(nextChar(data, position)))
            fitLength = position;
        nextChar(data, fitLength);
        while (u_ispunct(nextChar(data, position)))
            fitLength = position;
    }
    return fitLength;
}

}

void Block::nextLine(ViewCSSImp* view, FormattingContext* context, CSSStyleDeclarationImp*& activeStyle,
                     CSSStyleDeclarationPtr& firstLetterStyle, CSSStyleDeclarationPtr& firstLineStyle,
                     CSSStyleDeclarationImp* style, bool linefeed,
                     FontTexture*& font, float& point)
{
    if (firstLetterStyle) {
        firstLetterStyle = 0;
        if (firstLineStyle)
            activeStyle = setActiveStyle(view, firstLineStyle.get(), font, point);
        else
            activeStyle = setActiveStyle(view, style, font, point);
    } else {
        context->nextLine(view, this, linefeed);
        if (firstLineStyle) {
            firstLineStyle = 0;
            activeStyle = setActiveStyle(view, style, font, point);
        }
    }
}

void Block::getPsuedoStyles(ViewCSSImp* view, FormattingContext* context, CSSStyleDeclarationImp* style,
                            CSSStyleDeclarationPtr& firstLetterStyle, CSSStyleDeclarationPtr& firstLineStyle)
{
    bool isFirstLetter = true;
    for (Box* i = context->lineBox->getFirstChild(); i; i = i->getNextSibling()) {
        if (dynamic_cast<InlineBox*>(i)) {
            isFirstLetter = false;
            break;
        }
    }

    // The current line box is the 1st line of this block box.
    // style to use can be a pseudo element styles from any ancestor elements.
    // Note the :first-line, first-letter pseudo-elements can only be attached to a block container element.
    std::list<CSSStyleDeclarationImp*> firstLineStyles;
    std::list<CSSStyleDeclarationImp*> firstLetterStyles;
    Box* box = this;
    for (;;) {
        if (CSSStyleDeclarationImp* s = box->getStyle()) {
            if (CSSStyleDeclarationImp* p = s->getPseudoElementStyle(CSSPseudoElementSelector::FirstLine))
                firstLineStyles.push_front(p);
            if (isFirstLetter) {
                if (CSSStyleDeclarationImp* p = s->getPseudoElementStyle(CSSPseudoElementSelector::FirstLetter))
                    firstLetterStyles.push_front(p);
                if (s->getPseudoElementSelectorType() == CSSPseudoElementSelector::Marker)
                    isFirstLetter = false;
            }
        }
        Box* parent = box->getParentBox();
        if (!parent || parent->getFirstChild() != box)
            break;
        box = parent;
    }
    if (!firstLineStyles.empty()) {
        firstLineStyle = new(std::nothrow) CSSStyleDeclarationImp(CSSPseudoElementSelector::FirstLine);
        if (firstLineStyle) {
            for (auto i = firstLineStyles.begin(); i != firstLineStyles.end(); ++i)
                firstLineStyle->specify(*i);
            if (style->display.isInline()) {
                // 'style' should inherit properties from 'firstLineStyle'.
                // cf. 7.1.1. First formatted line definition in CSS - Selectors Level 3
                // cf. http://test.csswg.org/suites/css2.1/20110323/html4/first-line-pseudo-021.htm
                firstLineStyle->specifyWithoutInherited(style);
            }
            firstLineStyle->compute(view, getStyle(), 0);
            firstLineStyle->resolve(view, this);
        }
    }
    if (!firstLetterStyles.empty()) {
        firstLetterStyle = new(std::nothrow) CSSStyleDeclarationImp(CSSPseudoElementSelector::FirstLetter);
        if (firstLetterStyle) {
            for (auto i = firstLetterStyles.begin(); i != firstLetterStyles.end(); ++i)
                firstLetterStyle->specify(*i);
            if (style->display.isInline() && style->getPseudoElementSelectorType() == CSSPseudoElementSelector::NonPseudo)
                firstLetterStyle->specify(style);
            firstLetterStyle->compute(view, firstLineStyle.get() ? firstLineStyle.get() : style, 0);
            firstLetterStyle->resolve(view, this);
        }
    }
}

size_t Block::layOutFloatingFirstLetter(ViewCSSImp* view, FormattingContext* context, const std::u16string& data, CSSStyleDeclarationImp* firstLetterStyle)
{
    Document document = view->getDocument();
    if (!floatingFirstLetter)
        floatingFirstLetter = document.createElement(u"div");
    else {
        while (Node child = floatingFirstLetter.getFirstChild())
            floatingFirstLetter.removeChild(child);
    }
    size_t length = getfirstLetterLength(data, 0);  // TODO: position?
    Text text = document.createTextNode(data.substr(0, length));
    floatingFirstLetter.appendChild(text);
    Block* floatingBox = view->createBlock(floatingFirstLetter, this, firstLetterStyle, true);
    floatingBox->insertInline(text);
    floatingBox->flags &= ~(Box::NEED_EXPANSION | Box::NEED_CHILD_EXPANSION);
    addBlock(floatingFirstLetter, floatingBox);
    view->addStyle(floatingFirstLetter, firstLetterStyle);
    inlines.push_front(floatingFirstLetter);
    floatingBox->layOut(view, context);
    layOutFloat(view, floatingFirstLetter, floatingBox, context);
    return length;
}

float Block::measureText(ViewCSSImp* view, CSSStyleDeclarationImp* activeStyle,
                                 const char16_t* text, size_t length, float point, bool isFirstCharacter,
                                 FontGlyph*& glyph, std::u16string& transformed)
{
    FontTexture* font = activeStyle->getFontTexture();
    unsigned transform = activeStyle->textTransform.getValue();
    unsigned variant = activeStyle->fontVariant.getValue();
    float width = 0.0f;
    const char16_t* p = text;
    const char16_t* end = text + length;
    char32_t u;
    while (p < end && (p = utf16to32(p, &u)) && u) {
        if (u == '\n' || u == u'\u200B')
            continue;
        switch (transform) {
        case 1:  // capitalize
            if (u == u'\u00A0')  // NBSP
                isFirstCharacter = true;
            else if (isFirstCharacter && !u_ispunct(u)) {
                u = u_totitle(u);
                isFirstCharacter = false;
            }
            break;
        case 2:  // uppercase
            u = u_toupper(u);
            break;
        case 3:  // lowercase
            u = u_tolower(u);
            break;
        default:  // none
            break;
        }
        char32_t caps = u;
        if (variant == CSSFontVariantValueImp::SmallCaps)
            caps = u_toupper(u);
        FontTexture* currentFont = font;
        glyph = font->getGlyph(caps);
        if (font->isMissingGlyph(glyph)) {
            FontTexture* altFont = currentFont;
            while (altFont = activeStyle->getAltFontTexture(view, altFont, caps)) {
                FontGlyph* altGlyph = altFont->getGlyph(caps);
                if (!altFont->isMissingGlyph(altGlyph)) {
                    glyph = altGlyph;
                    currentFont = altFont;
                    break;
                }
            }
        }
        if (caps == u)
            width += glyph->advance * currentFont->getScale(point);
        else
            width += glyph->advance * currentFont->getScale(point) * currentFont->getSmallCapsScale();
        append(transformed, u);
        if (u == ' ' || u == u'\u00A0')  // SP or NBSP
            width += activeStyle->wordSpacing.getPx();
        if (!activeStyle->letterSpacing.isNormal())
            width += activeStyle->letterSpacing.getPx();
    }
    return width;
}

bool Block::layOutText(ViewCSSImp* view, Node text, FormattingContext* context,
                       std::u16string data, Element element, CSSStyleDeclarationImp* style)
{
    assert(element);
    assert(style);

    // An empty inline element should pass 'data' as an empty string. In this case,
    // the inline box to be generated must not be collapsed away by returning false.
    // cf. 10.8 Line height calculations: the ’line-height’ and ’vertical-align’ properties
    bool discardable = !data.empty();
    if (discardable && style->display.isInline()) {
        if (element.getFirstChild() == text && (style->marginLeft.getPx() || style->borderLeftWidth.getPx() || style->paddingLeft.getPx()) ||
            element.getLastChild() == text && (style->marginRight.getPx() || style->borderRightWidth.getPx() || style->paddingRight.getPx()))
            discardable = false;
    }

    if (style->processWhiteSpace(data, context->prevChar) == 0) {
        if (!context->atLineHead && style->whiteSpace.isBreakingLines())
            context->breakable = true;
        if (discardable)
            return !isAnonymous();
    }

    bool psuedoChecked = isAnonymous() && getParentBox()->getFirstChild() != this;
    CSSStyleDeclarationPtr firstLineStyle;
    CSSStyleDeclarationPtr firstLetterStyle;
    CSSStyleDeclarationImp* activeStyle;
    FontTexture* font;
    float point;
    activeStyle = setActiveStyle(view, style, font, point);

    size_t position = 0;  // within data
    InlineBox* inlineBox = 0;
    InlineBox* wrapBox = 0;    // characters moved to the next line
    for (;;) {
        if (context->atLineHead && !wrapBox) {
            size_t next = style->processLineHeadWhiteSpace(data, position);
            if (position != next && data.length() <= next && discardable)
                return (position == 0) ? !isAnonymous() : true;
            position = next;
        }

        context->useMargin(this);

        if (!context->lineBox) {
            if (!context->addLineBox(view, this))
                return false;  // TODO error
        }
        if (!psuedoChecked && getFirstChild() == context->lineBox) {
            psuedoChecked = true;
            getPsuedoStyles(view, context, style, firstLetterStyle, firstLineStyle);
            if (firstLetterStyle) {
                activeStyle = setActiveStyle(view, firstLetterStyle.get(), font, point);
                if (firstLetterStyle->isFloat()) {
                    position += layOutFloatingFirstLetter(view, context, data, firstLetterStyle.get());
                    if (data.length() <= position)
                        break;
                    nextLine(view, context, activeStyle, firstLetterStyle, firstLineStyle, style, false, font, point);
                    continue;
                }
            } else if (firstLineStyle)
                activeStyle = setActiveStyle(view, firstLineStyle.get(), font, point);
        }
        LineBox* lineBox = context->lineBox;

        float nbl = 0.0f;
        if (wrapBox) {
            for (InlineBox* box = wrapBox; box; box = dynamic_cast<InlineBox*>(box->getNextSibling()))
                nbl += box->getTotalWidth();
            context->x += nbl;
            context->leftover -= nbl;
            if (context->leftover < 0.0f && (lineBox->hasChildBoxes() || context->hasNewFloats())) {
                nextLine(view, context, activeStyle, firstLetterStyle, firstLineStyle, style, false, font, point);
                continue;
            }
        }

        if (!inlineBox) {
            inlineBox = new(std::nothrow) InlineBox(text, activeStyle);
            if (!inlineBox)
                return false;  // TODO error
            inlineBox->resolveWidth();
            if (0 < position || !inlineBox->isEmptyInlineAtFirst(style, element, text))
                inlineBox->clearBlankLeft();
        } else {
            inlineBox->setStyle(activeStyle);
            context->x += inlineBox->width;
            context->leftover -= inlineBox->width;
        }
        float blankLeft = inlineBox->getBlankLeft();
        float blankRight = inlineBox->getBlankRight();

        context->x += blankLeft;
        context->leftover -= blankLeft;

        bool linefeed = false;
        float advanced = 0.0f;

        if (data.empty()) {
            inlineBox->setData(font, point, data, 0, 0);
            if (!inlineBox->isEmptyInlineAtLast(style, element, text)) {
                inlineBox->clearBlankRight();
                blankRight = 0;
            }
        } else if (data[position] == '\n') {
            ++position;
            linefeed = true;
        } else {
            std::u16string inlineData;
            size_t fitLength = firstLetterStyle ? getfirstLetterLength(data, position) : (data.length() - position);
            // We are still not sure if there's a room for text in context->lineBox.
            // If there's no room due to float box(es), move the linebox down to
            // the closest bottom of float box.
            // And repeat this process until there's no more float box in the context.
            const char16_t* p = data.c_str() + position;
            size_t length = 0;  // of the new text segment
            size_t wrap = position;
            size_t next = position;
            float advanced = 0.0f;
            float wrapWidth = 0.0f;
            context->setText(p, fitLength);
            unsigned transform = activeStyle->textTransform.getValue();
            bool isFirstCharacter = wrapBox ? false : true;
            if (transform == CSSTextTransformValueImp::Capitalize) {
                if (!wrapBox && position == 0)
                    isFirstCharacter = context->isFirstCharacter(data);
            }
            do {
                wrap = next;
                wrapWidth = advanced;
                next = position + context->getNextTextBoundary();
                FontGlyph* glyph;
                std::u16string transformed;
                float w = measureText(view, activeStyle, p, next - wrap, point, isFirstCharacter, glyph, transformed);
                p += next - wrap;
                isFirstCharacter = true;
                if (firstLetterStyle || data.length() <= next && inlineBox->isEmptyInlineAtLast(style, element, text))
                    w += blankRight;
                while (context->leftover < w && (context->breakable || activeStyle->whiteSpace.isBreakingLines())) {
                    if (activeStyle->whiteSpace.isCollapsingSpace() && 0 < transformed.length() && transformed[transformed.length() - 1] == u' ') {
                        float lineEnd = (next - wrap == 1) ? 0.0f : w - glyph->advance * font->getScale(point) - activeStyle->wordSpacing.getPx();
                        if (!activeStyle->letterSpacing.isNormal())
                            lineEnd -= activeStyle->letterSpacing.getPx();
                        if (lineEnd == 0 || lineEnd <= context->leftover) {
                            context->breakable = true;
                            context->dontWrap();
                            w = lineEnd;
                            transformed.erase(transformed.length() - 1);
                            inlineData += transformed;
                            advanced += w;
                            context->leftover -= w;
                            wrap = length = next - position - 1;
                            updateMCW(nbl + w);
                            goto BreakLine;
                        }
                    }
                    // This text segment doesn't fit in the current line.
                    if (position < wrap) {
                        next = wrap;
                        goto BreakLine;
                    }
                    if (!wrapBox && position == 0) {
                        wrapBox = context->getWrapBox(data);
                        if (wrapBox) {
                            if (!wrapBox->getStyle()->whiteSpace.isBreakingLines())
                                break;
                            if (firstLineStyle) {
                                // If the current line is the first line, the style applied to the wrap-box has to be changed.
                                bool isFirstCharacter = true;
                                for (InlineBox* box = wrapBox; box; box = dynamic_cast<InlineBox*>(box->getNextSibling())) {
                                    Node node = box->getNode();
                                    CSSStyleDeclarationImp* wrapStyle = view->getStyle(interface_cast<Element>(node));
                                    if (!wrapStyle)
                                        wrapStyle = getStyle();
                                    FontTexture* font;
                                    float point;
                                    box->style = setActiveStyle(view, wrapStyle, font, point);
                                    FontGlyph* glyph;
                                    std::u16string transformed;
                                    // TODO: measureText using the original text data
                                    box->width = measureText(view, wrapStyle, box->getData().c_str(), box->getData().length(), point,
                                                             isFirstCharacter, glyph, transformed);
                                    box->data.clear();
                                    box->setData(font, point, transformed, 0, 0.0f);
                                    isFirstCharacter = false;
                                }
                            }
                        }
                    }
                    if (lineBox->hasChildBoxes() || context->hasNewFloats())
                        goto NextLine;
                    if (context->shiftDownLineBox(view)) {
                        if (wrapBox) {
                            nbl = 0.0f;
                            for (InlineBox* box = wrapBox; box; box = dynamic_cast<InlineBox*>(box->getNextSibling()))
                                nbl += box->getTotalWidth();
                            context->x += nbl;
                            context->leftover -= nbl;
                        }
                    } else {
                        inlineData += transformed;
                        advanced += w;
                        context->leftover -= w;
                        length = next - position;
                        context->breakable = false;
                        updateMCW(nbl + w);
                        goto BreakLine;
                    }
                }

                inlineData += transformed;
                advanced += w;
                context->leftover -= w;
                length = next - position;
                if (context->breakable || activeStyle->whiteSpace.isBreakingLines()) {
                    updateMCW(nbl + w);
                    nbl = 0.0f;
                } else
                    updateMCW(nbl + advanced);
                context->breakable = false;
                if (wrap < next && data[next - 1] == '\n') {
                    linefeed = true;
                    break;
                }
            } while (next < position + fitLength);
        BreakLine:
            inlineBox->setData(font, point, inlineData, wrap - position, wrapWidth);
            inlineBox->width += advanced;
            position = next;

            if (firstLetterStyle || data.length() <= position && inlineBox->isEmptyInlineAtLast(style, element, text))
                inlineBox->width -= blankRight;
            else {
                inlineBox->clearBlankRight();
                blankRight = 0;
            }
        }

        while (wrapBox) {
            InlineBox* next = dynamic_cast<InlineBox*>(wrapBox->getNextSibling());
            context->appendInlineBox(view, wrapBox, wrapBox->getStyle()); // TODO: leading, etc.
            wrapBox = next;
        }

        if (inlineBox->hasHeight()) {
            // Switch height from 'line-height' to the content height.
            inlineBox->height = font->getLineHeight(point);
            inlineBox->leading = activeStyle->lineHeight.getPx() - inlineBox->height;
            lineBox->underlinePosition = std::max(lineBox->underlinePosition, font->getUnderlinePosition(point));
            lineBox->underlineThickness = std::max(lineBox->underlineThickness, font->getUnderlineThickness(point));
            lineBox->lineThroughPosition = std::max(lineBox->lineThroughPosition, font->getLineThroughPosition(point));
            lineBox->lineThroughThickness = std::max(lineBox->lineThroughThickness, font->getLineThroughThickness(point));
        }
        context->x += advanced + blankRight;
        context->appendInlineBox(view, inlineBox, activeStyle);
        style->addBox(inlineBox);  // activeStyle? maybe not...
        if (data.length() <= position) {  // layout done?
            if (linefeed)
                context->nextLine(view, this, linefeed);
            break;
        }
        inlineBox = 0;
    NextLine:
        nextLine(view, context, activeStyle, firstLetterStyle, firstLineStyle, style, linefeed, font, point);
    }
    return true;
}

LineBox::LineBox(CSSStyleDeclarationImp* style) :
    Box(0),
    baseline(0.0f),
    underlinePosition(0.0f),
    underlineThickness(1.0f),
    lineThroughPosition(0.0f),
    lineThroughThickness(1.0f),
    leftGap(0.0f),
    rightGap(0.0f),
    rightBox(0)
{
    setStyle(style);
    // Keep 'height' 0.0f here since float and positioned elements are
    // also placed in line boxes in this implementation.
}

bool LineBox::layOut(ViewCSSImp* view, FormattingContext* context)
{
    for (Box* box = getFirstChild(); box; box = box->getNextSibling()) {
        if (box->isAbsolutelyPositioned())
            continue;
        if (InlineBox* inlineBox = dynamic_cast<InlineBox*>(box)) {
            CSSStyleDeclarationImp* style = box->getStyle();
            if (style && style->display.isInlineLevel())
                inlineBox->offsetV = style->verticalAlign.getOffset(view, style, this, inlineBox);
            else {
                float leading = inlineBox->getLeading() / 2.0f;
                inlineBox->offsetV = getBaseline() - (leading + inlineBox->getBaseline());
            }
        }
    }
    return true;
}

float LineBox::shrinkTo()
{
    float w = Box::shrinkTo();

    float wl = 0.0f;
    float l = 0.0f;
    for (auto child = getFirstChild(); child; child = child->getNextSibling()) {
        if (child->isAnonymous() || !child->style)
            break;
        if (child->style->float_.getValue() == CSSFloatValueImp::Left) {
            float e = child->getEffectiveTotalWidth();
            wl += e;
            l += e;
            if (e == 0.0f)
                l = std::max(wl + child->getTotalWidth(), l);
        } else
            break;
    }

    float wr = 0.0f;
    float r = 0.0f;
    for (auto child = getLastChild(); child; child = child->getPreviousSibling()) {
        if (child->isAnonymous() || !child->style)
            break;
        if (child->style->float_.getValue() == CSSFloatValueImp::Right) {
            float e = child->getEffectiveTotalWidth();
            wr += e;
            r += e;
            if (e == 0.0f)
                r = std::max(wr + child->getTotalWidth(), r);
        } else
            break;
    }

    w += wl + wr;
    return std::max(w, std::max(l, r));
}

void LineBox::fit(float w, FormattingContext* context)
{
    assert(parentBox);
    assert(dynamic_cast<Block*>(parentBox));
    float leftover = std::max(0.0f, w - shrinkTo());
    switch (dynamic_cast<Block*>(parentBox)->getTextAlign()) {
    case CSSTextAlignValueImp::Left:
    case CSSTextAlignValueImp::Default: // TODO: rtl
        leftGap = 0.0f;
        rightGap = leftover;
        break;
    case CSSTextAlignValueImp::Right:
        leftGap = leftover;
        rightGap = 0.0f;
        break;
    case CSSTextAlignValueImp::Center:
        leftGap = rightGap = leftover / 2.0f;
        break;
    default:  // TODO: support Justify and Default
        break;
    }
}

void LineBox::resolveXY(ViewCSSImp* view, float left, float top, Block* clip)
{
    left += offsetH;
    top += offsetV + getClearance();
    x = left;
    y = top;
    clipBox = clip;
    left += getBlankLeft();  // Node floats are placed inside margins.
    top += getBlankTop();
    float next = 0.0f;
    bool usedLeftGap = false;
    for (auto child = getFirstChild(); child; child = child->getNextSibling()) {
        Block* floatingBox = 0;
        next = left;
        if (!child->isAbsolutelyPositioned()) {
            if (!child->isFloat())
                next += child->getTotalWidth();
            else {
                floatingBox = dynamic_cast<Block*>(child);
                assert(floatingBox);
                if (floatingBox == rightBox)
                    break;
                next = left + floatingBox->getEffectiveTotalWidth();
            }
        }
        if (!usedLeftGap && !floatingBox) {
            left += leftGap;
            next += leftGap;
            usedLeftGap = true;
        }
        child->resolveXY(view, left, top, clip);
        left = next;
    }
    if (rightBox) {
        float right = x + getParentBox()->width - getBlankRight();
        for (auto child = getLastChild(); child; child = child->getPreviousSibling()) {
            Block* floatingBox = dynamic_cast<Block*>(child);
            right -= floatingBox->getEffectiveTotalWidth();
            child->resolveXY(view, right, top, clip);
            if (floatingBox == rightBox)
                break;
        }
    }
}

void LineBox::dump(std::string indent)
{
    float relativeX = 0.0f;
    float relativeY = 0.0f;
    resolveRelativeOffset(relativeX, relativeY);
    std::cout << indent << "* line box";
    if (3 <= getLogLevel())
        std::cout << " [" << std::hex << flags << ']' << std::dec;
    std::cout << " (" << x + relativeX << ", " << y + relativeY << ") " <<
        "w:" << width << " h:" << height << " (" << relativeX << ", " << relativeY <<") ";
    if (hasClearance())
        std::cout << "c:" << clearance << ' ';
    std::cout << "m:" << marginTop << ':' << marginRight << ':' << marginBottom << ':' << marginLeft << '\n';
    indent += "  ";
    for (Box* child = getFirstChild(); child; child = child->getNextSibling())
        child->dump(indent);
}

InlineBox::InlineBox(Node node, CSSStyleDeclarationImp* style) :
    Box(node),
    font(0),
    point(0.0f),
    baseline(0.0f),
    leading(0.0f),
    wrap(0),
    wrapWidth(0.0f),
    emptyInline(0)
{
    if (style) {
        setStyle(style);
        visibility = style->visibility.getValue();
    }
}

bool InlineBox::isEmptyInlineAtFirst(CSSStyleDeclarationImp* style, Element& element, Node& node)
{
    if (element != node)
        return (element.getFirstChild() == node) && !(style->getEmptyInline() & 1);
    if (!emptyInline)
        emptyInline = style->checkEmptyInline();
    return (emptyInline & 1) || emptyInline == 4;
}

bool InlineBox::isEmptyInlineAtLast(CSSStyleDeclarationImp* style, Element& element, Node& node)
{
    if (element != node)
        return (element.getLastChild() == node) && !(style->getEmptyInline() & 2);
    if (!emptyInline)
        emptyInline = style->checkEmptyInline();
    return !(emptyInline & 1) && ((emptyInline & 2) || emptyInline == 4);
}

void InlineBox::setData(FontTexture* font, float point, const std::u16string& data, size_t wrap, float wrapWidth)
{
    assert(data[0] != 0 || data.empty());
    this->font = font;
    this->point = point;
    if (this->data.empty()) {
        this->wrap = wrap;
        this->wrapWidth = wrapWidth;
    } else {
        this->wrap = this->data.length() + wrap;
        this->wrapWidth = this->width + wrapWidth;
    }
    this->data += data;
    baseline = font->getAscender(point);
    if (0 < this->data.length() && this->data[this->data.length() - 1] == u' ')
        this->wrap = this->data.length();
}

InlineBox* InlineBox::split()
{
    assert(wrap < data.length());
    InlineBox* wrapBox = new(std::nothrow) InlineBox(node, getStyle());
    if (!wrapBox)
        return 0;
    wrapBox->marginTop = marginTop;
    wrapBox->marginRight = marginRight;
    wrapBox->marginBottom = marginBottom;
    wrapBox->paddingTop = paddingTop;
    wrapBox->paddingRight = paddingRight;
    wrapBox->paddingBottom = paddingBottom;
    wrapBox->borderTop = borderTop;
    wrapBox->borderRight = borderRight;
    wrapBox->borderBottom = borderBottom;
    wrapBox->visibility = visibility;
    wrapBox->setData(font, point, data.substr(wrap), data.length() - wrap, 0.0f);
    wrapBox->width = width - wrapWidth;
    wrapBox->wrap = 0;
    wrapBox->wrapWidth = 0.0f;
    clearBlankRight();
    data.erase(wrap);
    wrap = data.length();
    width = wrapWidth;
    return wrapBox;
}

float InlineBox::atEndOfLine()
{
    size_t length = data.length();
    if (length < 1)
        return 0.0f;
    if (style->whiteSpace.isCollapsingSpace() && data[length - 1] == u' ') {
        data.erase(length - 1);
        if (data.length() == 0) {
            // Deal with the errors in floating point operations.
            float w = -width;
            width = 0.0f;
            return w;
        } else {
            float w = -font->measureText(u" ", point) - getStyle()->wordSpacing.getPx();
            if (!getStyle()->letterSpacing.isNormal())
                w -= getStyle()->letterSpacing.getPx();
            width += w;
            return w;
        }
    }
    return 0.0f;
}

float InlineBox::getSub() const
{
    if (!font)
        return 0.0f;
    return font->getSub(point);
}

float InlineBox::getSuper() const
{
    if (!font)
        return 0.0f;
    return font->getSuper(point);
}

void InlineBox::resolveWidth()
{
    // The ‘width’ and ‘height’ properties do not apply.
    if (isInline()) {
        backgroundColor = style->backgroundColor.getARGB();
        updatePadding();
        updateBorderWidth();
        marginTop = style->marginTop.isAuto() ? 0 : style->marginTop.getPx();
        marginRight = style->marginRight.isAuto() ? 0 : style->marginRight.getPx();
        marginLeft = style->marginLeft.isAuto() ? 0 : style->marginLeft.getPx();
        marginBottom = style->marginBottom.isAuto() ? 0 : style->marginBottom.getPx();
    } else {
        backgroundColor = 0x00000000;
        paddingTop = paddingRight = paddingBottom = paddingLeft = 0.0f;
        borderTop = borderRight = borderBottom = borderLeft = 0.0f;
        marginTop = marginRight = marginLeft = marginBottom = 0.0f;
    }
}

// To deal with nested inline elements in the document tree, resolveRelativeOffset
// is repeatedly applied to this inline level box up to a non-inline element.
void InlineBox::resolveRelativeOffset(float& x, float &y)
{
    CSSStyleDeclarationImp* s = getStyle();
    if (!font) {
        Box::resolveRelativeOffset(x, y);
        s = s->getParentStyle();
    }
    while (s && s->display.isInline()) {
        s->resolveRelativeOffset(x, y);
        s = s->getParentStyle();
    }
}

void InlineBox::resolveXY(ViewCSSImp* view, float left, float top, Block* clip)
{
    left += offsetH;
    top += offsetV + leading / 2.0f;
    if (!childWindow && getFirstChild())
        getFirstChild()->resolveXY(view, left + getBlankLeft(), top + getBlankTop(), clip);
    x = left;
    y = top;
    clipBox = clip;

    if (isPositioned()) {
        assert(getStyle());
        getStyle()->getStackingContext()->setClipBox(clipBox);
    }
}

void InlineBox::dump(std::string indent)
{
    std::cout << indent << "* inline-level box";
    if (3 <= getLogLevel())
        std::cout << " [" << std::hex << flags << ']' << std::dec;
    std::cout << " (" << x << ", " << y << ") " <<
        "w:" << width << " h:" << height << ' ' <<
        "m:" << marginTop << ':' << marginRight << ':' << marginBottom << ':' << marginLeft << ' ' <<
        "p:" << paddingTop << ':' <<  paddingRight << ':'<< paddingBottom<< ':' << paddingLeft << ' ' <<
        "b:" << borderTop << ':' <<  borderRight << ':' << borderBottom<< ':' << borderLeft << ' ' <<
        '"' << data << "\" ";
    if (getStyle())
        std::cout << std::hex << CSSSerializeRGB(getStyle()->color.getARGB()) << std::dec;
    std::cout << '\n';
    indent += "  ";
    for (Box* child = getFirstChild(); child; child = child->getNextSibling())
        child->dump(indent);
}

}}}}  // org::w3c::dom::bootstrap
