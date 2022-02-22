#include <chrono>
#include <iostream>
#include "uci.h"

int main(int argc, char *argv[]) {
  srand(time(nullptr));
  sce::UCI uci;
  uci.init();
  return 0;
}