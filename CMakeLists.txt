cmake_minimum_required(VERSION 3.6)
project(amsd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES amsd.cpp server.cpp amsd.hpp ssh.cpp request.cpp operations.cpp operations/fwver.cpp operations/mmupgrade.cpp lib/cgminer_api.cpp operations/issues.cpp lib/avalon_errno.cpp lib/avalon_errno.hpp lib/cgminer_api.hpp lib/api_parser.cpp lib/api_parser.hpp lib/mysql.cpp lib/mysql.hpp operations/supertac.cpp utils.cpp)
add_executable(amsd ${SOURCE_FILES})