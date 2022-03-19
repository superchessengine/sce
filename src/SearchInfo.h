//
// Created by khushitshah on 2/18/22.
//

#ifndef SCE_SEARCHINFO_H
#define SCE_SEARCHINFO_H

#include "move.hpp"
#include <chrono>

namespace sce {

struct SearchInfo {
  /* Config */
  bool sort_moves = true;
  bool iterative_deepening = true;
  bool clear_tt_every_move = false;
  bool terminate_helpers = false;
  int search_depth = -1;
  bool helper_thread = false;
  bool null_move = true;

  bool is_root = true;
  int num_helper_threads = 0;

  /* Analytics */
  int ttHits = 0;
  long long nodes_searched = 0;
  long long search_time = 0;
  std::chrono::milliseconds search_finish_time;
  long long null_cutoffs = 0;
  long long cutt_offs = 0;

  int ply = 0;

  /* Move Ordering */
  long history[2][7][64] = {0};
  // maxdepth = 100
  std::uint32_t killers[2][100] = {0};

  bool quit_search = false;

  std::chrono::milliseconds starttime;

  int fhf;
  int fh;
};
};
#endif //SCE_SEARCHINFO_H
