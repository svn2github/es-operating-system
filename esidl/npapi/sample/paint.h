/*
 * Copyright 2010 Google Inc.
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

#ifndef SAMPE_PAINT_H_
#define SAMPE_PAINT_H_

#include "esnpapi.h"

#include <any.h>
#include <reflect.h>
#include <org/w3c/dom.h>

class EventHandler;

class PluginInstance {
 public:
  explicit PluginInstance(org::w3c::dom::html::Window* window)
      : window(window),
        penDown(false) {
    initialize();
  }
  ~PluginInstance();

  NPObject* getScriptableInstance() {
    return 0;
  }

 private:
  org::w3c::dom::html::Window* window;

  EventHandler* downHandler;
  EventHandler* moveHandler;
  EventHandler* upHandler;
  EventHandler* selectHandler;

  bool penDown;

  void initialize();
  void down(org::w3c::dom::events::Event* evt);
  void move(org::w3c::dom::events::Event* evt);
  void up(org::w3c::dom::events::Event* evt);
  void select(org::w3c::dom::events::Event* evt);
};

class EventHandler : public org::w3c::dom::events::EventListener {
 public:
  EventHandler(PluginInstance* instance,
               void (PluginInstance::*handler)(org::w3c::dom::events::Event*))
      : instance(instance),
        handler(handler) {
  }
  virtual void handleEvent(org::w3c::dom::events::Event* evt) {
    (instance->*handler)(evt);
  }
  unsigned int retain() {
    return 1;
  };
  unsigned int release() {
    return 1;
  };

 private:
  PluginInstance* instance;
  void (PluginInstance::*handler)(org::w3c::dom::events::Event* evt);
};

#endif  // SAMPE_PAINT_H_