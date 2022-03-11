//
// Created by khushitshah on 2/28/22.
//

#ifdef USE_NN
#include "StaticEvaluatorNN.h"

cppflow::model sce::StaticEvaluatorNN::model("se_softmax_final_1");
int sce::StaticEvaluatorNN::no_of_nn_calls = 0;
sce::SETT *sce::StaticEvaluatorNN::_tt = new SETT(SETT_SIZE);
#endif