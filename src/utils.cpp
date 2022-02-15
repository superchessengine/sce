//
// Created by khushitshah on 2/15/22.
//
#include <string>
#include "position.hpp"
#include "utils.h"


std::string get_board(const std::string &fen) noexcept {
    std::string board;
    for (char i: fen) {
        if (i == '/') continue;
        if (i >= '1' && i <= '9') {
            for (int j = 0; j < i - '0'; j++) {
                board += '.';
            }
        } else if (i == ' ') {
            return board;
        } else {
            board += i;
        }
    }
    return board;
}


std::string get_board_pretty(const std::string &fen) noexcept {
    std::string board;
    for (char i: fen) {
        if (i == '/') board += '\n';
        if (i >= '1' && i <= '9') {
            for (int j = 0; j < i - '0'; j++) {
                board += '.';
            }
        } else if (i == ' ') {
            return board;
        } else {
            board += i;
        }
    }
    return board;
}


std::string get_san(const libchess::Position &pos, const libchess::Move &move) noexcept {
    std::string san;
    if (move.type() == libchess::MoveType::ksc) {
        return "O-O";
    } else if (move.type() == libchess::MoveType::qsc) {
        return "O-O-O";
    }

    san += get_piece_char(move.piece());

    san += libchess::square_strings[(int) move.from()];

    if (move.type() == libchess::MoveType::Capture || move.type() == libchess::MoveType::promo_capture) {
        san += 'x';
    }

    san += libchess::square_strings[(int) move.to()];

    if (move.type() == libchess::MoveType::promo || move.type() == libchess::MoveType::promo_capture) {
        san += '=';
        san += get_piece_char(move.promotion());
    }

    if (pos.is_checkmate()) {
        san += '#';
    } else if (pos.in_check()) {
        san += '+';
    }

    return san;
}

std::string get_piece_char(libchess::Piece piece) noexcept {
    if (piece == libchess::Piece::Pawn) {
        return "";
    } else if (piece == libchess::Piece::Knight) {
        return "N";
    } else if (piece == libchess::Piece::Bishop) {
        return "B";
    } else if (piece == libchess::Piece::Rook) {
        return "R";
    } else if (piece == libchess::Piece::Queen) {
        return "Q";
    } else if (piece == libchess::Piece::King) {
        return "K";
    } else {
        return "-";
    }
}
