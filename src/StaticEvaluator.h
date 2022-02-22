//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_STATICEVALUATOR_H
#define SCE_STATICEVALUATOR_H

#include "position.hpp"
namespace sce {
class StaticEvaluator {
 public:
  static int evaluate(libchess::Position position) noexcept;

  static int evaluateMove(libchess::Position position, const libchess::Move &move) noexcept;
};
}

#endif //SCE_STATICEVALUATOR_H
