//
// Created by khushitshah on 2/16/22.
//

#ifndef SCE_TT_H
#define SCE_TT_H

#include <cstdint>
#include <cstring>
#include "constants.h"

enum TTEntryType {
    EXACT = 0, LOWER_BOUND = 1, UPPER_BOUND = 2
};

struct TTEntry {
    std::uint64_t hash;
    TTEntryType type;
    std::int8_t depth;
    int eval;
    Color color;
};

class TT {
private:
    std::size_t _size, _filled;
    TTEntry *_entries;

    [[nodiscard]] std::size_t index(const std::uint64_t hash) const noexcept {
        return hash % _size;
    }

public:
    TT(unsigned long long mb) : _filled(0) {
        _size = mb * 1024 * 1024 / sizeof(TTEntry);
        _entries = new TTEntry[_size];
        clear();
    }

    ~TT() {
        delete _entries;
    }

    [[nodiscard]] TTEntry *get(std::uint64_t hash) {
        return &_entries[index(hash)];
    }

    void add(const TTEntry &entry) {
        const auto idx = index(entry.hash);
        _filled += _entries[idx].hash == 0 ? 1 : 0;
        if (_entries->hash == entry.hash && _entries->depth >= entry.depth) {
            return;
        }
        _entries[idx] = entry;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return _size;
    }

    [[nodiscard]] int hashfull() const noexcept {
        return 1000 * (static_cast<double>(_filled) / _size);
    }

    void clear() noexcept {
        _filled = 0;
        std::memset(_entries, 0, _size * sizeof(TTEntry));
    }

    void prefetch(const std::uint64_t hash) const noexcept {
        __builtin_prefetch(&_entries[index(hash)]);
    }
};


#endif //SCE_TT_H
