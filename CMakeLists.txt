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

set(SOURCE_FILES amsd.cpp server.cpp amsd.hpp ssh.cpp request.cpp Operations/Operations.cpp Operations/fwver.cpp DataProcessing/CgMinerAPI.cpp Operations/issues.cpp lib/avalon_errno.cpp lib/avalon_errno.hpp lib/api_parser.cpp lib/api_parser.hpp Operations/supertac.cpp utils.cpp Operations/controller.cpp Runtime/DataBase.cpp lib/rfc1342.cpp lib/rfc1342.hpp lib/rfc3339.cpp lib/rfc3339.hpp Operations/mailreport.cpp Operations/config.cpp Operations/status.cpp Operations/farmap.cpp Operations/glimpse.cpp Operations/history.cpp Operations/ascset.cpp Operations/rawapi.cpp user.cpp Operations/user.cpp Operations/login.cpp Operations/version.cpp Operations/jsonreport.cpp compatibility.hpp compatibility.cpp Runtime/RuntimeData.cpp Operations/Operations.hpp Runtime/Runtime.hpp DataProcessing/DataProcessing.hpp DataProcessing/CrapNemesis.cpp DataProcessing/Collector.cpp DataProcessing/Issue.cpp DataProcessing/AvalonError.cpp DataProcessing/Report.cpp Runtime/Config.cpp User/User.hpp User/Login.cpp User/Auth.cpp Services/Services.cpp Services/Services.hpp Services/API_Common.cpp Services/API_UnixSocket.cpp Services/API_TCP.cpp)

add_executable(amsd ${SOURCE_FILES})
add_executable(amsd_cli amsd_cli.cpp)
add_executable(ams_cgi ams_cgi.cpp)

target_link_libraries(ams_cgi)
target_link_libraries(amsd_cli jansson)
target_link_libraries(amsd Reimu pthread jansson ssh2 sqlite3 event curl b64 crypto)

install(TARGETS amsd_cli amsd DESTINATION bin)
