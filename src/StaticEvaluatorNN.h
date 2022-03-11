//
// Created by khushitshah on 2/28/22.
//

#ifdef USE_NN
#ifndef SCE_STATICEVALUATORNN_H
#define SCE_STATICEVALUATORNN_H

#include "cppflow.h"
#include <iostream>
#include "position.hpp"
#include "constants.h"
#include "utils.h"
#include <math.h>
#include "SETT.h"

namespace sce {
class StaticEvaluatorNN {
  static cppflow::model model;
  constexpr static const float white_move[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  constexpr static const float black_move[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  static SETT *_tt;
 public:

  static std::vector<float> evaluate(float *input_arr, int cnt) {

//	 for (auto a : model.get_operations()) {
//	   std::cout << a << std::endl;
//	 }

	no_of_nn_calls++;
	auto input = cppflow::tensor(std::vector<float>(input_arr, input_arr + cnt * 9 * 8 * 12), {cnt, 9, 8, 12});

	auto output = model({{"serving_default_conv2d_6_input", input}}, {{"StatefulPartitionedCall"}});

//	std::cout << output.size() << " " << output[0].shape() << std::endl;

	auto outputTensor = (float *)TF_TensorData(output[0].get_tensor().get());
	std::vector<float> ret;
	for (int i = 0; i < cnt; i++) {
	  if (outputTensor[i*cnt + 1] > outputTensor[i*cnt + 0]
		  && outputTensor[i*cnt + 1] > outputTensor[i*cnt + 2]) {
		ret.push_back(0);
	  } else {
		ret.push_back(1000 * (outputTensor[i*cnt + 2] - outputTensor[i*cnt + 0]));
	  }
	}
	return ret;
  }

  static float evaluateBoard(libchess::Position position) {

	auto entry = _tt->get(position.hash());
	if(entry->hash == position.hash()) {
	  return entry->eval;
	}

	// else we want to evaluate this board and all the moves in current position.

	int cnt = 1;
	auto moves =  position.legal_moves();

	std::vector<libchess::Move> captures;

	for(auto move : moves) {
	  if(move.is_capturing() || move.is_promoting()) {
		captures.push_back(move);
	  }
	}

	moves = captures;

	cnt += moves.size();

	float input_arr[cnt][9][8][12];

	for (int i = 0; i < cnt; i++) {
	  for (int j = 0; j < 9; j++) {
		for (int k = 0; k < 8; k++) {
		  for (int l = 0; l < 12; l++) {
			input_arr[i][j][k][l] = 0;
		  }
		}
	  }
	}

	for(int i = 0; i < cnt; i ++) {
	  if(i == 0) {
		fill_input_arr(input_arr,  i, position);
	  } else {
		position.makemove(moves[i - 1]);
		fill_input_arr(input_arr, i, position);
	  	position.undomove();
	  }
	}

	std::vector<float> scr = evaluate(&input_arr[0][0][0][0], cnt);

	for(int i = 0; i < cnt; i ++) {
	  if(i == 0) {
		if(position.turn() != libchess::Side::White) {
		  scr[i] *= -1;
		}
		_tt->add({position.hash(), int(scr[i])});
 	  }
	  else {
		position.makemove(moves[i - 1]);
		if (position.turn() != libchess::Side::White) {
		  scr[i] *= -1;
		}
		_tt->add({position.hash(), int(scr[i])});
	  }
	}
	return scr[0];
  }

  static void fill_input_arr(float input_arr[][9][8][12], int cntIndex, const libchess::Position &position) {
	int index = 0;
	std::string board_str =
		get_board(position.get_fen()) + (position.turn() == libchess::Side::White ? "00000000" : "11111111");

	assert(board_str.length() == 72);

	for (int i = 0; i < 9; i ++) {
	  for (int j = 0; j < 8; j ++) {
		if (board_str[index] == '1') {
		  memcpy(input_arr[cntIndex][i][j], black_move, 12 * sizeof(float));
		} else if (board_str[index] == '0') {
		  memcpy(input_arr[cntIndex][i][j], white_move, 12 * sizeof(float));
		} else if (board_str[index] == 'p') {
		  input_arr[cntIndex][i][j][0] = 1;
		} else if (board_str[index] == 'n') {
		  input_arr[cntIndex][i][j][1] = 1;
		} else if (board_str[index] == 'b') {
		  input_arr[cntIndex][i][j][2] = 1;
		} else if (board_str[index] == 'r') {
		  input_arr[cntIndex][i][j][3] = 1;
		} else if (board_str[index] == 'q') {
		  input_arr[cntIndex][i][j][4] = 1;
		} else if (board_str[index] == 'k') {
		  input_arr[cntIndex][i][j][5] = 1;
		} else if (board_str[index] == 'P') {
		  input_arr[cntIndex][i][j][6] = 1;
		} else if (board_str[index] == 'N') {
		  input_arr[cntIndex][i][j][7] = 1;
		} else if (board_str[index] == 'B') {
		  input_arr[cntIndex][i][j][8] = 1;
		} else if (board_str[index] == 'R') {
		  input_arr[cntIndex][i][j][9] = 1;
		} else if (board_str[index] == 'Q') {
		  input_arr[cntIndex][i][j][10] = 1;
		} else if (board_str[index] == 'K') {
		  input_arr[cntIndex][i][j][11] = 1;
		}
		index++;
	  }
	}
  }

  static void init() {
//	_tt = new SETT(SETT_SIZE);
//	model = cppflow::model("se_softmax_final_1");
	for (auto x : model.get_operations()) {
	  std::cout << x << std::endl;
	}
  }
  static int no_of_nn_calls;
};
}

#endif //SCE_STATICEVALUATORNN_H

#endif //USE_NN