/*
 * Copyright 2017 Red Hat, Inc.
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
 * File:   transform.hpp
 * Author: alex
 *
 * Created on May 15, 2017, 1:40 PM
 */

#ifndef UPDATE_CHECKER_TRANSFORM_HPP
#define	UPDATE_CHECKER_TRANSFORM_HPP

#include "jansson.h"

#include "CheckerException.hpp"

namespace checker {

json_t* download_manager_transform(json_t* dmjson);

} // namespace

#endif	/* UPDATE_CHECKER_TRANSFORM_HPP */

