//
// Created by khushitshah on 2/15/22.
//

#include <iostream>
#include <algorithm>
#include <chrono>
#include "Engine.h"
#include "StaticEvaluator.h"
#include "utils.h"
#include "SearchInfo.h"
#include "SearchThread.h"

Engine::Engine(TT &tt) : _tt(tt) {
    for (int i = 0; i < 8; i++) {
        _threads[i] = new SearchThread(this);
        _threads[i]->setThreadId(i);
    }
}

int Engine::negamax(libchess::Position pos, int depth, int alpha, int beta, Color color, SearchInfo *info) noexcept {
    info->ply = info->ply % 100;
    if (info->helper_thread && info->terminate_helpers) {
        return 0;
    }

//    if ((info->nodes_searched & 2047) && info->search_finish_time <=
//                                         std::chrono::duration_cast<std::chrono::milliseconds>(
//                                                 std::chrono::system_clock::now().time_since_epoch()).count()) {
//        info->quit_search = true;
//    }

    int score = -INF;

    // TODO: check performance with and without prefetch.
    _tt.prefetch(pos.hash());

    info->nodes_searched++;

    auto alphaOrigin = alpha;

    if (pos.threefold()) {
        return 0;
    }

    alpha = std::max(alpha, -IMMEDIATE_MATE_SCORE + (info->search_depth - depth));
    beta = std::min(beta, IMMEDIATE_MATE_SCORE - (info->search_depth - depth));

    if (alpha >= beta && !info->is_root) {
        return alpha;
    }

    if (depth == 0) {
        return QuiescenceSearch(pos, alpha, beta, color, info);
    }

    if (pos.is_terminal()) {
        if (pos.is_checkmate()) {
            return -(IMMEDIATE_MATE_SCORE - (info->search_depth - depth));
        } else {
            return 0;
        }
    }


    auto ttEntry = _tt.get(pos.hash());
    if (ttEntry->hash == pos.hash() && ttEntry->depth >= depth && !info->is_root) {
        info->ttHits++;
        int scr = ttEntry->eval * (ttEntry->color == color ? 1 : -1);
        if (ttEntry->type == TTEntryType::EXACT) {
            return scr;
        } else if (ttEntry->type == TTEntryType::UPPER_BOUND)
            beta = std::min(beta, scr);
        else if (ttEntry->type == TTEntryType::LOWER_BOUND)
            alpha = std::max(alpha, scr);
        if (alpha > beta)
            return scr;
    }

//    if (ttEntry->hash == pos.hash() && info->helper_thread) {
//        info->ttHits++;
//        int scr = ttEntry->eval * (ttEntry->color == color ? 1 : -1);
//        if (ttEntry->type == TTEntryType::EXACT) {
//            alpha = std::max(alpha, scr - 500);
//            beta = std::min(beta, scr + 500);
//            if (ttEntry->depth >= depth)
//                return scr;
//        } else if (ttEntry->type == TTEntryType::UPPER_BOUND)
//            beta = std::min(beta, scr);
//        else if (ttEntry->type == TTEntryType::LOWER_BOUND)
//            alpha = std::max(alpha, scr);
//        if (alpha > beta)
//            return scr;
//    }

    std::vector<std::pair<libchess::Move, int>> childMoves;
    const auto _ = pos.legal_moves();
    childMoves.reserve(_.size());
    for (auto &move: _) {
        if (!info->restrict_root_moves || !info->is_root)
            childMoves.emplace_back(move, 0);
        else if (info->restrict_root_moves && info->is_root) {
            bool find = false;
            for (int j = 0; j < info->restrict_root_moves_size; j++) {
                if (move == info->restrict_root_moves_ar[j]) {
                    find = true;
                    break;
                }
            }
            if (find)
                childMoves.emplace_back(move, 0);
        }
    }

    // only randomize root moves.
    set_scores(pos, childMoves, color, info);

    info->is_root = false;
    libchess::Move bestMove;
    for (int i = 0; i < childMoves.size(); i++) {
        sortNextMove(i, childMoves);
        auto &move = childMoves[i];

        pos.makemove(move.first);
        info->ply++;
        int moveScore = -negamax(pos, depth - 1, -beta, -alpha, -color, info);
        info->ply--;
        pos.undomove();

        if (info->quit_search) {
            return 0;
        }

        if (info->helper_thread && info->terminate_helpers) {
            return 0;
        }

        if (depth == info->search_depth && !info->helper_thread) {
//            std::cout << "Move: " << move.first << " Score: " << moveScore << " sortScore: " << move.second
//                      << std::endl;
            this->moves.emplace_back(move.first, moveScore);
        }

        if (moveScore > score) {
            score = moveScore;
            bestMove = move.first;
        }

        if (moveScore > alpha) {
            if (moveScore > beta) {
                if (!move.first.is_capturing()) {
                    std::swap(info->killers[0][info->ply], info->killers[1][info->ply]);
                    info->killers[0][info->ply] = move.first;
                }
            } else if (!move.first.is_capturing()) {
                info->history[(int) move.first.from()][(int) move.first.to()] += depth * depth;
            }
        }
        alpha = std::max(alpha, score);

        if (alpha >= beta) {
            break;
        }
    }
    TTEntry entry{pos.hash(), TTEntryType::EXACT, static_cast<int8_t>(depth), score, color, false, libchess::Move()};

    if (score <= alphaOrigin) { //
        entry.eval = alpha;
        entry.type = TTEntryType::UPPER_BOUND;
        entry.mv = bestMove;
    } else if (score >= beta) { // CUTOFF
        // TODO: Add to history heuristics.
        entry.eval = beta;
        entry.type = TTEntryType::LOWER_BOUND;
        entry.mv = bestMove;
    } else {
        entry.pv = true;
        entry.mv = bestMove;
    }

    _tt.add(entry);

    if (score >= beta) {
        return beta;
    }

    return score;
}


int Engine::QuiescenceSearch(libchess::Position pos, int alpha, int beta, Color color, SearchInfo *info) noexcept {
    info->ply = info->ply % 100;
    if (info->helper_thread && info->terminate_helpers) {
        return 0;
    }
//    if ((info->nodes_searched & 2047) && info->search_finish_time <=
//                                         std::chrono::duration_cast<std::chrono::milliseconds>(
//                                                 std::chrono::system_clock::now().time_since_epoch()).count()) {
//        info->quit_search = true;
//    }


    int score = color * StaticEvaluator::evaluate(pos);

    info->nodes_searched++;

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

    set_scores(pos, childMoves, color, info);

    for (int i = 0; i < childMoves.size(); i++) {
        sortNextMove(i, childMoves);
        auto &move = childMoves[i];

        pos.makemove(move.first);
        info->ply++;
        int moveScore = -QuiescenceSearch(pos, -beta, -alpha, -color, info);
        info->ply--;
        pos.undomove();

        if (info->quit_search) {
            return 0;
        }

        if (info->helper_thread && info->terminate_helpers) {
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
Engine::get_moves(libchess::Position pos, int depth, Color color, SearchInfo *info) noexcept {
    moves.clear();
    if (info->clear_tt_every_move)
        _tt.clear();

    info->ttHits = 0;
    info->search_time = 0;
    info->sort_time = 0;
    info->move_gen_time = 0;
    info->static_eval_time = 0;
    info->makemove_time = 0;
    info->threefold_time = 0;
    info->search_depth = depth;
    info->nodes_searched = 0;

    auto t1 = std::chrono::high_resolution_clock::now();
    if (info->iterative_deepening) {

        int alpha = -INF;
        int beta = INF;

        std::vector<libchess::Move> _ = pos.legal_moves();

        for (int k = 0; k < info->num_helper_threads; k++) {
            auto *helperInfo = new SearchInfo();
            helperInfo->helper_thread = true;
            helperInfo->terminate_helpers = false;
            helperInfo->is_root = true;
            helperInfo->sort_moves = true;
            helperInfo->nodes_searched = 0;
//          TODO: check if root splitting improves performance or not.
//            if (_.size() >= 8) {
//                helperInfo->restrict_root_moves = true;
//                 restrict overlapping moves.
//                helperInfo->restrict_root_moves_size = (_.size() / info->num_helper_threads) + 1;
//                for (int j = 0; j < helperInfo->restrict_root_moves_size; j++) {
//                    helperInfo->restrict_root_moves_ar[j] = _[rand() % _.size()];
//                }
//            }
            _threads[k]->setSearchInfo(helperInfo);
            _threads[k]->setRootPos(pos);
        }

        std::pair<libchess::Move, int> bestMove;
        for (int i = 1; i <= depth;) {
            moves.clear();
            info->search_depth = i;
            info->quit_search = false;

//            std::cout << "Starting threads.. " << std::endl;
            for (int k = 0; k < info->num_helper_threads; k++) {
                _threads[k]->setDepth(i);
                _threads[k]->getSearchInfo()->search_depth = i;
                _threads[k]->getSearchInfo()->terminate_helpers = false;
                _threads[k]->getSearchInfo()->is_root = true;
                _threads[k]->getSearchInfo()->ply = 0;
                _threads[k]->getSearchInfo()->quit_search = false;
                _threads[k]->start_searching(alpha, beta);
            }

            info->helper_thread = false;
            info->is_root = true;
            info->ply = 0;
//            std::cout << "Depth: " << i << std::endl;
            int val = negamax(pos, i, alpha, beta, color, info);
            for (auto move: moves) {
                if (move.second > bestMove.second) {
                    bestMove = move;
                }
            }
            std::cout << "info depth " << i << "score cp " << bestMove.second << " pv ";
            print_pv_line(pos, _tt);
            std::cout << std::endl;
//            std::cout << "Waiting for threads to finish..." << std::endl;
            for (int k = 0; k < info->num_helper_threads; k++) {
                _threads[k]->wait_for_search_to_finish();
            }
            if (info->quit_search) {
                return {bestMove};
            }
//            print_pv_line(pos, _tt);
//            std::cout << std::endl;
            if ((val <= alpha) || (val >= beta)) {
                alpha = -INF;
                beta = INF;
                continue;
            }
            alpha = val - VAL_WINDOW;
            beta = val + VAL_WINDOW;
            i++;
        }
//        for (int k = 0; k < info->num_helper_threads; k++) {
//            delete _threads[k]->getSearchInfo();
//        }
    } else {
        negamax(pos, depth, -INF, INF, color, info);
    }
    auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1);
    info->search_time += d1.count();

    info->search_depth = -1;
    return moves;
}

void Engine::set_scores(libchess::Position pos, std::vector<std::pair<libchess::Move, int>> &mvs, Color color,
                        SearchInfo *info) const noexcept {


    auto entry = _tt.get(pos.hash());

    for (auto &move: mvs) {
        if (info->helper_thread && info->is_root) {
//          TODO: Random + TT
            move.second = rand() % 200000;
//          continue;
        } else {
            move.second = 0;
        }

        // the best move in current position stored in hashtable.
        if (move.first == entry->mv) {
            move.second += 200000;
        }


        if (move.first == info->killers[0][info->ply]) {
            move.second += VAL_KILLER_0;
        } else if (move.first == info->killers[1][info->ply]) {
            move.second += VAL_KILLER_1;
        } else if (!move.first.is_capturing()) {
            move.second += info->history[(int) move.first.to()][(int) move.first.from()];
        }


        move.second += getMVVLVA(move.first);

//        if (entry->hash == pos.hash()) {
//            if (entry->mv != libchess::Move() && move.first == entry->mv)
//                move.second += 2000000;
//        }
//        if (entry->pv) {
//            move.second += 200000;
//        }
//        // add MVV/LVA score
//        move.second += getMVVLVA(move.first) + 10000;
//
//        move.second += info->history[(int) move.first.from()][(int) move.first.to()];
//
//        if (move.second == info->killers[0][info->ply]) {
//            move.second += 100000;
//        } else if (move.second == info->killers[1][info->ply]) {
//            move.second += 50000;
//        }
//
//
//        pos.makemove(move.first);
//        // TODO: move.second += color * StaticEvaluator::evaluate(pos);
//        auto hash = pos.hash();
//        auto _tempEntry = _tt.get(pos.hash());
//
//        pos.undomove();
//
//        if (hash != _tempEntry->hash) continue;
//        if (_tempEntry->type == TTEntryType::EXACT) {
//            move.second += 10000;
//        } else if (_tempEntry->type == TTEntryType::LOWER_BOUND) {
//            move.second += -10000;
//        } else if (_tempEntry->type == TTEntryType::UPPER_BOUND) {
//            move.second -= 5000;
//        }
//        if (_tempEntry->pv) {
//            move.second += 200000;
//        }
//        // previous evaluation helps alot.
//        move.second += (_tempEntry->eval * (color == _tempEntry->color ? 1 : -1));
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
//    if (!sort_moves) return;
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