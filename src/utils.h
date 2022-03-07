//
// Created by khushitshah on 2/15/22.
//
#include <string>
#include <unordered_set>
#include "TT.h"
#include "SearchInfo.h"
#include "position.hpp"

#ifndef SCE_UTILS_H
#define SCE_UTILS_H
namespace sce {
std::string get_board(const std::string &fen) noexcept;

std::string get_board_pretty(const std::string &fen) noexcept;

std::string get_san(const libchess::Position &pos, const libchess::Move &move) noexcept;

std::string get_piece_char(libchess::Piece piece) noexcept;

void print_pv_line(libchess::Position pos, const TT *tt, std::unordered_set<std::uint64_t> hashes = {}, int depth = 0);

bool is_check(libchess::Position &pos, libchess::Move move) noexcept;

void print_search_info(SearchInfo *info) noexcept;

bool is_endgame(libchess::Position &pos) noexcept;
}



#endif
