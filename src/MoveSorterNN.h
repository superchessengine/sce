//
// Created by khushitshah on 2/28/22.
//

#ifdef USE_MSNN
#ifndef SCE_MOVESORTERNN_H
#define SCE_MOVESORTERNN_H

#include "cppflow.h"
#include <iostream>
#include "position.hpp"
#include "constants.h"
#include "utils.h"
#include "MSTT.h"

namespace sce {
class MoveSorterNN {
  static cppflow::model model;
  constexpr static const float white_move[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  constexpr static const float black_move[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  static MSTT *_tt;
 public:

  static void evaluate(float *input_arr, int cnt, uint8_t op[][1970]) {
	no_of_nn_calls++;

	auto input = cppflow::tensor(std::vector<float>(input_arr, input_arr + cnt * 9 * 8 * 12), {cnt, 9 * 8 * 12});

	auto output = model({{"serving_default_dense_input", input}}, {{"StatefulPartitionedCall"}});

	std::cout << output[0].shape() << std::endl;

	auto outputTensor = (double *)TF_TensorData(output[0].get_tensor().get());

	for (int i = 0; i < cnt; i++) {
	  for (int j = 0; j < 1970; j++) {
		op[i][j] = (int)outputTensor[i * 1970 + j] * (254);
	  }
	}
  }

  static void assign_scores(std::vector<std::pair<libchess::Move, int>> &moves_, uint8_t scores[1970]) {
	for (auto move : moves_) {
	  int index = sce::moveToIndex(move.first);

	  assert(index >= 0);

	  // assuming moves are scored based on nn first then all other methods, we can set the second directly here.
	  move.second = scores[index];
	}
  }

  static void get_nn_op_from_map(std::map<uint16_t, uint8_t> mp_, uint8_t nnOp[1970]) {
	for (int i = 0; i < 1970; i++) {
	  // default value will be zero if the key is not found in the map.
	  nnOp[i] = mp_[i];
	}
  }

  static void scoreMoves(libchess::Position position, std::vector<std::pair<libchess::Move, int>> &moves_) {

	auto entry = _tt->get(position.hash());

	if (entry->hash == position.hash()) {
	  hits++;

	  uint8_t nn_output[1970];

	  get_nn_op_from_map(*entry->moves, nn_output);

	  assign_scores(moves_, nn_output);
	}

	// else we want to evaluate this board and all the moves in current position.
	int cnt = 1;
	auto moves = position.legal_moves();

	cnt += moves.size();

	float input_arr[cnt][9 * 8 * 12];

	for (int i = 0; i < cnt; i++) {
	  for (int j = 0; j < 9; j++) {
		for (int k = 0; k < 8; k++) {
		  for (int l = 0; l < 12; l++) {
			input_arr[i][j * 8 * 12 + k * 12 + l] = 0;
		  }
		}
	  }
	}

	for (int i = 0; i < cnt; i++) {
	  if (i == 0) {
		fill_input_arr(input_arr, i, position);
	  } else {
		position.makemove(moves[i - 1]);
		fill_input_arr(input_arr, i, position);
		position.undomove();
	  }
	}

	uint8_t moveScore[cnt][1970];
	evaluate(&input_arr[0][0], cnt, moveScore);

	for (int i = 0; i < cnt; i++) {
	  if (i == 0) {
		MSTTEntry entry;

		entry.hash = position.hash();
		entry.moves = new std::map<uint16_t , uint8_t>();

		// only store values of all the valid moves in given position in the transposition table.
		auto curMoves = position.legal_moves();

		for (auto move : curMoves) {
		  (*entry.moves)[moveToIndex(move)] = moveScore[i][moveToIndex(move)];
		}

		assign_scores(moves_, moveScore[i]);

		_tt->add(entry);
	  } else {
		position.makemove(moves[i - 1]);

		MSTTEntry entry;

		entry.hash = position.hash();
		entry.moves = new std::map<uint16_t, uint8_t>();

		auto curMoves = position.legal_moves();

		for (auto move : curMoves) {
		  (*entry.moves)[moveToIndex(move)] = moveScore[i][moveToIndex(move)];
		}

		_tt->add(entry);

		position.undomove();
	  }
	}
  }

  static void fill_input_arr(float input_arr[][9 * 8 * 12], int cntIndex, const libchess::Position &position) {
	int index = 0;
	std::string board_str =
		get_board(position.get_fen()) + (position.turn() == libchess::Side::White ? "00000000" : "11111111");

	assert(board_str.length() == 72);

	for (int i = 0; i < 9; i++) {
	  for (int j = 0; j < 8; j++) {
		if (board_str[index] == '1') {
		  memcpy(input_arr[cntIndex] + (i * 8 * 12 + j * 12), black_move, 12 * sizeof(float));
		} else if (board_str[index] == '0') {
		  memcpy(input_arr[cntIndex] + (i * 8 * 12 + j * 12), white_move, 12 * sizeof(float));
		} else if (board_str[index] == 'p') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 0] = 1;
		} else if (board_str[index] == 'n') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 1] = 1;
		} else if (board_str[index] == 'b') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 2] = 1;
		} else if (board_str[index] == 'r') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 3] = 1;
		} else if (board_str[index] == 'q') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 4] = 1;
		} else if (board_str[index] == 'k') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 5] = 1;
		} else if (board_str[index] == 'P') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 6] = 1;
		} else if (board_str[index] == 'N') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 7] = 1;
		} else if (board_str[index] == 'B') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 8] = 1;
		} else if (board_str[index] == 'R') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 9] = 1;
		} else if (board_str[index] == 'Q') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 10] = 1;
		} else if (board_str[index] == 'K') {
		  input_arr[cntIndex][i * 8 * 12 + j * 12 + 11] = 1;
		}
		index++;
	  }
	}
	// print the input_arr[cntIndex] for debugging.
//	for(int i = 0; i < 9; i ++) {
//	  for(int j = 0; j < 8; j ++) {
//		for(int k = 0; k < 12; k ++) {
//		  std::cout << input_arr[cntIndex][i*8*12 + j*12 + k];
//		}
//		std::cout << ", ";
//	  }
//	  std::cout << std::endl;
//	}
  }

  static void init() {
	for (auto x : model.get_operations()) {
	  std::cout << x << std::endl;
	}
  }

  static bool findAndGet(const libchess::Position &pos, std::vector<std::pair<libchess::Move, int>> &moves_) {
	auto entry = _tt->get(pos.hash());

	if (entry->hash == pos.hash()) {

	  uint8_t nnOp[1970];

	  get_nn_op_from_map(*entry->moves, nnOp);

	  assign_scores(moves_, nnOp);

	  return true;
	}
	return false;
  }

  static int no_of_nn_calls;
  static int hits;
};
}

#endif //SCE_MOVESORTERNN_H

#endif //USE_NN