//
// Created by khushitshah on 2/15/22.
//

#ifndef SCE_CONSTANTS_H
#define SCE_CONSTANTS_H

#include <cassert>

typedef int Color;
const Color COLOR_WHITE = 1;
const Color COLOR_BLACK = -1;

const int INF = 1000000000;
const int IMMEDIATE_MATE_SCORE = 9999999;
const int VAL_WINDOW = 50;

// TODO: look up better values, right now piece values are high to discourage sacrifice for positional advantage.
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 350;
const int BISHOP_VALUE = 350;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;

const int PAWN_INC[] = {0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 50, 50, 50, 50, 50, 50, 10, 10, 20, 30, 30, 20, 10, 10,
                        5, 5, 10, 25, 25, 10, 5, 5, 0, 0, 0, 20, 20, 0, 0, 0, 5, -5, -10, 0, 0, -10, -5, 5, 5,
                        10, 10, -20, -20, 10, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0};

const int KNIGHT_INC[] = {-50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0, 0, 0, 0, -20, -40, -30, 0, 10,
                          15, 15, 10, 0, -30, -30, 5, 15, 20, 20, 15, 5, -30, -30, 0, 15, 20, 20, 15, 0, -30,
                          -30, 5, 10, 15, 15, 10, 5, -30, -40, -20, 0, 5, 5, 0, -20, -40, -50, -40, -30, -30,
                          -30, -30, -40, -50};

const int BISHOP_INC[] = {-20, -10, -10, -10, -10, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 10, 10,
                          5, 0, -10, -10, 5, 5, 10, 10, 5, 5, -10, -10, 0, 10, 10, 10, 10, 0, -10, -10, 10, 10,
                          10, 10, 10, 10, -10, -10, 5, 0, 0, 0, 0, 5, -10, -20, -10, -10, -10, -10, -10, -10,
                          -20};

const int ROOK_INC[] = {0, 0, 0, 0, 0, 0, 0, 0, 5, 10, 10, 10, 10, 10, 10, 5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,
                        0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0, 0, 0, -5, -5, 0, 0, 0, 0,
                        0, 0, -5, 0, 0, 0, 5, 5, 0, 0, 0};

const int QUEEN_INC[] = {-20, -10, -10, -5, -5, -10, -10, -20, -10, 0, 0, 0, 0, 0, 0, -10, -10, 0, 5, 5, 5, 5,
                         0, -10, -5, 0, 5, 5, 5, 5, 0, -5, 0, 0, 5, 5, 5, 5, 0, -5, -10, 5, 5, 5, 5, 5, 0, -10,
                         -10, 0, 5, 0, 0, 0, 0, -10, -20, -10, -10, -5, -5, -10, -10, -20};

const int KING_INC[] = {-30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30,
                        -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -20, -30,
                        -30, -40, -40, -30, -30, -20, -10, -20, -20, -20, -20, -20, -20, -10, 20, 20, 0, 0, 0,
                        0, 20, 20, 20, 30, 10, 0, 0, 10, 30, 20};

const int KING_ENDGAME_INC[] = {-50, -40, -30, -20, -20, -30, -40, -50, -30, -20, -10, 0, 0, -10, -20, -30, -30,
                                -10, 20, 30, 30, 20, -10, -30, -30, -10, 30, 40, 40, 30, -10, -30, -30, -10, 30,
                                40, 40, 30, -10, -30, -30, -10, 20, 30, 30, 20, -10, -30, -30, -30, 0, 0, 0, 0,
                                -30, -30, -50, -30, -30, -30, -30, -30, -30, -50};


// MVV/LVA constants
// 0-P, 1-N, 2-B, 3-R, 4-Q, 5-K
const int MVVLVA_LOOOKUP[6][6] = {{6002, 20225, 20250, 20400, 20800, 26900},
                                  {4775, 6004,  20025, 20175, 20575, 26675},
                                  {4750, 4975,  6006,  20150, 20550, 26650},
                                  {4600, 4825,  4850,  6008,  20400, 26500},
                                  {4200, 4425,  4450,  4600,  6010,  26100},
                                  {3100, 3325,  3350,  3500,  3900,  26000}};


/**
 * returns the index of the piece table score for the given index and color.
 * @param index the index for white.
 * @param color color of the piece.
 * @return
 */
inline int get_index(const int &index, const int &color) noexcept {
    if (color == COLOR_WHITE)
        return index;

    else if (color == COLOR_BLACK)
        return (7 - index / 8) * 8 + index % 8;

    assert(false);
    return 0;
}


#endif //SCE_CONSTANTS_H
