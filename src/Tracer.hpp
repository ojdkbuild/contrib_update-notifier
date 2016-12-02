/*
 * Copyright 2016 Red Hat, Inc.
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
 * File:   Tracer.hpp
 * Author: alex
 *
 * Created on November 25, 2016, 11:25 PM
 */

#ifndef TRACER_HPP
#define	TRACER_HPP

#include <stdint.h>
#include <string>

#include "JsonRecord.hpp"

namespace checker {

class Tracer {
    bool enabled;    
    JsonRecord json;
    uint32_t counter;
    
public:
    Tracer();

    Tracer(bool enabled);

    void trace(const std::string& message);
    
    const JsonRecord& get_json() const;

    bool is_enabled() const;

    void set_enabled(bool value);
    
private:
    Tracer(const Tracer& other);

    Tracer& operator=(const Tracer& other);

};

} // namespace

#endif	/* TRACER_HPP */

