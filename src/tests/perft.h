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

#include "../core/movegen.h"

template<bool bulk_counting, bool output>
U64 perft(Board &board, int depth) {
	Move moves[200];
	Move *moves_end = Movegen::gen_moves(board, moves, false);

	// Bulk counting the number of moves at depth 1.
	if (depth == 1 && bulk_counting)
		return moves_end - moves;
	if (depth == 0)
		return 1;

	// DFS like routine, calling itself recursively with lowered depth.
	U64 nodes = 0;
	for (Move *it = moves; it != moves_end; it++) {
		board.make_move(*it);
		U64 node_count = perft<bulk_counting, false>(board, depth - 1);
		if constexpr (output) {
			std::cout << *it << ": " << node_count << std::endl; // Used for debugging purposes.
		}
		nodes += node_count;
		board.undo_move(*it);
	}
	return nodes;

}
