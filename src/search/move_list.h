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

#include "../chess/move_generation.h"
#include "history.h"
#include "see.h"

namespace search {
    template<bool captures_only>
    class MoveList {

        static constexpr unsigned int MOVE_SCORE_HASH = 10'000'000;
        static constexpr unsigned int MOVE_SCORE_PROMO = 9'000'000;
        static constexpr unsigned int MOVE_SCORE_GOOD_CAPTURE = 8'000'000;
        static constexpr unsigned int MOVE_SCORE_FIRST_KILLER = 7'000'000;
        static constexpr unsigned int MOVE_SCORE_SECOND_KILLER = 6'000'000;
        static constexpr unsigned int MOVE_SCORE_COUNTER = 5'000'000;
        static constexpr unsigned int MOVE_SCORE_BAD_CAPTURE = 4'000'000;

    public:
        MoveList(const chess::Board &board, const chess::Move &hash_move, const chess::Move &last_move, const History &history, const Ply &ply) : current(0), board(board),
                                                                                                                                                  hash_move(hash_move), last_move(last_move), history(history), ply(ply) {
            size = chess::gen_moves(board, moves, captures_only) - moves;
            std::transform(moves, moves + size, scores, [this](const chess::Move &move) {
                return score_move(move);
            });
        }

        [[nodiscard]] bool empty() const {
            return current == size;
        }

        [[nodiscard]] chess::Move next_move() {
            for (unsigned int i = current; i < size; i++) {
                if (scores[i] > scores[current]) {
                    std::swap(scores[i], scores[current]);
                    std::swap(moves[i], moves[current]);
                }
            }
            return moves[current++];
        }

    private:
        chess::Move moves[200];
        unsigned int size, current;
        Score scores[200];
        const chess::Board &board;
        const chess::Move &hash_move;
        const chess::Move &last_move;
        const History &history;
        const Ply &ply;

        [[nodiscard]] Score get_mvv_lva(const chess::Move &move) const {
            return move.eq_flag(chess::Move::EP_CAPTURE)
                           ? MVVLVA[PAWN][PAWN]
                           : MVVLVA[board.piece_at(move.get_to()).type][board.piece_at(move.get_from()).type];
        }

        [[nodiscard]] Score score_move(const chess::Move &move) const {
            if (move == hash_move) {
                return MOVE_SCORE_HASH;
            } else if (move.is_promo()) {
                return MOVE_SCORE_PROMO;
            } else if (move.is_capture()) {
                return (see(board, move, 0) ? MOVE_SCORE_GOOD_CAPTURE : MOVE_SCORE_BAD_CAPTURE) + get_mvv_lva(move);
            } else if (move == history.killer_moves[ply][0]) {
                return MOVE_SCORE_FIRST_KILLER;
            } else if (move == history.killer_moves[ply][1]) {
                return MOVE_SCORE_SECOND_KILLER;
            } else if (move == history.counter_moves[last_move.get_from()][last_move.get_to()]) {
                return MOVE_SCORE_COUNTER;
            } else {
                return history.butterfly[move.get_from()][move.get_to()];
            }
        }
    };
} // namespace search