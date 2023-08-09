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

namespace selfplay {

    enum GameResult {
        WHITE_WIN,
        DRAW,
        BLACK_WIN
    };

    std::string get_wdl(const GameResult &result) {
        switch (result) {
            case WHITE_WIN:
                return "1";
            case DRAW:
                return "0";
            case BLACK_WIN:
                return "-1";
        }
        return "";
    }

    struct DataEntry {
        std::string fen;
        unsigned int ply;
        chess::Move best_move;
        Score eval;
        std::optional<GameResult> result;

        DataEntry(std::string fen, unsigned int ply, chess::Move best_move, Score eval, std::optional<GameResult> result) : fen(std::move(fen)), ply(ply),
                                                                                                                           best_move(best_move), eval(eval),
                                                                                                                           result(result) {}

        [[nodiscard]] std::string to_string() const {
            std::stringstream ss;
            ss << fen << ";" << ply << ";" << best_move.to_uci() << ";"
               << int(eval) << ";" << get_wdl(result.value_or(DRAW)) << ";";
            return ss.str();
        }
    };

} // namespace selfplay