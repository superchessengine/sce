//
// Created by khushitshah on 2/28/22.
//

#ifndef SCE_STATICEVALUATORNN_H
#define SCE_STATICEVALUATORNN_H

#include "cppflow.h"
#include <iostream>
#include "position.hpp"
#include "src/constants.h"
#include "src/utils.h"
#include <math.h>

namespace sce {
class StaticEvaluatorNN {
  static cppflow::model model;
  constexpr static const float white_move[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  constexpr static const float black_move[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

 public:

  static float evaluate(float input_arr[9][8][12]) {
//
//	 for (auto a : model.get_operations()) {
//	   std::cout << a << std::endl;
//	 }

	no_of_nn_calls++;
	float _input[9 * 8 * 12];
	memcpy(_input, input_arr, sizeof(float) * 9 * 8 * 12);

	auto input = cppflow::tensor(std::vector<float>(_input, _input + 9 * 8 * 12), {9, 8, 12});
	input = cppflow::expand_dims(input, 0);

	auto output = model({{"serving_default_conv2d_6_input", input}}, {{"StatefulPartitionedCall"}});

	if (output[0].get_data<float>()[1] > output[0].get_data<float>()[0]
		&& output[0].get_data<float>()[1] > output[0].get_data<float>()[2]) {
	  return 0;
	}

	float scr = output[0].get_data<float>()[2] - output[0].get_data<float>()[0];

	return 1000 * scr;
  }

  static float evaluateBoard(const libchess::Position &position) {
	float input_arr[9][8][12] = {0};
	int index = 0;
	std::string board_str =
		get_board(position.get_fen()) + (position.turn() == libchess::Side::White ? "00000000" : "11111111");

	assert(board_str.length() == 72);

	for (auto &i : input_arr) {
	  for (auto &j : i) {
		if (board_str[index] == '1') {
		  memcpy(j, black_move, 12 * sizeof(float));
		} else if (board_str[index] == '0') {
		  memcpy(j, white_move, 12 * sizeof(float));
		} else if (board_str[index] == 'p') {
		  j[0] = 1;
		} else if (board_str[index] == 'n') {
		  j[1] = 1;
		} else if (board_str[index] == 'b') {
		  j[2] = 1;
		} else if (board_str[index] == 'r') {
		  j[3] = 1;
		} else if (board_str[index] == 'q') {
		  j[4] = 1;
		} else if (board_str[index] == 'k') {
		  j[5] = 1;
		} else if (board_str[index] == 'P') {
		  j[6] = 1;
		} else if (board_str[index] == 'N') {
		  j[7] = 1;
		} else if (board_str[index] == 'B') {
		  j[8] = 1;
		} else if (board_str[index] == 'R') {
		  j[9] = 1;
		} else if (board_str[index] == 'Q') {
		  j[10] = 1;
		} else if (board_str[index] == 'K') {
		  j[11] = 1;
		}
		index++;
	  }
	}
	float scr = evaluate(input_arr);

	// score should be negative if black is winning in this position and positive if white is winning
	if (position.turn() == libchess::Side::White) {
	  // white to play, and score is positive, white is winning, if score is negative black is winning. 
	  return scr;
	} else {
	  // black to play, and score is negative, white is winning, if score is positive black is winning. 
	  return -scr;
	}
  }

  static void init() {
	model = cppflow::model("se_softmax_final_1");
	for (auto x : model.get_operations()) {
	  std::cout << x << std::endl;
	}
  }
  static int no_of_nn_calls;
};
}

#endif //SCE_STATICEVALUATORNN_H

