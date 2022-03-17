#include "uci.h"
#include "position.hpp"
#include "SearchInfo.h"
#include <iostream>
#include <string>
#include "Engine.h"
#include "SearchThread.h"
#include "utils.h"

namespace sce
{

	void UCI::init()
	{
		tt = new TT(MAX_TT_SIZE);
		_engine = new Engine(tt);
		UCI::runLoop();
	}

	void UCI::runLoop()
	{
		while (true)
		{
			std::string line;
			getline(std::cin, line);
			ss.clear();
			std::cout << "debug: " << line << std::endl;
			ss.str(line);
			std::string token;
			ss >> token;
			if (token == "isready")
			{
				std::cout << "readyok" << std::endl;
			}
			else if (token == "position")
			{
				getPosition();
			}
			else if (token == "go")
			{
				go();
			}
			else if (token == "quit")
			{
				exit(0);
			}
			else if (token == "uci")
			{
				std::cout << "id name "
						  << "Super Chess Engine" << std::endl;
				std::cout << "id author "
						  << "SCE Team" << std::endl;
				std::cout << "uciok" << std::endl;
			} else if (token == "ucinewgame") {
				tt->clear();
			} else if(token == "strength") {
				ss >> strength;
			}
			else
			{
				//   std::cout << "Unknown command" << std::endl;
			}
		}
	}

	void UCI::getPosition()
	{
		std::string token;
		ss >> token;
		if (token == "startpos")
		{
			_curFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		}
		else if (token == "fen")
		{
			ss >> _curFen;
		}
		_curPos.set_fen(_curFen);
		ss >> token;
		if (token == "moves")
		{
			while (ss >> token)
			{
				_curPos.makemove(token);
			}
			_curFen = _curPos.get_fen();
		}
	}

	void UCI::go()
	{
		auto *info = new SearchInfo();
		//   std::cout << _curFen << std::endl;
		libchess::Position pos = _curPos;
		//   std::cout << get_board_pretty(pos.get_fen()) << std::endl << pos.turn() << std::endl;

		long long wtime = INTMAX_MAX; // 400s is the time by default (20s to move)
		long long btime = INTMAX_MAX; // 400s is the time by default (20s to move).
		int movestogo = 30;

		std::string token;
		while (ss >> token)
		{
			if (token == "wtime")
			{
				ss >> wtime;
			}
			else if (token == "btime")
			{
				ss >> btime;
			}
			else if (token == "movestogo")
			{
				ss >> movestogo;
			}
			else if (token == "movetime")
			{
				ss >> wtime;
				btime = wtime;
			}
		}

		// save the search start time for time management
		if(strength == 3)
			info->search_depth = MAX_DEPTH;
		else if (strength == 2) 
			info->search_depth = 5;
		else if (strength == 1) 
			info->search_depth = 3;
		else  {
			std::cout << "Using strength = 3 as default, given strength: " << strength << endl;
			info->search_depth = MAX_DEPTH;
		}
		
		info->num_helper_threads = MAX_THREADS;
		info->clear_tt_every_move = false;
		info->starttime =
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

		info->search_finish_time =
			info->starttime + std::chrono::milliseconds(((pos.turn() ? btime : wtime) / (movestogo + 1)) - 40);

		const auto moves = _engine->get_moves(pos, info->search_depth,
											  pos.turn() ? COLOR_BLACK : COLOR_WHITE, info);
		auto mx = moves[0];
		for (auto m : moves)
		{
			if (m.second > mx.second)
			{
				mx = m;
			}
		}

		auto lmoves = pos.legal_moves();
		bool found = false;
		for (auto m : lmoves)
		{
			if (m == mx.first)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			mx = std::make_pair(lmoves[0], 0);
		}

		std::cout << "bestmove " << mx.first << std::endl;
	}
}