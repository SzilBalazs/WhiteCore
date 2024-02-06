// WhiteCore is a C++ chess engine
// Copyright (c) 2023-2024 Balázs Szilágyi
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

namespace chess {
    struct CastlingRights {

        static constexpr unsigned char WHITE_KING = 1;
        static constexpr unsigned char WHITE_QUEEN = 2;
        static constexpr unsigned char BLACK_KING = 4;
        static constexpr unsigned char BLACK_QUEEN = 8;

        unsigned int data = 0;

        CastlingRights() = default;

        explicit CastlingRights(const std::string &str) {
            for (char c : str) {
                if (c == 'K') {
                    *this += WHITE_KING;
                } else if (c == 'Q') {
                    *this += WHITE_QUEEN;
                } else if (c == 'k') {
                    *this += BLACK_KING;
                } else if (c == 'q') {
                    *this += BLACK_QUEEN;
                }
            }
        }

        CastlingRights &operator+=(const unsigned int &right) {
            data |= right;
            return *this;
        }

        CastlingRights &operator-=(const unsigned int &right) {
            data &= ~right;
            return *this;
        }

        [[nodiscard]] bool operator[](const unsigned int &right) const {
            return data & right;
        }

        [[nodiscard]] std::string to_string() const {
            std::string res;
            if ((*this)[WHITE_KING]) {
                res += 'K';
            }
            if ((*this)[WHITE_QUEEN]) {
                res += 'Q';
            }
            if ((*this)[BLACK_KING]) {
                res += 'k';
            }
            if ((*this)[BLACK_QUEEN]) {
                res += 'q';
            }
            if (res.empty()) {
                res = "-";
            }
            return res;
        }
    };
} // namespace chess
