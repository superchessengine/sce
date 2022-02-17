//
// Created by khushitshah on 2/15/22.
//

#include <iostream>
#include "StaticEvaluator.h"
#include "utils.h"
#include "constants.h"

int StaticEvaluator::evaluate(const libchess::Position &position) noexcept {
    int score = 0;

    const int color = position.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK;
//    std::cout << get_board_pretty(position.get_fen()) << std::endl << std::endl;
    if (position.is_checkmate()) {
        if (color == COLOR_WHITE) {
            return -9999999;
        } else {
            return 9999999;
        }
    }

    if (position.is_stalemate()) {
        return 0;
    }


    // TODO: Check how is it affecting performance.
//    if (position.threefold()) {
//        return 0;
//    }

    // TODO: Check performance, can it be more optimized?
    std::string board = get_board(position.get_fen());

    for (int i = 0; i < 64; i++) {
        if (board[i] == 'P') {
            score += (PAWN_VALUE + PAWN_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'p') {
            score -= (PAWN_VALUE + PAWN_INC[get_index(i, COLOR_BLACK)]);
        } else if (board[i] == 'N') {
            score += (KNIGHT_VALUE + KNIGHT_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'n') {
            score -= (KNIGHT_VALUE + KNIGHT_INC[get_index(i, COLOR_BLACK)]);
        } else if (board[i] == 'B') {
            score += (BISHOP_VALUE + BISHOP_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'b') {
            score -= (BISHOP_VALUE + BISHOP_INC[get_index(i, COLOR_BLACK)]);
        } else if (board[i] == 'R') {
            score += (ROOK_VALUE + ROOK_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'r') {
            score -= (ROOK_VALUE + ROOK_INC[get_index(i, COLOR_BLACK)]);
        } else if (board[i] == 'Q') {
            score += (QUEEN_VALUE + QUEEN_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'q') {
            score -= (QUEEN_VALUE + QUEEN_INC[get_index(i, COLOR_BLACK)]);
        } else if (board[i] == 'K') {
            score += (KING_VALUE + KING_INC[get_index(i, COLOR_WHITE)]);
        } else if (board[i] == 'k') {
            score -= (KING_VALUE + KING_INC[get_index(i, COLOR_BLACK)]);
        }
    }

    return score;
}

int StaticEvaluator::evaluateMove(libchess::Position &position, const libchess::Move &move) noexcept {
    position.makemove(move);
    int score = evaluate(position);
    position.undomove();

    return score;
}
