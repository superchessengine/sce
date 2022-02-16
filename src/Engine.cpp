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
    int score = -INF;

    _tt.prefetch(pos.hash());

    nodes_searched++;

    auto alphaOrigin = alpha;

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

    auto ttEntry = _tt.get(pos.hash());
    if (depth != search_depth && ttEntry->hash == pos.hash() && ttEntry->depth >= depth) {
        ttHits++;
        if (ttEntry->type == TTEntryType::EXACT)
            return ttEntry->eval;
        else if (ttEntry->type == TTEntryType::UPPER_BOUND)
            beta = std::min(beta, (int) ttEntry->eval);
        else if (ttEntry->type == TTEntryType::LOWER_BOUND)
            alpha = std::max(alpha, (int) ttEntry->eval);

        if (alpha > beta)
            return ttEntry->eval;
    }


    t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<libchess::Move, int>> childMoves;
    const auto _ = pos.legal_moves();
    childMoves.reserve(_.size());
    for (auto &move: _) {
        childMoves.push_back({move, 0});
    }
    set_scores(pos, childMoves);
    d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    move_gen_time += d1.count();

    libchess::Move bestMove;
    for (int i = 0; i < childMoves.size(); i++) {
        t1 = std::chrono::high_resolution_clock::now();
        sortNextMove(i, childMoves);
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        sort_time += d1.count();
        auto &move = childMoves[i];
        if (search_depth == depth)
            std::cout << move.first << " " << move.second << std::endl;

        t1 = std::chrono::high_resolution_clock::now();
        pos.makemove(move.first);
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        makemove_time += d1.count();

        int moveScore = -negamax(pos, depth - 1, -beta, -alpha, -color);

        t1 = std::chrono::high_resolution_clock::now();
        pos.undomove();
        d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
        makemove_time += d1.count();

        if (depth == this->search_depth) {
            // std::cout << "Move: " << move << " Score: " << score << std::endl;
            this->moves.emplace_back(move.first, score);
        }

        if (moveScore > score) {
            score = moveScore;
            bestMove = move.first;
        }

        alpha = std::max(alpha, score);

        if (alpha >= beta) {
            break;
        }
    }
    TTEntry entry{pos.hash(), TTEntryType::EXACT, static_cast<int8_t>(depth), score, color, false, libchess::Move()};
    if (score <= alphaOrigin) { //
        entry.type = TTEntryType::UPPER_BOUND;
        entry.mv = bestMove;
    } else if (score >= beta) { // CUTOFF
        entry.type = TTEntryType::LOWER_BOUND;
    } else {
        entry.pv = true;
        entry.mv = bestMove;
    }

    _tt.add(entry);

    return score;
}

std::vector<std::pair<libchess::Move, int>>
Engine::get_moves(libchess::Position &pos, int depth, Color color) noexcept {
    moves.clear();
    _tt.clear();
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
            moves.clear();
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

void Engine::set_scores(libchess::Position pos, std::vector<std::pair<libchess::Move, int>> &mvs) const noexcept {
    if (!sort_moves) return;
    // for each move.
    // if it is pv move score += 10000
    // if it is a best move  +=  10000
    // score += MVV/LVA
    // if the pos after move is cut off, score += -200000
    auto entry = _tt.get(pos.hash());
    for (auto &move: mvs) {
        // mv or best move doesn't matter.
        if (entry->mv != libchess::Move() && move.first == entry->mv) {
            move.second += 200000;
        }

        pos.makemove(move.first);
        auto _tempEntry = _tt.get(pos.hash());
//        move.second += StaticEvaluator::evaluate(pos) * 0.1;
        pos.undomove();

        if (_tempEntry->type == TTEntryType::LOWER_BOUND)
            move.second += -20000;

        move.second += _tempEntry->eval;

        // add MVV/LVA score
        move.second += getMVVLVA(move.first);
    }
}


int Engine::getMVVLVA(const libchess::Move &move) {
    if (!move.is_capturing()) {
        return 0;
    }
    int pieceCaptured = move.captured();
    int pieceCapturing = move.piece();
    return MVVLVA_LOOOKUP[pieceCapturing][pieceCaptured];
}

void Engine::sortNextMove(int index, std::vector<std::pair<libchess::Move, int>> &mvs) const noexcept {
    if (!sort_moves) return;
    int bestIndex = index;
    int bestScore = mvs[index].second;

    for (int i = index; i < mvs.size(); i++) {
        if (mvs[i].second > bestScore) {
            bestScore = mvs[i].second;
            bestIndex = i;
        }
    }

    std::swap(mvs[index], mvs[bestIndex]);
}