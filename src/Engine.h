//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_ENGINE_H
#define SCE_ENGINE_H


#include "position.hpp"
#include "constants.h"
#include "type.h"

class Engine {
private:
    std::vector<std::pair<libchess::Move, int>> moves;
    int search_depth = -1;
public:
    long long nodes_searched = 0;

    std::vector<std::pair<libchess::Move, int>>
    get_moves(libchess::Position &pos, int depth, Color color) noexcept;

    int negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color) noexcept;

    long long search_time;
    long long sort_time;
    long long move_gen_time;
    long long static_eval_time;
    long long makemove_time;
    long long threefold_time;
    bool sort_moves = true;
};


#endif //SCE_ENGINE_H
