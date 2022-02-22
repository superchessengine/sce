//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_ENGINE_H
#define SCE_ENGINE_H

#include "position.hpp"
#include "constants.h"
#include "TT.h"
#include "SearchInfo.h"
#include "SearchThread.fwd.h"

namespace sce {

class Engine {
 private:
  std::vector<std::pair<libchess::Move, int>> moves;
  TT *_tt;
  SearchThread *_threads[MAX_THREADS];
 public:
  Engine(TT *tt);

  std::vector<std::pair<libchess::Move, int>>
  get_moves(libchess::Position pos, int depth, Color color, SearchInfo *info) noexcept;

  int negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color, SearchInfo *info) noexcept;

  void
  set_scores(libchess::Position &pos, std::vector<std::pair<libchess::Move, int>> &mvs, Color color,
			 SearchInfo *info, bool q = false) const noexcept;

  static int getMVVLVA(const libchess::Move &move);

  static void sortNextMove(int index, std::vector<std::pair<libchess::Move, int>> &moves) noexcept;

  int QuiescenceSearch(libchess::Position &pos, int alpha, int beta, Color color, SearchInfo *info) noexcept;

};
}
#endif //SCE_ENGINE_H
