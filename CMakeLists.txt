# This file is part of AMSD.
# Copyright (C) 2016-2017  CloudyReimu <cloudyreimu@gmail.com>
#
# AMSD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# AMSD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with AMSD.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)
project(amsd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")

set(SOURCE_FILES amsd.cpp server.cpp amsd.hpp ssh.cpp request.cpp operations/operations.cpp operations/fwver.cpp operations/mmupgrade.cpp DataProcessing/CgMinerAPI.cpp operations/issues.cpp lib/avalon_errno.cpp lib/avalon_errno.hpp lib/api_parser.cpp lib/api_parser.hpp operations/supertac.cpp utils.cpp operations/controller.cpp config.cpp core/data_collector.cpp runtime/database.cpp core/data_collector.hpp core/report_mail.cpp lib/rfc1342.cpp lib/rfc1342.hpp lib/rfc3339.cpp lib/rfc3339.hpp operations/mailreport.cpp operations/config.cpp core/report.cpp core/report.hpp core/issue.cpp core/issue.hpp operations/status.cpp operations/farmap.cpp operations/glimpse.cpp operations/history.cpp operations/ascset.cpp operations/rawapi.cpp user.cpp operations/user.cpp operations/login.cpp operations/version.cpp operations/jsonreport.cpp compatibility.hpp compatibility.cpp runtime/runtime_data.cpp operations/operations.hpp runtime/runtime.hpp DataProcessing/DataProcessing.hpp DataProcessing/CrapNemesis.cpp DataProcessing/Collector.cpp DataProcessing/Issue.cpp)

add_executable(amsd ${SOURCE_FILES})
add_executable(amsd_cli amsd_cli.cpp)
add_executable(ams_cgi ams_cgi.cpp)

target_link_libraries(ams_cgi)
target_link_libraries(amsd_cli jansson)
target_link_libraries(amsd Reimu pthread jansson ssh2 sqlite3 event curl b64 crypto)

install(TARGETS amsd_cli amsd DESTINATION bin)
