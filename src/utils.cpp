//
// Created by khushitshah on 2/15/22.
//
#include <string>
#include <iostream>
#include "position.hpp"
#include "utils.h"

namespace sce
{

   std::map<std::string, int> uciToIndexMap;


  std::string get_board(const std::string &fen) noexcept
  {
    std::string board;
    for (char i : fen)
    {
      if (i == '/')
        continue;
      if (i >= '1' && i <= '9')
      {
        for (int j = 0; j < i - '0'; j++)
        {
          board += '.';
        }
      }
      else if (i == ' ')
      {
        return board;
      }
      else
      {
        board += i;
      }
    }
    return board;
  }

  std::string get_board_pretty(const std::string &fen) noexcept
  {
    std::string board;
    for (char i : fen)
    {
      if (i == '/')
        board += '\n';
      else if (i >= '1' && i <= '9')
      {
        for (int j = 0; j < i - '0'; j++)
        {
          board += '.';
        }
      }
      else if (i == ' ')
      {
        return board;
      }
      else
      {
        board += i;
      }
    }
    return board;
  }

  std::string get_san(const libchess::Position &pos, const libchess::Move &move) noexcept
  {
    std::string san;
    if (move.type() == libchess::MoveType::ksc)
    {
      return "O-O";
    }
    else if (move.type() == libchess::MoveType::qsc)
    {
      return "O-O-O";
    }

    san += get_piece_char(move.piece());

    san += libchess::square_strings[(int)move.from()];

    if (move.type() == libchess::MoveType::Capture || move.type() == libchess::MoveType::promo_capture)
    {
      san += 'x';
    }

    san += libchess::square_strings[(int)move.to()];

    if (move.type() == libchess::MoveType::promo || move.type() == libchess::MoveType::promo_capture)
    {
      san += '=';
      san += get_piece_char(move.promotion());
    }

    if (pos.is_checkmate())
    {
      san += '#';
    }
    else if (pos.in_check())
    {
      san += '+';
    }

    return san;
  }

  std::string get_piece_char(libchess::Piece piece) noexcept
  {
    if (piece == libchess::Piece::Pawn)
    {
      return "";
    }
    else if (piece == libchess::Piece::Knight)
    {
      return "N";
    }
    else if (piece == libchess::Piece::Bishop)
    {
      return "B";
    }
    else if (piece == libchess::Piece::Rook)
    {
      return "R";
    }
    else if (piece == libchess::Piece::Queen)
    {
      return "Q";
    }
    else if (piece == libchess::Piece::King)
    {
      return "K";
    }
    else
    {
      return "-";
    }
  }

  // recursive function till tt_entry contains pv move.
  void print_pv_line(libchess::Position pos, const TT *tt, std::unordered_set<std::uint64_t> hashes, int depth)
  {

    if (depth >= 20)
      return;

    if (pos.is_terminal())
      return;

    if (hashes.find(pos.hash()) != hashes.end())
    {
      return;
    }

    hashes.insert(pos.hash());

    auto entry = tt->get(pos.hash());

    if (!entry->pv)
      return;

    pos.makemove(entry->mv);
    std::cout << entry->mv << " ";
    print_pv_line(pos, tt, hashes, depth + 1);
    pos.undomove();
  }

  bool is_check(libchess::Position &pos, libchess::Move move) noexcept
  {
    pos.makemove(move);
    bool in_check = pos.in_check();
    pos.undomove();
    return in_check;
  }

  void print_search_info(SearchInfo *info) noexcept
  {
    std::cout << "SearchInfo: " << std::endl;
    std::cout << "\t search_depth: " << info->search_depth << std::endl;
    std::cout << "\t nodes_searched: " << info->nodes_searched << std::endl;
    std::cout << "\t sort moves: " << info->sort_moves << std::endl;
  }

  bool is_endgame(const libchess::Position &pos) noexcept
  {
    std::string board = get_board(pos.get_fen());
    int score = 0;

    for (int i = 0; i < 64; i++)
    {
      if (board[i] == 'P')
      {
        score += PAWN_VALUE;
      }
      else if (board[i] == 'p')
      {
        score += PAWN_VALUE;
      }
      else if (board[i] == 'N')
      {
        score += KNIGHT_VALUE;
      }
      else if (board[i] == 'n')
      {
        score += KNIGHT_VALUE;
      }
      else if (board[i] == 'B')
      {
        score += BISHOP_VALUE;
      }
      else if (board[i] == 'b')
      {
        score += BISHOP_VALUE;
      }
      else if (board[i] == 'R')
      {
        score += ROOK_VALUE;
      }
      else if (board[i] == 'r')
      {
        score += ROOK_VALUE;
      }
      else if (board[i] == 'Q')
      {
        score += QUEEN_VALUE;
      }
      else if (board[i] == 'q')
      {
        score += QUEEN_VALUE;
      }
      else if (board[i] == 'K')
      {
        score += KING_VALUE;
      }
      else if (board[i] == 'k')
      {
        score += KING_VALUE;
      }
    }
    return score < ENDGAME_THRESHOLD;
  }


  void loadUciToIndexMap() {
	std::ifstream i("uci_to_index.json");
	nlohmann::json j;
	i >> j;

	for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
	  uciToIndexMap[it.key()] = it.value();
	}

	std::cout << "UCI to Index Map loaded successfully." << std::endl;
  }

  int moveToIndex(const libchess::Move &move) {
	// move must not be zero.
	assert(move != 0);
	return uciToIndexMap[(std::string) move];
  }
};