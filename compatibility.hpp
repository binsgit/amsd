/*
    This file is part of AMSD.
    Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>

    AMSD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMSD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMSD_COMPATIBILITY_HPP
#define AMSD_COMPATIBILITY_HPP

#include "amsd.hpp"

#define JANSSON_VERSION_MEOW	(JANSSON_MAJOR_VERSION*100+JANSSON_MINOR_VERSION*10+JANSSON_MICRO_VERSION)

#if JANSSON_VERSION_MEOW < 270

#endif

#ifndef json_boolean_value
#define json_boolean_value     json_is_true
#endif

#if SQLITE_VERSION_NUMBER < 3008007
extern SQLITE_API int sqlite3_bind_blob64(sqlite3_stmt*, int, const void*, sqlite3_uint64, void(*)(void*));
#endif

#endif //AMSD_COMPATIBILITY_HPP
