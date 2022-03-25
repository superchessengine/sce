//#define USE_NN true // Use neural network

#include <chrono>
#include <iostream>

#include "uci.h"
#include "StaticEvaluator.h"
#include "StaticEvaluatorNN.h"
#include "MoveSorterNN.h"
#include "utils.h"
#include "constants.h"

#ifdef USE_NN
#include <tensorflow/c/c_api.h>
#endif
int main(int argc, char *argv[]) {

  sce::loadUciToIndexMap();


#ifdef USE_MSNN
  std::cout << sizeof(sce::MSTTEntry) << std::endl;
#endif
  srand(time(nullptr));
#ifdef USE_NN
  // testing tensorflow.
  printf("Hello from TensorFlow C library version %s\n", TF_Version());
  sce::StaticEvaluatorNN::init();

  // testing move sorter NN
//  std::vector<std::pair<libchess::Move, int>> moves;
//  libchess::Position pos = libchess::Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
//  for (auto move : pos.legal_moves()) {
//	moves.push_back(std::make_pair(move, -1));
//  }
//  sce::MoveSorterNN::scoreMoves(pos, moves);
//  for(auto move: moves) {
//	std::cout << move.first << " " << move.second << std::endl;
//  }
#endif


  sce::SearchInfo *info = new sce::SearchInfo;
  info->helper_thread = false;
  std::cout << "StartPos: " << std::endl << sce::StaticEvaluator::evaluate(libchess::Position(
	  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), info) << std::endl;
  std::cout << "e2e4 " << std::endl << sce::StaticEvaluator::evaluate(libchess::Position(
	  "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"), info) << std::endl;
  std::cout << "d7d5 " << std::endl << sce::StaticEvaluator::evaluate(libchess::Position(
	  "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"), info) << std::endl;
  std::cout << "black is winning " << std::endl << sce::StaticEvaluator::evaluate(libchess::Position(
	  "rnb1kbnr/ppp1pppp/8/3q4/4p3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 4"), info) << std::endl;
  std::cout << "white is winning " << std::endl << sce::StaticEvaluator::evaluate(libchess::Position(
	  "rnb1kbnr/ppp1pppp/8/3Q4/4p3/5P2/PPPP2PP/RNB1KBNR b KQkq - 0 4"), info) << std::endl;

  sce::UCI uci;
  uci.init();
  return 0;
}