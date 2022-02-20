#ifndef SCE_UCI_HPP
#define SCE_UCI_HPP

#include<string>
#include <sstream>
#include "Engine.h"

class UCI {
private:
    void run();

    void Position();

    void Go();

    std::string _curFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Engine *_engine;
    TT *tt;
    std::stringstream ss;
public:
    void init();

};

#endif