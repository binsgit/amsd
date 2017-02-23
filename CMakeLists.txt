cmake_minimum_required(VERSION 3.6)
project(amsd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES amsd.cpp server.cpp amsd.hpp ssh.cpp request.cpp operations.cpp operations/fwver.cpp operations/mmupgrade.cpp lib/cgminer_api.cpp operations/issues.cpp lib/avalon_errno.cpp lib/avalon_errno.hpp lib/cgminer_api.hpp lib/api_parser.cpp lib/api_parser.hpp operations/supertac.cpp utils.cpp operations/controller.cpp config.cpp data_collector/data_collector.cpp database.cpp data_collector/avalon.hpp data_collector/avalon.cpp amsd_cli.cpp)
add_executable(amsd ${SOURCE_FILES})