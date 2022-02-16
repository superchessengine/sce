//
// Created by khushitshah on 2/15/22.
//

#include <iostream>
#include <algorithm>
#include <chrono>
#include "Engine.h"
#include "StaticEvaluator.h"
#include "utils.h"

int Engine::negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color) noexcept {
    _tt.prefetch(pos.hash());

    nodes_searched++;

    auto alphaOrigin = alpha;

    auto t1 = std::chrono::high_resolution_clock::now();
    if (pos.threefold()) {
        return 0;
    }
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    threefold_time += d1.count();

    auto ttEntry = _tt.get(pos.hash());
    if (ttEntry->hash == pos.hash() && ttEntry->depth >= depth) {
        ttHits++;
        if (depth != search_depth && ttEntry->type == TTEntryType::EXACT)
            return ttEntry->eval * (color == ttEntry->color ? 1 : -1);
        else if (ttEntry->type == TTEntryType::UPPER_BOUND)
            beta = std::min(beta, (int) ttEntry->eval *
                                  (color == ttEntry->color ? 1 : -1));
        else if (ttEntry->type == TTEntryType::LOWER_BOUND)
            alpha = std::max(alpha, (int) ttEntry->eval *
                                    (color == ttEntry->color ? 1 : -1));

        if (alpha > beta)
            return ttEntry->eval * (color == ttEntry->color ? 1 : -1);
    }
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
        std::stable_sort(moves.begin(), moves.end(), [&pos](const auto &a, const auto &b) {
            return StaticEvaluator::evaluateMove(pos, a) > StaticEvaluator::evaluateMove(pos, b);
        });
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        sort_time += d1.count();
    }


    if (depth == search_depth) {
        for (const auto &move: moves) {
            std::cout << get_san(pos, move) << " ";
        }
        std::cout << std::endl;
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
    TTEntry entry{pos.hash(), TTEntryType::EXACT, static_cast<int8_t>(depth), score, color};
    if (score <= alphaOrigin) {
        entry.type = TTEntryType::UPPER_BOUND;
    } else if (score >= beta) {
        entry.type = TTEntryType::LOWER_BOUND;
    }

    _tt.add(entry);

    return score;
}

std::vector<std::pair<libchess::Move, int>>
Engine::get_moves(libchess::Position &pos, int depth, Color color) noexcept {
    moves.clear();

    ttHits = 0;
    search_time = 0;
    sort_time = 0;
    move_gen_time = 0;
    static_eval_time = 0;
    makemove_time = 0;
    threefold_time = 0;

    nodes_searched = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    if (iterative_deepening) {
        for (int i = 1; i <= depth; i++) {
            search_depth = i;
            std::cout << "Depth: " << i << std::endl;
            negamax(pos, i, -INF, INF, color);

        }
    } else {
        search_depth = depth;
        negamax(pos, depth, -INF, INF, color);
    }
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    search_time += d1.count();

    search_depth = -1;
    return moves;
}
