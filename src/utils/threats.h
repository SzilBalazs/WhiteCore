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

namespace threats {

    using namespace chess;

    template<Color color>
    Bitboard get_pawn_threats(Bitboard pawns, Bitboard opponent_pieces) {
        if constexpr (color == WHITE) {
            return (step<NORTH_EAST>(pawns) | step<NORTH_WEST>(pawns)) & opponent_pieces;
        } else {
            return (step<SOUTH_EAST>(pawns) | step<SOUTH_WEST>(pawns)) & opponent_pieces;
        }
    }

    Bitboard get_minor_threats(Bitboard occ, Bitboard knights, Bitboard bishops, Bitboard opponent_majors) {
        Bitboard attacks = 0;
        while (knights) {
            Square sq = knights.pop_lsb();
            attacks |= masks_knight[sq];
        }
        while (bishops) {
            Square sq = bishops.pop_lsb();
            attacks |= attacks_piece<BISHOP>(sq, occ);
        }
        return attacks & opponent_majors;
    }

    Bitboard get_rook_threats(Bitboard occ, Bitboard rooks, Bitboard opponent_queens) {
        Bitboard attacks = 0;
        while (rooks) {
            Square sq = rooks.pop_lsb();
            attacks |= attacks_piece<ROOK>(sq, occ);
        }
        return attacks & opponent_queens;
    }

    // Huge inspiration from BlackMarlin's implementation
    Bitboard get_threats(const Board &board) {

        Bitboard occ = board.occupied();
        Bitboard white = board.sides<WHITE>();
        Bitboard black = board.sides<BLACK>();

        Bitboard pawns = board.pieces<PAWN>();
        Bitboard knights = board.pieces<KNIGHT>();
        Bitboard bishops = board.pieces<BISHOP>();
        Bitboard rooks = board.pieces<ROOK>();
        Bitboard queens = board.pieces<QUEEN>();

        Bitboard minors = knights | bishops;
        Bitboard majors = rooks | queens;
        Bitboard pieces = minors | majors;

        Bitboard threats = 0;

        threats |= get_pawn_threats<WHITE>(pawns & white, black & pieces);
        threats |= get_pawn_threats<BLACK>(pawns & black, white & pieces);

        threats |= get_minor_threats(occ, knights & white, bishops & white, majors & black);
        threats |= get_minor_threats(occ, knights & black, bishops & black, majors & white);

        threats |= get_rook_threats(occ, rooks & white, queens & black);
        threats |= get_rook_threats(occ, rooks & black, queens & white);

        return threats;
    }

} // namespace threats