cmake_minimum_required(VERSION 3.21)
project(sce)

find_library(TENSORFLOW_LIB tensorflow HINT /usr/local/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)


OPTION(OLD_VALS "Option description" OFF) # Enabled by default
IF(OLD_VALS)
    ADD_DEFINITIONS(-D OLD_VALS=1)
ENDIF(OLD_VALS)

OPTION(NEW_VALS "Option description" ON) # Enabled by default
IF(NEW_VALS)
    ADD_DEFINITIONS(-D NEW_VALS=1)
ENDIF(NEW_VALS)



add_executable(SuperChessEngine src/main.cpp src/constants.h src/StaticEvaluator.cpp src/StaticEvaluator.h src/utils.cpp src/utils.h src/Engine.cpp src/Engine.h src/TT.h src/SearchInfo.h src/SearchThread.h src/uci.cpp src/uci.h)

#target_link_libraries(SuperChessEngine "-static -static-libgcc -pthread")
target_link_libraries(SuperChessEngine "${TENSORFLOW_LIB}")
target_link_libraries(SuperChessEngine ${CMAKE_SOURCE_DIR}/libs/libchess/build/static/libchess.a)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/cppflow)
