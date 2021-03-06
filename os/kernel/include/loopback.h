/*
 * Copyright 2008, 2009 Google Inc.
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NINTENDO_ES_KERNEL_LOOPBACK_H_INCLUDED
#define NINTENDO_ES_KERNEL_LOOPBACK_H_INCLUDED

#include <es.h>
#include <es/ref.h>
#include <es/ring.h>
#include <es/base/IStream.h>
#include <es/device/INetworkInterface.h>
#include "thread.h"

class Loopback : public es::NetworkInterface, public es::Stream
{
    Ref     ref;
    Monitor monitor;
    Ring    ring;
    long    ringSize;

public:
    Loopback(void* buf, long size) :
        ring(buf, size), ringSize(size)
    {
    }
    ~Loopback()
    {
    }

    // INetworkInterface
    int addMulticastAddress(const unsigned char* mac)
    {
        return 0;
    }
    int getMacAddress(unsigned char* mac)
    {
        memset(mac, 0, 6);
        return 6;
    }
    int getMTU()
    {
        return 1500;
    }
    bool getLinkState()
    {
        return true;
    }
    bool getPromiscuousMode()
    {
        return true;
    }
    unsigned long long getInOctets()
    {
        return 0;
    }
    unsigned int getInUcastPkts()
    {
        return 0;
    }
    unsigned int getInNUcastPkts()
    {
        return 0;
    }
    unsigned int getInDiscards()
    {
        return 0;
    }
    unsigned int getInErrors()
    {
        return 0;
    }
    unsigned int getInUnknownProtos()
    {
        return 0;
    }
    unsigned long long getOutOctets()
    {
        return 0;
    }
    unsigned int getOutUcastPkts()
    {
        return 0;
    }
    unsigned int getOutNUcastPkts()
    {
        return 0;
    }
    unsigned int getOutDiscards()
    {
        return 0;
    }
    unsigned int getOutErrors()
    {
        return 0;
    }
    unsigned int getOutCollisions()
    {
        return 0;
    }
    int getType()
    {
        return es::NetworkInterface::Loopback;
    }
    int removeMulticastAddress(const unsigned char* mac)
    {
        return 0;
    }
    void setPromiscuousMode(bool on)
    {
    }
    int start()
    {
    }
    int stop()
    {
    }

    // IStream
    long long getPosition();
    void setPosition(long long pos);
    long long getSize();
    void setSize(long long size);
    int read(void* dst, int count);
    int read(void* dst, int count, long long offset);
    int write(const void* src, int count);
    int write(const void* src, int count, long long offset);
    void flush();

    // IInterface
    Object* queryInterface(const char* riid);
    unsigned int addRef();
    unsigned int release();
};

#endif  // NINTENDO_ES_KERNEL_LOOPBACK_H_INCLUDED
