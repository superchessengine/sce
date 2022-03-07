#include <chrono>
#include <iostream>
#include "uci.h"
#include "StaticEvaluator.h"

int main(int argc, char *argv[]) {
  srand(time(nullptr));

  std::cout << "StartPos: " << sce::StaticEvaluator::evaluate(libchess::Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) << std::endl;
  std::cout << "e2e4 " << sce::StaticEvaluator::evaluate(libchess::Position("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1")) << std::endl;
  std::cout << "d7d5 " << sce::StaticEvaluator::evaluate(libchess::Position("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2")) << std::endl;
  std::cout << "black is winning " << sce::StaticEvaluator::evaluate(libchess::Position("rnb1kbnr/ppp1pppp/8/3q4/4p3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 4")) << std::endl;
  std::cout << "white is winning " << sce::StaticEvaluator::evaluate(libchess::Position("rnb1kbnr/ppp1pppp/8/3Q4/4p3/5P2/PPPP2PP/RNB1KBNR b KQkq - 0 4")) << std::endl;

  sce::UCI uci;
  uci.init();
  return 0;
}