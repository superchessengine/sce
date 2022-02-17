//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_ENGINE_H
#define SCE_ENGINE_H


#include "position.hpp"
#include "constants.h"
#include "TT.h"

class Engine {
private:
    std::vector<std::pair<libchess::Move, int>> moves;
    TT &_tt;
    int search_depth = -1;

//    void sort_moves_tt(libchess::Position &pos, std::vector<libchess::Move> &moves);

public:

    Engine(TT &tt) : _tt(tt) {}


    std::vector<std::pair<libchess::Move, int>>
    get_moves(libchess::Position &pos, int depth, Color color) noexcept;

    int negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color) noexcept;

    long long nodes_searched = 0;
    long long search_time = 0;
    long long sort_time = 0;
    long long move_gen_time = 0;
    long long static_eval_time = 0;
    long long makemove_time = 0;
    long long threefold_time = 0;
    bool sort_moves = true;
    int ttHits = 0;
    bool iterative_deepening = true;
    bool clear_tt_every_move = false;

    void
    set_scores(libchess::Position pos, std::vector<std::pair<libchess::Move, int>> &moves, Color color) const noexcept;

    static int getMVVLVA(const libchess::Move &move);

    void sortNextMove(int index, std::vector<std::pair<libchess::Move, int>> &moves) const noexcept;

    int QuiescenceSearch(libchess::Position &pos, int alpha, int beta, Color color) noexcept;
};


#endif //SCE_ENGINE_H
