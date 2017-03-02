cmake_minimum_required(VERSION 3.0)
project(amsd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")

set(SOURCE_FILES amsd.cpp server.cpp amsd.hpp ssh.cpp request.cpp operations.cpp operations/fwver.cpp operations/mmupgrade.cpp lib/cgminer_api.cpp operations/issues.cpp lib/avalon_errno.cpp lib/avalon_errno.hpp lib/cgminer_api.hpp lib/api_parser.cpp lib/api_parser.hpp operations/supertac.cpp utils.cpp operations/controller.cpp config.cpp core/data_collector.cpp database.cpp core/data_collector.hpp core/report_mail.cpp lib/rfc1342.cpp lib/rfc1342.hpp lib/rfc3339.cpp lib/rfc3339.hpp operations/mailreport.cpp operations/config.cpp)

add_executable(amsd ${SOURCE_FILES})
add_executable(amsd_cli amsd_cli.cpp)

target_link_libraries(amsd_cli jansson)
target_link_libraries(amsd pthread jansson ssh2 sqlite3 event curl b64)