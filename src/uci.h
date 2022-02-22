#ifndef SCE_UCI_HPP
#define SCE_UCI_HPP

#include <string>
#include <sstream>
#include "Engine.h"

namespace sce {
class UCI {
 private:
  void runLoop();
  void getPosition();
  void go();

  std::string _curFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Engine *_engine;
  TT *tt;
  std::stringstream ss;
 public:
  void init();
};
}
#endif