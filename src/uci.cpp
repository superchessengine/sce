#include "uci.h"
#include "position.hpp"
#include "SearchInfo.h"
#include <iostream>
#include <string>
#include <chrono>


void UCI::init() {
    std::cout << "Engine Started.." << std::endl;
    tt = new TT(500);
    _engine = new Engine(*tt);
    UCI::run();
}

void UCI::run() {
    while (true) {
        std::string line;
        getline(std::cin, line);
        ss.clear();
        ss.str(line);
        std::string token;
        ss >> token;
        if (token == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (token == "position") {
            Position();
        } else if (token == "go") {
            Go();
        } else if (token == "quit") {
            exit(0);
        } else if (token == "uci") {
            std::cout << "id name " << "Super Chess Engine" << std::endl;
            std::cout << "id author " << "SCE Team" << std::endl;
            std::cout << "uciok" << std::endl;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
}

void UCI::Position() {
    std::string token;
    ss >> token;
    if (token == "startpos") {
        _curFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    } else if (token == "fen") {
        ss >> _curFen;
    }

    ss >> token;
    if (token == "moves") {
        libchess::Position pos(_curFen);
        while (ss >> token) {
            pos.makemove(token);
        }
        _curFen = pos.get_fen();
    }
}

void UCI::Go() {
    SearchInfo *info = new SearchInfo();
    libchess::Position pos(_curFen);

    info->search_depth = 10;
    info->num_helper_threads = 2;

//    long long wtime = 100000;
//    long long btime = 100000;
//
//    std::string token;
//    while (ss >> token && (token != "wtime" && token != "btime")) {
//    }
//    if (token == "wtime") {
//        ss >> wtime;
//    } else if (token == "btime") {
//        ss >> btime;
//    }
//    while (ss >> token && (token != "wtime" && token != "btime")) {}
//    if (token == "wtime") {
//        ss >> wtime;
//    } else if (token == "btime") {
//        ss >> btime;
//    }
//
//    if (pos.turn() == libchess::Side::White) {
//        info->search_finish_time = std::chrono::duration_cast<std::chrono::milliseconds>(
//                std::chrono::system_clock::now().time_since_epoch() + std::chrono::milliseconds(wtime / 20)).count();
//    } else {
//        info->search_finish_time = std::chrono::duration_cast<std::chrono::milliseconds>(
//                std::chrono::system_clock::now().time_since_epoch() + std::chrono::milliseconds(btime / 20)).count();
//    }
    const auto moves = _engine->get_moves(pos, info->search_depth,
                                          pos.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK, info);
    auto max = moves[0];
    for (auto m: moves) {
        if (m.second > max.second) {
            max = m;
        }
    }
    std::cout << "bestmove " << max.first << " eval" << std::endl;
}




