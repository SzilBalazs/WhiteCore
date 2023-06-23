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

struct CastlingRights {
    unsigned int data = 0;

    CastlingRights() = default;

    explicit CastlingRights(const std::string &str) {
        for (char c : str) {
            if (c == 'K') add(WK_MASK);
            else if (c == 'Q')
                add(WQ_MASK);
            else if (c == 'k')
                add(BK_MASK);
            else if (c == 'q')
                add(BQ_MASK);
        }
    }

    void add(unsigned int right) {
        data |= right;
    }

    void remove(unsigned int right) {
        data &= ~right;
    }

    bool get(unsigned int right) const {
        return data & right;
    }

    std::string to_string() const {
        std::string res;
        if (get(WK_MASK))
            res += 'K';
        if (get(WQ_MASK))
            res += 'Q';
        if (get(BK_MASK))
            res += 'k';
        if (get(BQ_MASK))
            res += 'q';
        if (res.empty())
            res = "-";
        return res;
    }
};