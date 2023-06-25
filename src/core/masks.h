// WhiteCore is a C++ chess engine
// Copyright (c) 2023 Balázs Szilágyi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include "bitboard.h"

namespace core {
    extern Bitboard masks_adjacent_file[64];
    extern Bitboard masks_adjacent_north[64];
    extern Bitboard masks_adjacent_south[64];
    extern Bitboard masks_pawn[64][2];
    extern Bitboard masks_knight[64];
    extern Bitboard masks_king[64];
    extern Bitboard masks_file[64];
    extern Bitboard masks_rank[64];
    extern Bitboard masks_rook[64];
    extern Bitboard masks_diagonal[64];
    extern Bitboard masks_anti_diagonal[64];
    extern Bitboard masks_bishop[64];
    extern Bitboard masks_common_ray[64][64];
    extern LineType line_type[64][64];

    /*
 * Initializes masks. Must be called
 * before calling the move generator.
 */
    inline void init_masks() {

        for (Square sq = A1; sq < 64; sq += 1) {
            masks_bit[sq] = 1ULL << sq;

            masks_pawn[sq][WHITE] = step<NORTH_WEST>(masks_bit[sq]) | step<NORTH_EAST>(masks_bit[sq]);
            masks_pawn[sq][BLACK] = step<SOUTH_WEST>(masks_bit[sq]) | step<SOUTH_EAST>(masks_bit[sq]);

            masks_knight[sq] =
                    step<NORTH>(step<NORTH_WEST>(masks_bit[sq])) | step<NORTH>(step<NORTH_EAST>(masks_bit[sq])) |
                    step<WEST>(step<NORTH_WEST>(masks_bit[sq])) | step<EAST>(step<NORTH_EAST>(masks_bit[sq])) |
                    step<SOUTH>(step<SOUTH_WEST>(masks_bit[sq])) | step<SOUTH>(step<SOUTH_EAST>(masks_bit[sq])) |
                    step<WEST>(step<SOUTH_WEST>(masks_bit[sq])) | step<EAST>(step<SOUTH_EAST>(masks_bit[sq]));

            masks_king[sq] =
                    step<NORTH>(masks_bit[sq]) | step<NORTH_WEST>(masks_bit[sq]) | step<WEST>(masks_bit[sq]) |
                    step<NORTH_EAST>(masks_bit[sq]) |
                    step<SOUTH>(masks_bit[sq]) | step<SOUTH_WEST>(masks_bit[sq]) | step<EAST>(masks_bit[sq]) |
                    step<SOUTH_EAST>(masks_bit[sq]);

            masks_file[sq] = slide<NORTH>(sq) | slide<SOUTH>(sq);

            masks_rank[sq] = slide<WEST>(sq) | slide<EAST>(sq);

            masks_rook[sq] = masks_file[sq] | masks_rank[sq];

            masks_diagonal[sq] = slide<NORTH_EAST>(sq) | slide<SOUTH_WEST>(sq);

            masks_anti_diagonal[sq] = slide<NORTH_WEST>(sq) | slide<SOUTH_EAST>(sq);

            masks_bishop[sq] = masks_diagonal[sq] | masks_anti_diagonal[sq];
        }

        for (Square sq = A1; sq < 64; sq += 1) {
            unsigned int file = square_to_file(sq);

            masks_adjacent_north[sq] = slide<NORTH>(sq) | (file != 0 ? slide<NORTH>(sq + WEST) : 0) |
                                       (file != 7 ? slide<NORTH>(sq + EAST) : 0);
            masks_adjacent_south[sq] = slide<SOUTH>(sq) | (file != 0 ? slide<SOUTH>(sq + WEST) : 0) |
                                       (file != 7 ? slide<SOUTH>(sq + EAST) : 0);
            masks_adjacent_file[sq] =
                    ~masks_file[sq] & (masks_adjacent_north[sq] | masks_adjacent_south[sq] | step<WEST>(sq) | step<EAST>(sq));

            // Calculates the common ray and the line type of the shortest path between sq and sq2.
            for (Square sq2 = A1; sq2 < 64; sq2 += 1) {
                if (sq == sq2)
                    continue;
                for (Direction dir : DIRECTIONS) {
                    Bitboard value = slide(dir, sq) & slide(-dir, sq2);

                    if (value) {
                        masks_common_ray[sq][sq2] = value;
                        LineType type = HORIZONTAL;
                        switch (dir) {
                            case NORTH:
                            case SOUTH:
                                type = HORIZONTAL;
                                break;
                            case WEST:
                            case EAST:
                                type = VERTICAL;
                                break;
                            case NORTH_EAST:
                            case SOUTH_WEST:
                                type = DIAGONAL;
                                break;
                            case NORTH_WEST:
                            case SOUTH_EAST:
                                type = ANTI_DIAGONAL;
                                break;
                        }
                        line_type[sq][sq2] = type;
                        break;
                    }
                }
            }
        }
    }
} // namespace core