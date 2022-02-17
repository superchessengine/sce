//
// Created by khushitshah on 2/15/22.
//
#include <string>
#include <unordered_set>
#include "TT.h"

std::string get_board(const std::string &fen) noexcept;

std::string get_board_pretty(const std::string &fen) noexcept;

std::string get_san(const libchess::Position &pos, const libchess::Move &move) noexcept;

std::string get_piece_char(libchess::Piece piece) noexcept;

void print_pv_line(libchess::Position &pos, const TT &tt, std::unordered_set<std::uint64_t> hashes = {});

bool is_check(libchess::Position &pos, libchess::Move move) noexcept;