//
// Created by khushitshah on 2/16/22.
//

#ifndef SETT_H
#define SETT_H

#include <cstdint>
#include <cstring>
#include "constants.h"
#include "move.hpp"

namespace sce {
struct SETTEntry {
  std::uint64_t hash{};
  int eval{};
};

class SETT {
 private:
  std::size_t _size, _filled;
  SETTEntry *_entries;

  [[nodiscard]] std::size_t index(const std::uint64_t hash) const noexcept {
	return hash % _size;
  }

 public:
  SETT(unsigned long long mb) : _filled(0) {
	_size = mb * 1024 * 1024 / sizeof(SETTEntry);
	_entries = new SETTEntry[_size];
	clear();
  }

  ~SETT() {
	delete _entries;
  }

  [[nodiscard]] SETTEntry *get(std::uint64_t hash) const noexcept {
	return &_entries[index(hash)];
  }

  void add(const SETTEntry &entry) {
	const auto idx = index(entry.hash);
	_filled += _entries[idx].hash == 0 ? 1 : 0;

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
	std::memset(_entries, 0, _size * sizeof(SETTEntry));
  }

  void prefetch(const std::uint64_t hash) const noexcept {
	__builtin_prefetch(&_entries[index(hash)]);
  }
};

}
#endif //SETT_H
