//
// Created by khushitshah on 2/18/22.
//

#ifndef SCE_SEARCHTHREAD_H
#define SCE_SEARCHTHREAD_H


#include <thread>
#include <mutex>
#include <condition_variable>
#include "SearchInfo.h"
#include "position.hpp"
#include "Engine.fwd.h"

class Engine;

class SearchThread {
private:
    SearchInfo *_searchInfo;
    Engine *_engine;
    int _threadId;
    int _depth;

    int alpha = -INF;
    int beta = INF;

    libchess::Position _rootPos;
    std::thread _thread;

    std::mutex mutex;
    std::condition_variable cv;
    bool exit = false, searching = true;
public:
    SearchThread(Engine *engine) {
        _engine = engine;
        _searchInfo = nullptr;
        _threadId = -1;
        _depth = 0;
        _rootPos = libchess::Position();
        _thread = std::thread(&SearchThread::run, this);
    };

    SearchInfo *getSearchInfo() const {
        return _searchInfo;
    }

    void setSearchInfo(SearchInfo *searchInfo) {
        delete _searchInfo;
        _searchInfo = searchInfo;
    }

    Engine *getEngine() const {
        return _engine;
    }

    void setEngine(Engine *engine) {
        _engine = engine;
    }

    int getThreadId() const {
        return _threadId;
    }

    void setThreadId(int threadId) {
        _threadId = threadId;
    }

    int getDepth() const {
        return _depth;
    }

    void setDepth(int depth) {
        _depth = depth;
    }

    const libchess::Position &getRootPos() const {
        return _rootPos;
    }

    void setRootPos(const libchess::Position rootPos) {
        _rootPos = rootPos;
    }

    SearchThread(SearchInfo *searchInfo, int threadId, int depth, libchess::Position rootPos) {
        _searchInfo = searchInfo;
        _threadId = threadId;
        _depth = depth;
        _rootPos = rootPos;
        _thread = std::thread(&SearchThread::run, this);
    };


    ~SearchThread() {
        exit = true;
        if (_searchInfo != nullptr) {
            _searchInfo->terminate_helpers = true;
        }
        cv.notify_all();
        _thread.join();
    };

    void run() {
        while (true) {
            std::unique_lock<std::mutex> lk(mutex);
            searching = false;
            cv.notify_one();
            cv.wait(lk, [this] { return exit || searching; });
            if (exit) {
                return;
            }
            lk.unlock();
            search();
        }
    }

    void search() {
//        std::cout << "Thread " << _threadId << " started searching with depth: " << _searchInfo->search_depth
//                  << std::endl;
//        print_search_info(_searchInfo);
        _engine->negamax(_rootPos, _searchInfo->search_depth, alpha, beta,
                         _rootPos.turn() == libchess::Side::White ? COLOR_WHITE : COLOR_BLACK, _searchInfo);
//        print_search_info(_searchInfo);
//        std::cout << "Thread " << _threadId << " finished searching " << _searchInfo->nodes_searched << std::endl;
    }

    void wait_for_search_to_finish() {
        std::unique_lock<std::mutex> lk(mutex);
        _searchInfo->terminate_helpers = true;
        cv.wait(lk, [this] { return !searching; });
    }

    void start_searching(int alpha, int beta) {
        this->alpha = alpha;
        this->beta = beta;

        std::lock_guard<std::mutex> lk(mutex);
        searching = true;
        cv.notify_one(); // Wake up the thread in idle_loop()
    }
};

#endif //SCE_SEARCHTHREAD_H
