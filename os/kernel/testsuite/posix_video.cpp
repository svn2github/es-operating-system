/*
 * Copyright 2008 Chis Dan Ionut
 * Copyright 2008 Google Inc.
 * Copyright 2006 Nintendo Co., Ltd.
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

#include <es.h>
#include <es/handle.h>
#include <es/naming/IContext.h>
#include <es/device/ICursor.h>
#include "posix/core.h"

#define TEST(exp)                           \
    (void) ((exp) ||                        \
            (esPanic(__FILE__, __LINE__, "\nFailed test " #exp), 0))

#define BITS_PER_PIXEL 32
#define BPP (BITS_PER_PIXEL / 8)

u8 pixels[1024 * 768 * BPP];

void fill(IStream* fb, u8 r, u8 g, u8 b)
{
    for (long offset = 0; offset < 1024 * 768 * BPP; offset += 1024 * 768)
    {
        for (long j = offset; j < 1024 * 768; j+=BPP)
        {
            pixels[j] = b;
            pixels[j + 1] = g;
            pixels[j + 2] = r;
        }
        fb->write(pixels, 1024 * 768, offset);
    }
}

void pattern(IStream* fb)
{
    long offset;
    for (offset = 0; offset < 1024 * 256 * BPP; offset += BPP)
    {
        pixels[offset] = 255;
        pixels[offset + 1] = 0;
        pixels[offset + 2] = 0;
    }
    fb->write(pixels, 1024 * 256 * BPP, 0);

    for (; offset < 1024 * 512 * BPP; offset += BPP)
    {
        pixels[offset] = 0;
        pixels[offset + 1] = 255;
        pixels[offset + 2] = 0;
    }
    fb->write(pixels + 1024 * 256 * BPP, 1024 * 256 * BPP, 1024 * 256 * BPP);

    for (; offset < 1024 * 768 * BPP; offset += BPP)
    {
        pixels[offset] = 0;
        pixels[offset + 1] = 0;
        pixels[offset + 2] = 255;
    }
    fb->write(pixels + 1024 * 512 * BPP, 1024 * 256 * BPP, 1024 * 512 * BPP);
}

int main()
{
    IInterface* nameSpace;
    esInit(&nameSpace);

    Handle<IContext> root(nameSpace);
    Handle<IStream> mouse(root->lookup("device/mouse"));
    Handle<ICursor> cursor(root->lookup("device/cursor"));
    Handle<IStream> framebuffer(root->lookup("device/framebuffer"));
    Handle<IPageable> pageable(framebuffer);
    TEST(pageable);

    fill(framebuffer, 255, 0, 0);
    esSleep(10000000);
    fill(framebuffer, 0, 255, 0);
    esSleep(10000000);
    fill(framebuffer, 0, 0, 255);
    esSleep(10000000);
    fill(framebuffer, 255, 255, 255);
    esSleep(10000000);

    pattern(framebuffer);
    esSleep(10000000);

    cursor->show();

    for (int y = 0; y < 768; ++y)
    {
        cursor->setPosition(512, y);
    }

    for (int x = 0; x < 1024; ++x)
    {
        cursor->setPosition(x, 768 / 2);
    }

    for (int y = 0; y < 768; ++y)
    {
        cursor->setPosition(1023, y);
    }

    for (int x = 0; x < 1024; ++x)
    {
        cursor->setPosition(x, 767);
    }

    // long long size;
    // size = framebuffer->getSize();
    // for (long long offset(0); offset < size; offset += 4096)
    // {
    //    unsigned long pte = pageable->get(offset);
    //    esReport("%llx: %lx\n", offset, pte);
   // }

    esReport("done.\n");    // for testing
    return 0;
}
