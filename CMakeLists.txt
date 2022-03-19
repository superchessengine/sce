cmake_minimum_required(VERSION 3.12)
project(sce)

message("finding tensorflow!!")
find_library(TENSORFLOW_LIB NAMES tensorflow HINTS "${CMAKE_SOURCE_DIR}/libs/tensorflow/")
message(${TENSORFLOW_LIB})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")



# include_directories(${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)


ADD_DEFINITIONS(-D USE_NN=1)
#ADD_DEFINITIONS(-D USE_MSNN=1)

add_executable(SuperChessEngine src/main.cpp src/constants.h src/StaticEvaluator.cpp src/StaticEvaluator.h src/utils.cpp src/utils.h src/Engine.cpp src/Engine.h src/TT.h src/SearchInfo.h src/SearchThread.h src/uci.cpp src/uci.h src/SETT.h src/StaticEvaluatorNN.h src/StaticEvaluatorNN.cpp src/MoveSorterNN.cpp src/MoveSorterNN.h src/MSTT.h src/json.h)


target_link_libraries(SuperChessEngine -static)
target_link_libraries(SuperChessEngine -pthread)
target_link_libraries(SuperChessEngine ${CMAKE_SOURCE_DIR}/libs/libchess/build/static/libchess.a)
target_link_libraries(SuperChessEngine ${TENSORFLOW_LIB})

target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/tensorflow/include)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/libchess/src/libchess)
target_include_directories(SuperChessEngine PUBLIC ${CMAKE_SOURCE_DIR}/libs/cppflow)
