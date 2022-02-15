//
// Created by khushitshah on 2/15/22.
//

#include <iostream>
#include <algorithm>
#include <chrono>
#include "Engine.h"
#include "StaticEvaluator.h"

int Engine::negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color) noexcept {
    nodes_searched++;

    auto t1 = std::chrono::high_resolution_clock::now();
    if (pos.threefold()) {
        return 0;
    }
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    threefold_time += d1.count();

    t1 = std::chrono::high_resolution_clock::now();
    if (depth == 0 || pos.is_terminal()) {
        return color * StaticEvaluator::evaluate(pos);
    }
    d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    static_eval_time += d1.count();

    t1 = std::chrono::high_resolution_clock::now();
    auto moves = pos.legal_moves();
    d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    move_gen_time += d1.count();

    if (sort_moves) {
        t1 = std::chrono::high_resolution_clock::now();
        std::sort(moves.begin(), moves.end(), [&pos](const auto &a, const auto &b) {
            return StaticEvaluator::evaluateMove(pos, a) > StaticEvaluator::evaluateMove(pos, b);
        });
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        sort_time += d1.count();
    }
    int score = -INF;
    for (const auto &move: moves) {

        t1 = std::chrono::high_resolution_clock::now();
        pos.makemove(move);
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        makemove_time += d1.count();

        score = std::max(score, -negamax(pos, depth - 1, -beta, -alpha, -color));
        t1 = std::chrono::high_resolution_clock::now();
        pos.undomove();
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        makemove_time += d1.count();

        if (depth == this->search_depth) {
            // std::cout << "Move: " << move << " Score: " << score << std::endl;
            this->moves.emplace_back(move, score);
        }
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break;
        }
    }
    return score;
}

std::vector<std::pair<libchess::Move, int>>
Engine::get_moves(libchess::Position &pos, int depth, Color color) noexcept {
    moves.clear();
    search_depth = depth;

    search_time = 0;
    sort_time = 0;
    move_gen_time = 0;
    static_eval_time = 0;
    makemove_time = 0;
    threefold_time = 0;

    nodes_searched = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    negamax(pos, depth, -INF, INF, color);
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    search_time += d1.count();

    search_depth = -1;
    return moves;
}
