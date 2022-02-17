//
// Created by khushitshah on 2/15/22.
//

#include <iostream>
#include <algorithm>
#include <chrono>
#include <future>
#include "Engine.h"
#include "StaticEvaluator.h"
#include "utils.h"

int Engine::negamax(libchess::Position pos, int depth, int alpha, int beta, Color color, bool helper) noexcept {
    if (helper && terminate_helpers) {
        return 0;
    }

    int score = -INF;

    // TODO: check perfomance with and without prefetch.
    // _tt.prefetch(pos.hash());

    nodes_searched++;

    auto alphaOrigin = alpha;

//    std::cout << get_board_pretty(pos.get_fen()) << std::endl;

    if (pos.threefold()) {
        return 0;
    }

    alpha = std::max(alpha, -IMMEDIATE_MATE_SCORE + (search_depth - depth));
    beta = std::min(beta, IMMEDIATE_MATE_SCORE - (search_depth - depth));

    if (alpha >= beta) {
        return alpha;
    }

    if (depth == 0) {
        return QuiescenceSearch(pos, alpha, beta, color);
    }

    if (pos.is_terminal()) {
        if (pos.is_checkmate()) {
            return -(IMMEDIATE_MATE_SCORE - (search_depth - depth));
        } else {
            return 0;
        }
    }


    auto ttEntry = _tt.get(pos.hash());
    if (depth != search_depth && ttEntry->hash == pos.hash()) {
        ttHits++;
        int scr = ttEntry->eval * (ttEntry->color == color ? 1 : -1);
        if (ttEntry->type == TTEntryType::EXACT && ttEntry->depth >= depth)
            return scr;
        else if (ttEntry->type == TTEntryType::UPPER_BOUND)
            beta = std::min(beta, scr);
        else if (ttEntry->type == TTEntryType::LOWER_BOUND)
            alpha = std::max(alpha, scr);
        if (alpha > beta /*&& ttEntry->depth >= depth*/)
            return scr;
    }


    std::vector<std::pair<libchess::Move, int>> childMoves;
    const auto _ = pos.legal_moves();
    childMoves.reserve(_.size());
    for (auto &move: _) {
//        std::cout << "Move: " << move << std::endl;
        childMoves.emplace_back(move, 0);
    }

    set_scores(pos, childMoves, color, helper);

    libchess::Move bestMove;
    for (int i = 0; i < childMoves.size(); i++) {
        sortNextMove(i, childMoves);
        auto &move = childMoves[i];

        pos.makemove(move.first);
        int moveScore = -negamax(pos, depth - 1, -beta, -alpha, -color, helper);
        pos.undomove();

        if (helper && terminate_helpers) {
            return 0;
        }

        if (depth == this->search_depth && !helper) {
            std::cout << "Move: " << move.first << " Score: " << moveScore << " sortScore: " << move.second
                      << std::endl;
            this->moves.emplace_back(move.first, moveScore);
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
        entry.pv = true;
        entry.type = TTEntryType::UPPER_BOUND;
        entry.mv = bestMove;
    } else if (score >= beta) { // CUTOFF
        entry.type = TTEntryType::LOWER_BOUND;
        entry.mv = bestMove;
    } else {
        entry.pv = true;
        entry.mv = bestMove;
    }

    _tt.add(entry);

    if (score >= beta)
        return beta;

    return score;
}


int Engine::QuiescenceSearch(libchess::Position pos, int alpha, int beta, Color color, bool helper) noexcept {
    if (helper && terminate_helpers) {
        return 0;
    }

    int score = color * StaticEvaluator::evaluate(pos);

    nodes_searched++;

//    std::cout << get_board_pretty(pos.get_fen()) << std::endl;

    if (pos.threefold()) {
        return 0;
    }

    if (score >= beta) {
        return beta;
    } else if (score > alpha) {
        alpha = score;
    }


    if (pos.is_terminal()) {
        return score;
    }

    std::vector<std::pair<libchess::Move, int>> childMoves;

    const auto _ = pos.legal_moves();

    childMoves.reserve(_.size());
    for (auto &move: _) {
        if (move.is_capturing() || move.is_promoting() || is_check(pos, move))
            childMoves.emplace_back(move, 0);
    }

    if (childMoves.empty()) {
        return score;
    }

    set_scores(pos, childMoves, color, helper);

    for (int i = 0; i < childMoves.size(); i++) {
        sortNextMove(i, childMoves);
        auto &move = childMoves[i];

        pos.makemove(move.first);

        int moveScore = -QuiescenceSearch(pos, -beta, -alpha, -color);

        pos.undomove();

        if (helper && terminate_helpers) {
            return 0;
        }

        if (moveScore > score) {
            score = moveScore;
        }

        alpha = std::max(alpha, score);

        if (alpha >= beta) {
            break;
        }
    }

    return score;
}

std::vector<std::pair<libchess::Move, int>>
Engine::get_moves(libchess::Position pos, int depth, Color color) noexcept {
    moves.clear();
    if (clear_tt_every_move)
        _tt.clear();

    ttHits = 0;
    search_time = 0;
    sort_time = 0;
    move_gen_time = 0;
    static_eval_time = 0;
    makemove_time = 0;
    threefold_time = 0;
    search_depth = depth;

    nodes_searched = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    if (iterative_deepening) {
        libchess::Position pos1(pos.get_fen());
        libchess::Position pos2(pos.get_fen());
        libchess::Position pos3(pos.get_fen());

        assert(pos1.hash() == pos2.hash() && pos1.hash() == pos3.hash() && pos.hash() == pos1.hash());

        int alpha = -INF;
        int beta = INF;


        for (int i = 1; i <= depth;) {
            nodes_searched = 0;
            moves.clear();
            search_depth = i;
            terminate_helpers = false;

            std::cout << "Depth: " << i << std::endl;
            auto th3 = std::thread([&]() {
                negamax(pos1, i - rand() % 3, alpha, beta, color, true);
            });

            auto th1 = std::thread([&]() {
                negamax(pos1, i + rand() % 3, alpha, beta, color, true);
            });

//            auto th2 = std::thread([&]() {
//                negamax(pos1, i, alpha, beta, color, true);
//            });

            int val = negamax(pos, i, alpha, beta, color, false);


            print_pv_line(pos, _tt);

            std::cout << std::endl;

            terminate_helpers = true;

            th1.join();
//            th2.join();
            th3.join();


            if ((val <= alpha) || (val >= beta)) {
                alpha = -INF;
                beta = INF;
                continue;
            }
            alpha = val - VAL_WINDOW;
            beta = val + VAL_WINDOW;
            i++;
        }

    } else {
        negamax(pos, depth, -INF, INF, color);
    }
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    search_time += d1.count();

    search_depth = -1;
    return moves;
}

void Engine::set_scores(libchess::Position pos, std::vector<std::pair<libchess::Move, int>> &mvs, Color color,
                        bool helper) const noexcept {
    if (!sort_moves) return;


    auto entry = _tt.get(pos.hash());

    for (auto &move: mvs) {
//        if (helper) {
//            // TODO: Random + TT
//            move.second = rand() % 2000;
//        } else {
        move.second = 0;
//        }
        if (entry->hash == pos.hash()) {
            if (entry->mv != libchess::Move() && move.first == entry->mv)
                move.second += 200000;
        }

        // add MVV/LVA score
        move.second += getMVVLVA(move.first);


        pos.makemove(move.first);
        // TODO: move.second += color * StaticEvaluator::evaluate(pos);
        auto hash = pos.hash();
        auto _tempEntry = _tt.get(pos.hash());

        pos.undomove();

        if (hash != _tempEntry->hash) continue;
        if (_tempEntry->type == TTEntryType::EXACT) {
            move.second += 10000;
        } else if (_tempEntry->type == TTEntryType::LOWER_BOUND) {
            move.second += -10000;
        } else if (_tempEntry->type == TTEntryType::UPPER_BOUND) {
            move.second -= 5000;
        }
        // previous evaluation helps alot.
        move.second += (_tempEntry->eval * (color == _tempEntry->color ? 1 : -1));
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