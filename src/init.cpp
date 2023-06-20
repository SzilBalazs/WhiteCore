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

#include "utils/logger.h"
#include "core/magic.h"
#include "core/masks.h"

// Declarations
Bitboard masks_bit[64], masks_adjacent_file[64], masks_adjacent_north[64], masks_adjacent_south[64], masks_pawn[64][2],
		 masks_knight[64], masks_king[64], masks_file[64], masks_rank[64], masks_rook[64], masks_diagonal[64],
		 masks_anti_diagonal[64], masks_bishop[64], masks_common_ray[64][64];

LineType line_type[64][64];

Bitboard attack_table_rook[102400], attack_table_bishop[5248];

void init_all() {
	logger.init("log.txt");
	logger.info("Logger initialized!");

	init_masks();
	logger.info("Bitboard masks initialized!");

	init_magic();
	logger.info("Magic initialized!");
}