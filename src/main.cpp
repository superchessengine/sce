#include <iostream>
#include <algorithm>
#include "constants.h"
#include "TT.h"
#include "position.hpp"
#include "utils.h"
#include "Engine.h"

const int TTSize = 150;
const int DEPTH = 8;

int main() {
    std::cout << sizeof(TTEntry) << std::endl;

    TT tt(TTSize);
    Engine engine(tt);
    engine.iterative_deepening = true;
    engine.sort_moves = true;

    libchess::Position position;
    position.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "FEN: " << position.get_fen() << std::endl;

    std::cout << "depth: " << DEPTH << " with sorting: " << engine.sort_moves << std::endl;

    std::vector<std::string> game;
    while (!position.is_terminal() && !position.threefold()) {
        std::cout << std::endl;
        const auto moves = engine.get_moves(position, DEPTH,
                                            position.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK);

//        for (const auto &move: moves) {
//            std::cout << move.first << ":" << move.second << std::endl;
//        }
        const auto max = std::max_element(moves.begin(), moves.end(),
                                          [](const auto &lhs, const auto &rhs) {
                                              return lhs.second < rhs.second;
                                          });


        position.makemove(max->first);
        std::cout << "MAX: " << get_san(position, max->first) << ":" << max->second << " Searched: "
                  << engine.nodes_searched << std::endl
                  << "search time: " << engine.search_time / 1000. << "s sort time: " << engine.sort_time / 1000.
                  << "s Movegen time:" << engine.move_gen_time / 1000. << "s static_eval_time: "
                  << engine.static_eval_time / 1000. << "s makemove_time" << engine.makemove_time
                  << "s Threefold_time: " << engine.threefold_time << "s ttHits: " << engine.ttHits << std::endl;

        game.emplace_back(get_san(position, max->first));
    }

    for (const auto &move: game) {
        std::cout << move << " ";
    }
    std::cout << std::endl;
//    position.history();
    return 0;
}
