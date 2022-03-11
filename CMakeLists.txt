cmake_minimum_required(VERSION 3.12)
project(sce)

 find_library(TENSORFLOW_LIB tensorflow HINT /usr/local/lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)


#ADD_DEFINITIONS(-D USE_NN=1)



add_executable(SuperChessEngine src/main.cpp src/constants.h src/StaticEvaluator.cpp src/StaticEvaluator.h src/utils.cpp src/utils.h src/Engine.cpp src/Engine.h src/TT.h src/SearchInfo.h src/SearchThread.h src/uci.cpp src/uci.h src/SETT.h src/StaticEvaluatorNN.h src/StaticEvaluatorNN.cpp)

target_link_libraries(SuperChessEngine "-lpthread")
target_link_libraries(SuperChessEngine "${TENSORFLOW_LIB}")
target_link_libraries(SuperChessEngine ${CMAKE_SOURCE_DIR}/libs/libchess/build/static/libchess.a)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/cppflow)
