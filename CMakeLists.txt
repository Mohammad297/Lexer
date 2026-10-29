cmake_minimum_required(VERSION 3.15)
project(Lexer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_SOURCES
	src/checkers.cpp
	src/engine.cpp
	src/lexer.cpp
	src/token_stream.cpp
)

add_library(lexer_engine STATIC ${LIB_SOURCES})

target_include_directories(lexer_engine PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_include_directories(lexer_engine PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include/lex
)

add_executable (Lexer main.cpp)

target_link_libraries(Lexer PRIVATE lexer_engine)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Lexer PROPERTY CXX_STANDARD 20)
endif()

