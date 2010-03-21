/*
 * Copyright 2008-2010 Google Inc.
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

#include "esnpapi.h"
#include "proxyImpl.h"

#include <any.h>
#include <reflect.h>
#include <org/w3c/dom.h>

using namespace org::w3c::dom;

void initializeSvgMetaDataH_N()
{
    registerMetaData(svg::SVGHKernElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGHKernElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGICCColor::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGICCColor_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGImageElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGImageElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLangSpace::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLangSpace_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLength::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLength_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLengthList::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLengthList_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLinearGradientElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLinearGradientElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLineElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLineElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGLocatable::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGLocatable_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMarkerElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMarkerElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMaskElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMaskElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMatrix::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMatrix_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMetadataElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMetadataElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMissingGlyphElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMissingGlyphElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGMPathElement::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGMPathElement_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGNumber::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGNumber_Bridge<Any, invoke> >::createInstance));
    registerMetaData(svg::SVGNumberList::getMetaData(), reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, svg::SVGNumberList_Bridge<Any, invoke> >::createInstance));
}
