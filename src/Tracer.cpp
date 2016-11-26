/*
 * Copyright 2016, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   Tracer.cpp
 * Author: alex
 * 
 * Created on November 25, 2016, 11:25 PM
 */

#include "Tracer.hpp"

#include "platform.hpp"
#include "utils.hpp"

namespace checker {

Tracer::Tracer() :
enabled(false),
json(),
counter(0) { }

Tracer::Tracer(bool enabled) :
enabled(enabled),
json(),
counter(0) { }

// Pre-C++11 move logic
Tracer::Tracer(const Tracer& other) :
enabled(other.enabled),
json(other.json),
counter(other.counter) {
    other.enabled = false;
    other.counter = 0;
}

void Tracer::trace(const std::string& message) const {
    if (enabled) {
        std::string field = platform::current_datetime();
        field.push_back('_');
        field.append(utils::to_string(++counter));
        json.put_string(field, message);
    }
}

const JsonRecord& Tracer::get_json() const {
    return json;
}

bool Tracer::is_enabled() const {
    return enabled;
}

void Tracer::set_enabled(bool value) {
    this->enabled = value;
}

} // namespace
