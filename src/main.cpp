#include <iostream>
#include <algorithm>
#include "constants.h"
#include "TT.h"
#include "position.hpp"
#include "utils.h"
#include "Engine.h"
#include "SearchThread.h"
#include <cstdlib>
#include <ctime>

int TTSize = 500;
int depth = 18;
bool iterative_deepening = true;
bool sort_moves = true;

std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main(int argc, char *argv[]) {
    srand(time(nullptr));

    if (argc >= 2) {
        depth = atoi(argv[1]);
    }

    if (argc >= 3) {
        TTSize = atoi(argv[2]);
    }

    if (argc >= 4) {
        fen = argv[5];
    }


    std::cout << sizeof(TTEntry) << std::endl;

    TT tt(TTSize);
    Engine engine(tt);

    SearchInfo info;
    info.iterative_deepening = iterative_deepening;
    info.sort_moves = sort_moves;
    info.clear_tt_every_move = false;

    info.num_helper_threads = 2;
    libchess::Position position;
    position.set_fen(fen);

    std::cout << "FEN: " << position.get_fen() << std::endl;

    std::cout << "depth: " << depth << " with sorting: " << info.sort_moves << std::endl;

    std::string game = "";
    int cnt = 0;
    while (!position.is_terminal() && !position.threefold()) {
        if (cnt % 2 == 1) {
            std::cout << "Enter the move: " << std::endl;
            std::string mv;
            std::cin >> mv;
            position.makemove(mv);
            game += mv + " ";
            cnt++;
            continue;
        }
        std::cout << std::endl;
        const auto moves = engine.get_moves(position, depth,
                                            position.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK,
                                            &info);

        const auto max = std::max_element(moves.begin(), moves.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.second < rhs.second;
        });


        std::cout << std::endl;
        std::cout << get_board_pretty(position.get_fen()) << std::endl;
        std::cout << max->first << " " << max->second << std::endl;
        position.makemove(max->first);
        std::cout << "MAX: " << get_san(position, max->first) << ":" << max->second << " Searched: "
                  << info.nodes_searched << std::endl << "search time: " << info.search_time / 1000. << "s sort time: "
                  << info.sort_time / 1000. << "s Movegen time:" << info.move_gen_time / 1000. << "s static_eval_time: "
                  << info.static_eval_time / 1000. << "s makemove_time" << info.makemove_time << "s Threefold_time: "
                  << info.threefold_time << "s ttHits: " << info.ttHits << std::endl;

        game += get_san(position, max->first) + " ";
        std::cout << "Game: " << game << std::endl;
        cnt++;
    }
    std::cout << std::endl;
    for (const auto &move: game) {
        std::cout << move << " ";
    }
    std::cout << std::endl;
    return 0;
}
