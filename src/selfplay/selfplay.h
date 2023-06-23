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

#include "../search/time_manager.h"
#include "engine.h"

namespace selfplay {

    const unsigned int DEFAULT_HASH_SIZE = 32;
    const unsigned int DEFAULT_THREAD_COUNT = 1;

    enum GameResult {
        WHITE_WIN, DRAW, BLACK_WIN
    };

    inline std::optional<GameResult> get_game_result(const Board &board) {

        if (board.is_draw()) return DRAW;

        Move moves[200];
        int cnt = movegen::gen_moves(board, moves, false) - moves;

        if (cnt == 0) {
            if (board.is_check()) {
                return board.get_stm() == WHITE ? BLACK_WIN : WHITE_WIN;
            } else {
                return DRAW;
            }
        }

        return std::nullopt;
    }

    inline void run_game(Engine &engine_white, Engine &engine_black, const SearchLimits &limits, const std::string &starting_fen, const unsigned int hash_size = DEFAULT_HASH_SIZE, const unsigned int thread_count = DEFAULT_THREAD_COUNT) {
        engine_white.init(hash_size, thread_count);
        engine_black.init(hash_size, thread_count);
        Board board;
        board.load(starting_fen);
        int ply = 0;
        std::optional<GameResult> result;
        while (ply <= 500 && !result) {

            Color stm = board.get_stm();
            Engine &engine = stm == WHITE ? engine_white : engine_black;
            Move move = engine.search(board, limits);
            board.make_move(move);

            ply++;
            result = get_game_result(board);
        }
    }

} // namespace selfplay