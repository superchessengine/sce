#include <iostream>
#include <algorithm>
#include "constants.h"
#include "TT.h"
#include "position.hpp"
#include "utils.h"
#include "Engine.h"

int TTSize = 100;
int depth = 7;
bool iterative_deepening = true;
bool sort_moves = true;
std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        depth = atoi(argv[1]);
    }

    if (argc >= 3) {
        TTSize = atoi(argv[2]);
    }

    if (argc >= 4) {
        iterative_deepening = atoi(argv[3]);
    }

    if (argc >= 5) {
        sort_moves = atoi(argv[4]);
    }

    if (argc >= 6) {
        fen = argv[5];
    }


    std::cout << sizeof(TTEntry) << std::endl;

    TT tt(TTSize);
    Engine engine(tt);

    engine.sort_moves = sort_moves;
    engine.iterative_deepening = iterative_deepening;

    libchess::Position position;
    position.set_fen(fen);

    std::cout << "FEN: " << position.get_fen() << std::endl;

    std::cout << "depth: " << depth << " with sorting: " << engine.sort_moves << std::endl;

    std::vector<std::string> game;
    while (!position.is_terminal() && !position.threefold()) {
        std::cout << std::endl;
        const auto moves = engine.get_moves(position, depth,
                                            position.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK);

        const auto max = std::max_element(moves.begin(), moves.end(),
                                          [](const auto &lhs, const auto &rhs) {
                                              return lhs.second < rhs.second;
                                          });

//        print_pv_line(position, tt);

        std::cout << std::endl;

        position.makemove(max->first);
        std::cout << "MAX: " << get_san(position, max->first) << ":" << max->second << " Searched: "
                  << engine.nodes_searched << std::endl
                  << "search time: " << engine.search_time / 1000. << "s sort time: " << engine.sort_time / 1000.
                  << "s Movegen time:" << engine.move_gen_time / 1000. << "s static_eval_time: "
                  << engine.static_eval_time / 1000. << "s makemove_time" << engine.makemove_time
                  << "s Threefold_time: " << engine.threefold_time << "s ttHits: " << engine.ttHits << std::endl;

        game.emplace_back(get_san(position, max->first));
    }
    std::cout << std::endl;
    for (const auto &move: game) {
        std::cout << move << " ";
    }
    std::cout << std::endl;
    return 0;
}
