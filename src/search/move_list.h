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
#include "history.h"

namespace search {
    template<bool captures_only>
    class MoveList {

    public:
        MoveList(const core::Board &board, const core::Move &hash_move, const History &history, const Ply &ply) : current(0), board(board),
                                                                                                                  hash_move(hash_move), history(history), ply(ply) {
            size = core::gen_moves(board, moves, captures_only) - moves;
            for (unsigned int i = 0; i < size; i++) {
                scores[i] = score_move(moves[i]);
            }
        }

        bool empty() {
            return current == size;
        }

        core::Move next_move() {
            for (unsigned int i = current; i < size; i++) {
                if (scores[i] > scores[current]) {
                    std::swap(scores[i], scores[current]);
                    std::swap(moves[i], moves[current]);
                }
            }
            return moves[current++];
        }

    private:
        core::Move moves[200];
        unsigned int size, current;
        Score scores[200];
        const core::Board &board;
        const core::Move &hash_move;
        const History &history;
        const Ply &ply;

        Score get_mvv_lva(const core::Move &move) {
            if (move.eq_flag(EP_CAPTURE))
                return MVVLVA[PAWN][PAWN];
            else
                return MVVLVA[board.piece_at(move.get_to()).type][board.piece_at(move.get_from()).type];
        }

        Score score_move(const core::Move &move) {
            if (move == hash_move) {
                return 10'000'000;
            } else if (move.is_promo()) {
                return 9'000'000;
            } else if (move.is_capture()) {
                return 8'000'000 + get_mvv_lva(move);
            } else if (move == history.killer_moves[ply][0]) {
                return 7'000'000;
            } else if (move == history.killer_moves[ply][1]) {
                return 6'000'000;
            } else {
                return history.butterfly[move.get_from()][move.get_to()];
            }
        }
    };
} // namespace search