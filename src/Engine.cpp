
//
// Created by khushitshah on 2/15/22.
//

#include<iostream>
#include <algorithm>
#include <chrono>
#include "Engine.h"
#include "StaticEvaluator.h"
#include "StaticEvaluatorNN.h"
#include "utils.h"
#include "SearchInfo.h"
#include "SearchThread.h"
#include "MoveSorterNN.h"

namespace sce {
Engine::Engine(TT *tt) {
  _tt = tt;
  for (int i = 0; i < MAX_THREADS; i++) {
	_threads[i] = new SearchThread(this);
	_threads[i]->setThreadId(i);
  }
}

int
Engine::negamax(libchess::Position &pos, int depth, int alpha, int beta, Color color, SearchInfo *info) noexcept {
  info->ply = info->ply % 100;
  if (info->helper_thread && info->terminate_helpers) {
	return 0;
  }

  if (info->quit_search)
	return 0;

  int score = -INF;

  // TODO: check performance with and without prefetch.

  if (!info->helper_thread && info->nodes_searched & 4097) {
	std::chrono::milliseconds cur =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
	if (cur > info->search_finish_time) {
	  info->quit_search = true;
	  return 0;
	}
  }

  info->nodes_searched++;

  auto alphaOrigin = alpha;

  if (pos.threefold()) {
	return 0;
  }

  if (pos.is_terminal()) {
	return color * (StaticEvaluator::evaluate(pos, info) + info->ply);
  }

#ifdef USE_NN
  // we will SE all the child moves,
  // create a batch and evaluate all child moves in one go.
  // then use TT to store the values and access it at depth = 0.
  if (depth == 1 && !info->helper_thread) {
	StaticEvaluatorNN::evaluateBoard(pos, false);
  }
#endif

  if (depth <= 0) {
#ifdef USE_NN
	if (info->helper_thread) {
	  return QuiescenceSearch(pos, alpha, beta, color, info);
	} else {
	  // hack to disable nn eval in Quiescence search.
	  info->helper_thread = true;
	  int score = color * StaticEvaluatorNN::evaluateBoard(pos, true) * 0.4
		  + QuiescenceSearch(pos, alpha, beta, color, info) * 0.6;
	  info->helper_thread = false;
	  return score;
	}
#endif
	return QuiescenceSearch(pos, alpha, beta, color, info);
  }

  auto ttEntry = _tt->get(pos.hash());
  if (ttEntry->hash == pos.hash() && ttEntry->depth >= depth && !info->is_root) {
	info->ttHits++;
	int scr = ttEntry->eval;
	if (scr >= IMMEDIATE_MATE_SCORE) {
	  scr -= info->ply;
	} else if (scr <= -IMMEDIATE_MATE_SCORE) {
	  scr += info->ply;
	}
	if (ttEntry->type == TTEntryType::EXACT) {
	  return scr;
	} else if (ttEntry->type == TTEntryType::HF_ALPHA) { // means entry was never bigger than alpha.
	  if (scr <= alpha)
		return scr;
	  else
		beta = std::min(beta, scr);
	} else if (ttEntry->type == TTEntryType::HF_BETA) { // means entry was never smaller than beta.
	  if (scr >= beta)
		return scr;
	  else
		alpha = std::max(alpha, scr);
	}
  }

  if (info->null_move && depth >= 4 && !info->is_root && !pos.in_check() && !is_endgame(pos)) {
	pos.makenull();
	info->null_move = false;
	int scr = -negamax(pos, depth - 4, -beta, -beta + 1, -color, info);
	pos.undonull();

	if (scr >= beta) {
	  info->null_cutoffs++;
	  return beta;
	}
  }

  auto _ = pos.legal_moves();
  std::vector<std::pair<libchess::Move, int>> childMoves;
  for (auto &move : _) {
	childMoves.emplace_back(move, 0);
  }

  set_scores(pos, childMoves, color, info);

  info->is_root = false;
  libchess::Move bestMove;
  TTEntry entry{pos.hash(), TTEntryType::EXACT, static_cast<int8_t>(depth), alpha, color, false,
				libchess::Move()};
  for (int i = 0; i < childMoves.size(); i++) {
	sortNextMove(i, childMoves);
	auto &move = childMoves[i];

	if (!is_endgame(pos) && i >= get_prune_move(depth, childMoves.size()))
	  break;

	int moveScore = 0;
	pos.makemove(move.first);
	info->ply++;
	info->null_move = true;
	// if we are searching  with the score of bad captures.
	// late move reductions
	if (i >= 2 && depth >= 7 && !pos.in_check() && !is_endgame(pos)) {
	  moveScore = -negamax(pos, (depth / 2), -beta, -alpha, -color, info);
	  if (moveScore > alpha) {
		moveScore = -negamax(pos, depth - 1, -beta, -alpha, -color, info);
	  }
	} else {
	  moveScore = -negamax(pos, depth - 1, -beta, -alpha, -color, info);
	}
	info->ply--;
	pos.undomove();

	if (info->search_depth == depth) {
	  moves.emplace_back(move.first, moveScore);
	}

	if (info->quit_search) {
	  return 0;
	}

	if (info->helper_thread && info->terminate_helpers) {
	  return 0;
	}
	if (moveScore > score) {
	  score = moveScore;
	  bestMove = move.first;
	}

	if (moveScore > alpha) {
	  if (!move.first.is_capturing())
		info->history[pos.turn()][(int)move.first.piece()][(int)move.first.to()] += depth * depth;
	  alpha = moveScore;
	  entry.eval = moveScore;
	  entry.type = TTEntryType::EXACT;
	  entry.pv = true;
	  entry.mv = move.first;
	}

	if (moveScore >= beta) { /* too good move for opponent*/
	  info->cutt_offs++;
	  if (i == 0) {
		info->fhf++;
	  }
	  info->fh++;
	  if (!move.first.is_capturing()) {
		info->killers[1][info->ply] = info->killers[0][info->ply];
		info->killers[0][info->ply] = move.first;
	  }

	  _tt->add(TTEntry{pos.hash(), TTEntryType::HF_BETA, static_cast<int8_t>(depth), moveScore, color, false,
					   move.first});
	  return beta;
	}
  }

  if (alpha <= alphaOrigin) {
	entry.type = TTEntryType::HF_ALPHA;
	entry.eval = alpha;
	entry.pv = false;
	entry.mv = bestMove;
  }

  _tt->add(entry);

  return alpha;
}

int Engine::QuiescenceSearch(libchess::Position &pos, int alpha, int beta, Color color, SearchInfo *info) noexcept {
  info->ply = info->ply % 100;

  if (info->quit_search)
	return 0;

  if (info->helper_thread && info->terminate_helpers) {
	return 0;
  }
  int score = color * StaticEvaluator::evaluate(pos, info);

  if (!info->helper_thread && info->nodes_searched & 4097) {
	auto cur =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch());
	if (cur > info->search_finish_time) {
	  info->quit_search = true;
	  return 0;
	}
  }

  info->nodes_searched++;

  if (score >= beta) {
	return beta;
  } else if (score > alpha) {
	alpha = score;
  }

  if (pos.is_terminal()) {
	return score;
  }

  if (pos.threefold()) {
	return 0;
  }

  const auto _ = pos.legal_moves();
  std::vector<std::pair<libchess::Move, int>> childMoves;
  for (auto &move : _) {
	if (move.is_capturing() || move.is_promoting() || is_check(pos, move)) {
	  childMoves.emplace_back(move, 0);
	}
  }
  if (childMoves.empty()) {
	return score;
  }

  set_scores(pos, childMoves, color, info, true);

  for (int i = 0; i < childMoves.size(); i++) {
	sortNextMove(i, childMoves);
	auto move = childMoves[i];
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
	  if (i == 0) {
		info->fhf++;
	  }
	  info->fh++;
	  break;
	}
  }

  return score;
}

std::vector<std::pair<libchess::Move, int>> Engine::get_moves(libchess::Position pos,
															  int depth,
															  Color color,
															  SearchInfo *info) noexcept {
  moves.clear();
  if (info->clear_tt_every_move || is_endgame(pos))
	_tt->clear();

  if (_tt->hashfull() >= 950) {
	_tt->clear();

	// add 10s more time in seach
	info->search_finish_time += std::chrono::milliseconds(10000);
  }

  info->ttHits = 0;
  info->search_time = 0;
  info->search_depth = depth;
  info->nodes_searched = 0;
  info->cutt_offs = 0;
  info->null_cutoffs = 0;
  info->quit_search = false;

  int alpha = -INF;
  int beta = INF;

  std::vector<libchess::Move> _ = pos.legal_moves();
  for (int k = 0; k < info->num_helper_threads; k++) {
	auto helperInfo = new SearchInfo();
	helperInfo->helper_thread = true;
	helperInfo->terminate_helpers = false;
	helperInfo->is_root = true;
	helperInfo->sort_moves = true;
	helperInfo->nodes_searched = 0;

	_threads[k]->setSearchInfo(helperInfo);
  }

  std::pair<libchess::Move, int> bestMove;
  for (int i = 1; i <= depth;) {
	auto prevMoves = moves;
	moves.clear();
	info->search_depth = i;
	info->quit_search = false;

	for (int k = 0; k < info->num_helper_threads; k++) {
	  auto helperInfo = _threads[k]->getSearchInfo();
	  _threads[k]->setDepth(i);
	  helperInfo->search_depth = i;
	  helperInfo->terminate_helpers = false;
	  helperInfo->is_root = true;
	  helperInfo->ply = pos.halfmoves();
	  helperInfo->quit_search = false;

	  helperInfo->cutt_offs = 0;
	  helperInfo->null_cutoffs = 0;
	  helperInfo->nodes_searched = 0;
	  helperInfo->ttHits = 0;
	  helperInfo->fh = 0;
	  helperInfo->fhf = 0;

	  _threads[k]->setRootPos(pos);
	  _threads[k]->start_searching(alpha, beta);
	}

	info->helper_thread = false;
	info->is_root = true;
	info->ply = pos.halfmoves();

	info->cutt_offs = 0;
	info->null_cutoffs = 0;
	info->nodes_searched = 0;
	info->ttHits = 0;
	info->fh = 0;
	info->fhf = 0;

	moves.clear();

#ifdef USE_NN
	StaticEvaluatorNN::no_of_nn_calls = 0;
#endif

	std::cout << "Depth: " << i << std::endl;
	auto t1 = std::chrono::high_resolution_clock::now();

	int val = negamax(pos, i, alpha, beta, color, info);

	auto d1 = std::chrono::high_resolution_clock::now();
	long long search_time = std::chrono::duration_cast<std::chrono::milliseconds>(d1 - t1).count();
	long long nodes_searched = info->nodes_searched;
	long long cutoffs = info->cutt_offs;
	long long null_cutoffs = info->null_cutoffs;
	long long fh = info->fh;
	long long fhf = info->fhf;
	long long ttHits = info->ttHits;

	for (int k = 0; k < info->num_helper_threads; k++) {
	  nodes_searched += _threads[k]->getSearchInfo()->nodes_searched;
	  cutoffs += _threads[k]->getSearchInfo()->cutt_offs;
	  null_cutoffs += _threads[k]->getSearchInfo()->null_cutoffs;
	  fh += _threads[k]->getSearchInfo()->fh;
	  fhf += _threads[k]->getSearchInfo()->fhf;
	  ttHits += _threads[k]->getSearchInfo()->ttHits;

	  _threads[k]->wait_for_search_to_finish();
	}
	auto entry = _tt->get(pos.hash());

	std::cout << "info depth " << i << " score cp " << (entry->eval) << " ttHits " << ttHits << " nodes "
			  << nodes_searched << " time " << search_time << " hashfull " << _tt->hashfull() << " nps "
			  << (nodes_searched * 1000 / (search_time + 1))
			  << " cutoff " << cutoffs
			  << " null " << null_cutoffs
			  << " ordering " << (fhf / (float)(fh + 1)) * 100
			  #ifdef USE_NN
			  << " nn_hits " << StaticEvaluatorNN::hits
			  #endif
			  << " pv ";
	print_pv_line(pos, _tt);
	std::cout << std::endl;
	if (info->quit_search) {
	  return {{entry->mv, entry->eval}};
	}
	alpha = -INF;
	beta = INF;

	i++;
  }
  info->search_depth = -1;

  auto entry = _tt->get(pos.hash());
  return {{entry->mv, entry->eval}};
}

void Engine::set_scores(libchess::Position &pos,
						std::vector<std::pair<libchess::Move, int>> &mvs,
						Color color,
						SearchInfo *info,
						bool q) const noexcept {

  auto entry = _tt->get(pos.hash());

#ifdef USE_MSNN
  // TODO: update after better move sorter model.
  if (entry->hash != pos.hash()) {
	MoveSorterNN::scoreMoves(pos, mvs);
  }
#endif

  if (info->is_root && !info->helper_thread && entry->hash != pos.hash())
	for (auto &move : mvs) {
	  std::cout << move.first << " " << move.second << std::endl;
	}

//  exit(0);

  for (auto &move : mvs) {
	move.second = 0;

	if (!q) {
	  if (move.first == entry->mv) {
		move.second += 2000000;
	  }

	  if (move.first == info->killers[0][info->ply]) {
		move.second += VAL_KILLER_0;
	  } else if (move.first == info->killers[1][info->ply]) {
		move.second += VAL_KILLER_1;
	  } else if (!move.first.is_capturing()) {
		move.second += info->history[pos.turn()][(int)move.first.piece()][(int)move.first.to()];
	  }
	}
	move.second += getMVVLVA(move.first);
  }
}

int Engine::getMVVLVA(const libchess::Move &move) {
  if (!move.is_capturing()) {
	return 0;
  }
  int pieceCaptured = move.captured();
  int pieceCapturing = move.piece();
  return MVVLVA_LOOOKUP[pieceCapturing][pieceCaptured] + 1000000;
}

void Engine::sortNextMove(int index, std::vector<std::pair<libchess::Move, int>> &mvs) noexcept {
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

int Engine::get_prune_move(int depth, unsigned long long int size) {
  if (depth <= 4) return size;
  else if (depth <= 6) return size * 0.75;
  else if (depth <= 8) return size * 0.50;
  else if (depth <= 10) return size * 0.25;
  else return 3;
}
}