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

#include <stdio.h>
#include "esnpapi.h"

namespace
{

NPObject* stubAllocate(NPP npp, NPClass* aClass)
{
    return new StubObject(npp);
}

void stubDeallocate(NPObject* object)
{
    printf("%s\n", __func__);
    delete static_cast<StubObject*>(object);
}

void stubInvalidate(NPObject* object)
{
    printf("%s\n", __func__);
    return static_cast<StubObject*>(object)->invalidate();
}

bool stubHasMethod(NPObject* object, NPIdentifier name)
{
    return static_cast<StubObject*>(object)->hasMethod(name);
}

bool stubInvoke(NPObject* object, NPIdentifier name, const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    return static_cast<StubObject*>(object)->invoke(name, args, argCount, result);
}

bool stubInvokeDefault(NPObject* object, const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    return static_cast<StubObject*>(object)->invokeDefault(args, argCount, result);
}

bool stubHasProperty(NPObject* object, NPIdentifier name)
{
    return static_cast<StubObject*>(object)->hasProperty(name);
}

bool stubGetProperty(NPObject* object, NPIdentifier name, NPVariant* result)
{
    return static_cast<StubObject*>(object)->getProperty(name, result);
}

bool stubSetProperty(NPObject* object, NPIdentifier name, const NPVariant* value)
{
    return static_cast<StubObject*>(object)->setProperty(name, value);
}

bool stubRemoveProperty(NPObject* object, NPIdentifier name)
{
    return static_cast<StubObject*>(object)->removeProperty(name);
}

bool stubEnumeration(NPObject* object, NPIdentifier** value, uint32_t* count)
{
    return static_cast<StubObject*>(object)->enumeration(value, count);
}

bool stubConstruct(NPObject* object, const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    return static_cast<StubObject*>(object)->construct(args, argCount, result);
}

unsigned lookupSymbolTalbe(Object* object, const char* identifier, unsigned& interfaceNumber, unsigned& symbolNumber)
{
    const Reflect::SymbolData* symbolTable;
    while (symbolTable = object->getSymbolTable(interfaceNumber))
    {
        unsigned offset = 0;
        for (symbolTable += symbolNumber; symbolTable->symbol; ++symbolNumber, ++symbolTable)
        {
            if (!std::strcmp(symbolTable->symbol, identifier))
            {
                return symbolTable->offset;
            }
        }
        ++interfaceNumber;
    }
    return 0;
}

}   // namespace

long StubObject::enter()
{
    PluginInstance* instance = static_cast<PluginInstance*>(npp->pdata);
    if (instance)
    {
        ProxyControl* proxyControl = instance->getProxyControl();
        if (proxyControl)
        {
            return proxyControl->enter();
        }
    }
    return 0;
}

long StubObject::leave()
{
    PluginInstance* instance = static_cast<PluginInstance*>(npp->pdata);
    if (instance)
    {
        ProxyControl* proxyControl = instance->getProxyControl();
        if (proxyControl)
        {
            return proxyControl->leave();
        }
    }
    return 0;
}

void StubObject::invalidate()
{
}

bool StubObject::hasMethod(NPIdentifier name)
{
    NPUTF8* identifier = NPN_UTF8FromIdentifier(name);
    if (!identifier)
    {
        return false;
    }

    printf("%s(%s)\n", __func__, identifier);

    bool found = false;
    unsigned interfaceNumber = 0;
    unsigned symbolNumber = 0;
    for (;; ++symbolNumber)
    {
        unsigned offset = lookupSymbolTalbe(object, identifier, interfaceNumber, symbolNumber);
        if (!offset)
        {
            break;
        }
        const char* metaData = object->getMetaData(interfaceNumber);
        metaData += offset;
        if (*metaData == Reflect::kOperation)
        {
            found = true;
            break;
        }
    }
    NPN_MemFree(identifier);
    return found;
}

bool StubObject::invoke(NPIdentifier name, const NPVariant* args, uint32_t arg_count, NPVariant* result)
{
    NPUTF8* identifier = NPN_UTF8FromIdentifier(name);
    if (!identifier)
    {
        return false;
    }

    printf("%s(%s)\n", __func__, identifier);

    bool found = false;
    unsigned interfaceNumber = 0;
    unsigned symbolNumber = 0;
    for (;; ++symbolNumber)
    {
        unsigned offset = lookupSymbolTalbe(object, identifier, interfaceNumber, symbolNumber);
        if (!offset)
        {
            break;
        }
        const char* metaData = object->getMetaData(interfaceNumber);
        metaData += offset;
        if (*metaData == Reflect::kOperation)
        {
            Reflect::Method method(metaData);
            unsigned argumentCount = method.getParameterCount();
            // TODO: Support variadic operation
            if (argumentCount != arg_count)
            {
                continue;
            }
            enter();
            Any arguments[argumentCount];
            for (unsigned i = 0; i < argumentCount; ++i)
            {
                arguments[i] = convertToAny(npp, &args[i]);  // TODO: Use , type)
            }
            Any value = object->call(interfaceNumber, symbolNumber, argumentCount, arguments);
            convertToVariant(npp,value, result);
            found = true;
            leave();
            break;
        }
    }
    NPN_MemFree(identifier);
    return found;
}

bool StubObject::invokeDefault(const NPVariant* args, uint32_t arg_count, NPVariant* result)
{
    printf("%s()\n", __func__);
    return false;
}

bool StubObject::hasProperty(NPIdentifier name)
{
    NPUTF8* identifier = NPN_UTF8FromIdentifier(name);
    if (!identifier)
    {
        return false;
    }

    printf("%s(%s)\n", __func__, identifier);

    bool found = false;
    unsigned interfaceNumber = 0;
    unsigned symbolNumber = 0;
    for (;; ++symbolNumber)
    {
        unsigned offset = lookupSymbolTalbe(object, identifier, interfaceNumber, symbolNumber);
        if (!offset)
        {
            break;
        }
        const char* metaData = object->getMetaData(interfaceNumber);
        metaData += offset;
        if (*metaData == Reflect::kConstant || *metaData == Reflect::kGetter || *metaData == Reflect::kSetter)
        {
            found = true;
            break;
        }
    }
    NPN_MemFree(identifier);
    return found;
}

bool StubObject::getProperty(NPIdentifier name, NPVariant* result)
{
    NPUTF8* identifier = NPN_UTF8FromIdentifier(name);
    if (!identifier)
    {
        return false;
    }

    printf("%s(%s)\n", __func__, identifier);

    bool found = false;
    unsigned interfaceNumber = 0;
    unsigned symbolNumber = 0;
    for (;; ++symbolNumber)
    {
        unsigned offset = lookupSymbolTalbe(object, identifier, interfaceNumber, symbolNumber);
        if (!offset)
        {
            break;
        }
        const char* metaData = object->getMetaData(interfaceNumber);
        metaData += offset;
        if (*metaData == Reflect::kConstant)
        {
            // TODO: eval the constant value
            found = true;
            break;
        }
        if (*metaData == Reflect::kGetter)
        {
            enter();
            Any property = object->call(interfaceNumber, symbolNumber, 0, 0);
            convertToVariant(npp, property, result);
            found = true;
            leave();
            break;
        }
    }
    NPN_MemFree(identifier);
    return found;
}

bool StubObject::setProperty(NPIdentifier name, const NPVariant* value)
{
    NPUTF8* identifier = NPN_UTF8FromIdentifier(name);
    if (!identifier)
    {
        return false;
    }

    printf("%s(%s)\n", __func__, identifier);

    bool found = false;
    unsigned interfaceNumber = 0;
    unsigned symbolNumber = 0;
    for (;; ++symbolNumber)
    {
        unsigned offset = lookupSymbolTalbe(object, identifier, interfaceNumber, symbolNumber);
        if (!offset)
        {
            break;
        }
        const char* metaData = object->getMetaData(interfaceNumber);
        metaData += offset;
        if (*metaData == Reflect::kSetter)
        {
            enter();
            Any argument = convertToAny(npp, value);  // TODO: Use , type)
            object->call(interfaceNumber, symbolNumber, 1, &argument);
            found = true;
            leave();
            break;
        }
    }
    NPN_MemFree(identifier);
    return found;
}

bool StubObject::removeProperty(NPIdentifier name)
{
    return false;
}

bool StubObject::enumeration(NPIdentifier** value, uint32_t* count)
{
    return false;
}

bool StubObject::construct(const NPVariant* args, uint32_t argCount, NPVariant* result)
{
    return false;
}

NPClass StubObject::npclass =
{
    NP_CLASS_STRUCT_VERSION_CTOR,
    stubAllocate,
    stubDeallocate,
    stubInvalidate,
    stubHasMethod,
    stubInvoke,
    stubInvokeDefault,
    stubHasProperty,
    stubGetProperty,
    stubSetProperty,
    stubRemoveProperty,
    stubEnumeration,
    stubConstruct
};

StubControl::StubControl(NPP npp) :
    npp(npp)
{
}

StubControl::~StubControl()
{
    // TODO: Invalidate stubs in stubMap
}

NPObject* StubControl::createStub(Object* object)
{
    NPObject* npobject = NPN_CreateObject(npp, &StubObject::npclass);
    if (!npobject)
    {
        return 0;
    }
    StubObject* stub = static_cast<StubObject*>(npobject);
    stub->setObject(object);
    return stub;
}
