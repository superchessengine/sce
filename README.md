# SuperChessEngine

website: superchessengine.github.io

### Installation

- Build `libchess` library.

```
cd libs/libchess
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release  ..
make
```

- Now build `SuperChessEngine`, go back to root dir of the repository.

```
mkdir build
cd build
cmake  -DCMAKE_BUILD_TYPE=Release ..
make
```

- `SuperChessEngine` executable will be created in the build directory.
