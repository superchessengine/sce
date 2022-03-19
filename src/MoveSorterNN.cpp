//
// Created by khushitshah on 2/28/22.
//

#ifdef USE_MSNN
#include "MoveSorterNN.h"

cppflow::model sce::MoveSorterNN::model("move_sorter_final");
int sce::MoveSorterNN::no_of_nn_calls = 0;
int sce::MoveSorterNN::hits = 0;
sce::MSTT *sce::MoveSorterNN::_tt = new MSTT(MSTT_SIZE);

#endif