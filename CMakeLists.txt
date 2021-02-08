cmake_minimum_required(VERSION 3.17)

project(NetworkRequestsHandler LANGUAGES CXX)

set(LIB_NAME "network_requests_handler")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
	inc/
	asio/
	)

file (GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
	${PROJECT_SOURCE_DIR}/inc/*.hpp
	${PROJECT_SOURCE_DIR}/src/*.cpp
	)

add_library(${LIB_NAME} SHARED ${SRC_FILES})

set_target_properties(${LIB_NAME} PROPERTIES VERSION 0.1)

if (WIN32)

else()
	target_link_libraries(${LIB_NAME}
		-lpthread
		-lssl
		-lcrypto
	)
endif()
