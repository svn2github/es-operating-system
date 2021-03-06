/*
 * Copyright 2011, 2012 Esrille Inc.
 * Copyright 2008-2010 Google Inc.
 * Copyright 2006, 2007 Nintendo Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF Any KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ESIDL_REFLECT_H_INCLUDED
#define ESIDL_REFLECT_H_INCLUDED

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <string>

#include <Any.h>

/**
 * This class provides access to the string-encoded interface meta-data.
 *
 *  interface ->
 *    I constructor? name
 *    extends?
 *    [ operation, property, constant ]*
 *
 *  operation -> F special* name
 *  attribute -> A special* name
 *  constant  -> K name type value ' '
 *
 *  extends -> X name
 *  constructor -> c
 *  special ->
 *    r: read-only
 *    g: getter
 *    s: setter
 *    c: creator
 *    d: deleter
 *    f: caller
 *    t: stringifier
 *    o: omittable
 *    i: static
 *
 *  type ->
 *    b: boolean
 *    g: byte
 *    h: octet, unsigned byte
 *    s: short
 *    t: unsigned short
 *    l: long
 *    m: unsigned long
 *    x: long long
 *    y: unsigned long long
 *    f: float
 *    d: double
 *
 */
class Reflect
{
public:
    static const char kInterface = 'I';
    static const char kExtends = 'X';
    static const char kOperation = 'F';
    static const char kAttribute = 'A';
    static const char kConstant = 'K';

    static const char kConstructor = 'c';
    static const char kReadOnly = 'r';
    static const char kGetter = 'g';
    static const char kSetter = 's';
    static const char kCreator = 'c';
    static const char kDeleter = 'd';
    static const char kCaller = 'f';
    static const char kStringifier = 't';
    static const char kOmittable = 'o';
    static const char kStatic = 'i';

    static const char kBoolean = 'b';
    static const char kByte = 'g';
    static const char kUnsignedByte = 'h';
    static const char kOctet = 'h';
    static const char kShort = 's';
    static const char kUnsignedShort = 't';
    static const char kLong = 'l';
    static const char kUnsignedLong = 'm';
    static const char kLongLong = 'x';
    static const char kUnsignedLongLong = 'y';
    static const char kFloat = 'f';
    static const char kDouble = 'd';

    class Interface;

    static const char* skipDigits(const char* info, unsigned* value = 0)
    {
        unsigned n = 0;
        while (std::isdigit(*info))
        {
            n = 10 * n + (*info - '0');
            ++info;
        }
        if (value)
        {
            *value = n;
        }
        return info;
    }

    static const char* skipName(const char* info)
    {
        unsigned length;
        info = skipDigits(info, &length);
        return info + length;
    }

    static const char* skipSpecials(const char* info)
    {
        while (*info && !std::isdigit(*info))
        {
            ++info;
        }
        return info;
    }

    // Skip constatnt type and value
    static const char* skipConstantValue(const char* value)
    {
        // TODO: Fix this when DOMString constant is to be supported.
        return std::strchr(value, ' ') + 1;
    }

    static const char* skipProperty(const char* info)
    {
        const char* next = skipName(skipSpecials(info + 1));
        if (*info != kConstant)
        {
            return next;
        }
        return skipConstantValue(next);
    }

    /**
      * Gets the identifier of a prefixed name.
      */
    static std::string getIdentifier(const std::string& name)
    {
        size_t pos = name.rfind(':');
        if (pos == name.npos)
        {
            return name;
        }
        return name.substr(pos + 1);
    }

    /**
      * Gets the module name of a prefixed name.
      */
    static std::string getModuleName(const std::string& name)
    {
        size_t pos = name.rfind(':');
        if (pos == name.npos)
        {
            return "";
        }
        return name.substr(0, pos + 1);
    }

    /**
     * This represents a property (i.e., operation, attribute, or constant) loaded from the specified reflection data.
     */
    class Property
    {
    protected:
        const char* info;

    public:
        /** Default constructor
         */
        Property() :
            info(0)
        {
        }

        /**
         * Constructs an object which represents the specified operation.
         * @param info the string encoded reflection data generated by esidl.
         */
        Property(const char* info) :
            info(info)
        {
        }

        /**
         * Copy-constructor.
         */
        Property(const Property& property) :
            info(property.info)
        {
        }

        Property& next()
        {
            info = skipProperty(info);
            return *this;
        }

        /**
         * Gets the type of this property.
         */
        char getType() const
        {
            return *info;
        }

        bool isInterface() const
        {
            return *info == kInterface;
        }

        bool isExtends() const
        {
            return *info == kExtends;
        }

        bool isOperation() const
        {
            return *info == kOperation;
        }

        bool isAttribute() const
        {
            return *info == kAttribute;
        }

        bool isConstant() const
        {
            return *info == kConstant;
        }

        /**
         * Gets the name of this property.
         */
        const std::string getName() const
        {
            unsigned length;
            const char* name = skipDigits(skipSpecials(info + 1), &length);
            return std::string(name, length);
        }

        bool hasSpecial(char special) const
        {
            for (const char* p = info + 1; *p && !std::isdigit(*p); ++p)
            {
                if (*p == special)
                {
                    return true;
                }
            }
            return false;
        }

        bool isReadOnly() const
        {
            return hasSpecial(kReadOnly);
        }

        bool isGetter() const
        {
            return hasSpecial(kGetter);
        }

        bool isSetter() const
        {
            return hasSpecial(kSetter);
        }

        bool isCreator() const
        {
            return hasSpecial(kCreator);
        }

        bool isDeleter() const
        {
            return hasSpecial(kDeleter);
        }

        bool isCaller() const
        {
            return hasSpecial(kCaller);
        }

        bool isStringifier() const
        {
            return hasSpecial(kStringifier);
        }

        bool isOmittable() const
        {
            return hasSpecial(kOmittable);
        }

        bool isStatic() const
        {
            return hasSpecial(kStatic);
        }

        bool hasConstructor() const
        {
            return hasSpecial(kConstructor);
        }

        /**
         * Gets the value of this constant.
         */
        Any getValue() const
        {
            if (*info != kConstant)
                return Any();
            const char* k = skipName(skipSpecials(info + 1));
            switch (*k) {
            case kBoolean:
                return static_cast<bool>(std::strtol(k + 1, 0, 10));
            case kByte:
                return static_cast<int8_t>(std::strtol(k + 1, 0, 10));
            case kUnsignedByte:
                return static_cast<uint8_t>(std::strtoul(k + 1, 0, 10));
            case kShort:
                return static_cast<int16_t>(std::strtol(k + 1, 0, 10));
            case kUnsignedShort:
                return static_cast<uint16_t>(std::strtoul(k + 1, 0, 10));
            case kLong:
                return static_cast<int32_t>(std::strtol(k + 1, 0, 10));
            case kUnsignedLong:
                return static_cast<uint32_t>(std::strtoul(k + 1, 0, 10));
            case kLongLong:
                return static_cast<int64_t>(std::strtoll(k + 1, 0, 10));
            case kUnsignedLongLong:
                return static_cast<uint64_t>(std::strtoull(k + 1, 0, 10));
            case kFloat:
                return std::strtof(k + 1, 0);
            case kDouble:
                return std::strtod(k + 1, 0);
            default:
                return Any();
            }
        }
    };

    /**
     * This represents an interface loaded from the specified reflection data.
     */
    class Interface : public Property
    {
        size_t operationCount;
        size_t attributeCount;
        size_t constantCount;
        size_t stringSize;

    public:
        /**
         * Default constructor.
         */
        Interface() :
            operationCount(0),
            attributeCount(0),
            constantCount(0),
            stringSize(0)
        {
        }

        /**
         * Constructs a new object which represents the specified interface.
         * @param info the string encoded reflection data generated by esidl.
         */
        Interface(const char* info) :
            Property(info),
            operationCount(0),
            attributeCount(0),
            constantCount(0),
            stringSize(0)
        {
            const char* p = skipProperty(info);  // skip I
            while (*p)
            {
                char type = *p;
                switch (type)
                {
                case kConstant:
                    ++constantCount;
                    break;
                case kOperation:
                    ++operationCount;
                    break;
                case kAttribute:
                    ++attributeCount;
                    break;
                default:
                    break;
                }
                // Do the same as "p = skipProperty(p);" while summing up the string lengths.
                p = skipSpecials(p + 1);
                unsigned length;
                p = skipDigits(p, &length);
                p += length;
                stringSize += length + 1;
                if (type == kConstant)
                {
                    p = skipConstantValue(p);
                }
            }
        }

        /**
         * Copy-constructor.
         */
        Interface(const Interface& interface) :
            Property(interface.info),
            operationCount(interface.operationCount),
            attributeCount(interface.attributeCount),
            constantCount(interface.constantCount),
            stringSize(interface.stringSize)
        {
        }

        /**
         * Gets the name of the base interface.
         */
        std::string getExtends() const
        {
            Property extends(skipProperty(info));
            if (!extends.isExtends())
            {
                return "";
            }
            return extends.getName();
        }

        /**
         * Gets the number of operations in this interface.
         * @return the operation count excluding base interface operations.
         */
        size_t getOperationCount() const
        {
            return operationCount;
        }

        /**
         * Gets the number of attributes in this interface.
         * @return the attribute count excluding base interface attributes.
         */
        size_t getAttributeCount() const
        {
            return attributeCount;
        }

        /**
         * Gets the number of constants in this interface.
         */
        size_t getConstantCount() const
        {
            return constantCount;
        }

        size_t getStringSize() const
        {
            return stringSize;
        }
    };
};

#endif  // ESIDL_REFLECT_H_INCLUDED
