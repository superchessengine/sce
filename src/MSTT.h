//
// Created by khush on 18-03-2022.
//

#ifndef SCE_SRC_MSTT_H_
#define SCE_SRC_MSTT_H_

#include <cstdint>
#include <cstring>
#include <vector>
#include "constants.h"
#include "move.hpp"

namespace sce {
struct MSTTEntry {
  std::uint64_t hash{};
  std::map<std::uint16_t, std::uint8_t> *moves;
};

class MSTT {
 private:
  std::size_t _size, _filled;
  MSTTEntry *_entries;

  [[nodiscard]] std::size_t index(const std::uint64_t hash) const noexcept {
	return hash % _size;
  }

 public:
  MSTT(unsigned long long mb) : _filled(0) {
	_size = mb * 1024 * 1024 / sizeof(MSTTEntry);
	_entries = new MSTTEntry[_size];
	clear();
  }

  ~MSTT() {
	for(int i = 0; i < _size; i ++) {
	  delete _entries[i].moves;
	}
	delete _entries;
  }

  [[nodiscard]] MSTTEntry *get(std::uint64_t hash) const noexcept {
	return &_entries[index(hash)];
  }

  void add(const MSTTEntry &entry) {
	const auto idx = index(entry.hash);
	_filled += _entries[idx].hash == 0 ? 1 : 0;

	if(_entries[idx].hash != 0) {
	  delete _entries[idx].moves;
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
	for(int i = 0; i < _size; i ++) {
	  delete _entries[i].moves;
	}
	std::memset(_entries, 0, _size * sizeof(MSTTEntry));
  }

  void prefetch(const std::uint64_t hash) const noexcept {
	__builtin_prefetch(&_entries[index(hash)]);
  }
};

}
#endif //SCE_SRC_MSTT_H_
