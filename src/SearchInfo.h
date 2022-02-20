//
// Created by khushitshah on 2/18/22.
//

#ifndef SCE_SEARCHINFO_H
#define SCE_SEARCHINFO_H

#include "move.hpp"

struct SearchInfo {
    /* Config */
    bool sort_moves = true;
    bool iterative_deepening = true;
    bool clear_tt_every_move = false;
    bool terminate_helpers = false;
    int search_depth = -1;
    bool restrict_root_moves = false;
    int restrict_root_moves_size = 0;
    bool helper_thread = false;

    // restrict the root moves to the following moves.
    libchess::Move restrict_root_moves_ar[64];

    // if we are not in root, don't randomize move ordering even if we are in helper thread.
    bool is_root = true;
    int num_helper_threads = 0;

    /* Analytics */
    int ttHits = 0;
    long long nodes_searched = 0;
    long long search_time = 0;
    long long search_finish_time = 0;
    long long sort_time = 0;
    long long move_gen_time = 0;
    long long static_eval_time = 0;
    long long makemove_time = 0;
    long long threefold_time = 0;
    long long cutt_offs = 0;

    int ply = 0;
    /* Move Ordering */
    std::uint16_t history[64][64] = {0};
    // maxdepth = 100
    std::uint32_t killers[2][100] = {0};
    bool quit_search = false;
};


#endif //SCE_SEARCHINFO_H
