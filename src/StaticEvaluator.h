//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_STATICEVALUATOR_H
#define SCE_STATICEVALUATOR_H

#include "position.hpp"
#include "SearchInfo.h"
namespace sce {
class StaticEvaluator {
 public:
  static int evaluate(const libchess::Position& position, SearchInfo *info) noexcept;

  static int evaluateMove(libchess::Position position, const libchess::Move &move, SearchInfo *info) noexcept;
};
}

#endif //SCE_STATICEVALUATOR_H
