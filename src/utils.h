//
// Created by khushitshah on 2/15/22.
//
#include <string>

std::string get_board(const std::string &fen) noexcept;

std::string get_san(const libchess::Position &pos, const libchess::Move &move) noexcept;

std::string get_piece_char(libchess::Piece piece) noexcept;