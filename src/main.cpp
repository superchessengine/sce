#include <iostream>
#include <algorithm>
#include "constants.h"
#include "position.hpp"
#include "StaticEvaluator.h"
#include "utils.h"
#include "Engine.h"

int main() {

    Engine engine;

    std::cout << "Hello, World!" << std::endl;
    libchess::Position position;
    position.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "FEN: " << position.get_fen() << std::endl;

    engine.sort_moves = false;
    std::vector<std::string> game;
    while (!position.is_terminal() && !position.threefold()) {
        const auto moves = engine.get_moves(position, 6,
                                            position.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK);

//        for (const auto &move: moves) {
//            std::cout << move.first << ":" << move.second << std::endl;
//        }
        const auto max = std::max_element(moves.begin(), moves.end(),
                                          [](const auto &lhs, const auto &rhs) {
                                              return lhs.second < rhs.second;
                                          });
        std::cout << "MAX: " << max->first << ":" << max->second << " Searched: " << engine.nodes_searched << std::endl
                  << "search time: " << engine.search_time / 1000. << "s sort time: " << engine.sort_time / 1000.
                  << "s Movegen time:" << engine.move_gen_time / 1000. << "s static_eval_time: "
                  << engine.static_eval_time / 1000. << "s makemove_time" << engine.makemove_time
                  << "s Threefold_time: " << engine.threefold_time << "s " << std::endl;


        position.makemove(max->first);
        game.emplace_back(get_san(position, max->first));
    }

    for (const auto &move: game) {
        std::cout << move << " ";
    }
    std::cout << std::endl;
//    position.history();
    return 0;
}
