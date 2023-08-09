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

#include "../utils/utilities.h"
#include "bitboard.h"

namespace chess {
    // Stores a magic entry.
    struct Magic {
        Bitboard *ptr;
        Bitboard mask;
        Bitboard magic;
        unsigned int shift;
    };

    extern Bitboard attack_table_rook[102400];
    extern Bitboard attack_table_bishop[5248];

    // Slow naive function of getting the attacked squares of a sliding piece.
    [[nodiscard]] Bitboard attacks_sliding_slow(Square square, Bitboard occupied, PieceType pt) {
        assert((pt == ROOK) || (pt == BISHOP));
        switch (pt) {
            case ROOK:
                return slide<NORTH>(square, occupied) | slide<SOUTH>(square, occupied) | slide<WEST>(square, occupied) |
                       slide<EAST>(square, occupied);
            case BISHOP:
                return slide<NORTH_WEST>(square, occupied) | slide<NORTH_EAST>(square, occupied) |
                       slide<SOUTH_WEST>(square, occupied) | slide<SOUTH_EAST>(square, occupied);
            default:
                return {};
        }
    }

    // Converts the magic and the occupancy bitboard into an index in the lookup table.
    [[nodiscard]] unsigned int get_magic_index(const Magic &m, Bitboard occ) {
#ifdef BMI2
        return _pext_u64(occ.bb, m.mask.bb);
#else
        return (((occ & m.mask) * m.magic) >> (64 - m.shift)).bb;
#endif
    }

    // Initializes magic bitboards by pt.
    void init_magic(const Magic *magics, PieceType pt) {
        assert((pt == ROOK) || (pt == BISHOP));
        Bitboard occupied[4096], attacked[4096];

        for (Square square = A1; square < 64; square += 1) {
            const Magic &magic = magics[square];

            size_t length = 0;
            Bitboard occ = 0;
            do {
                occupied[length] = occ;
                attacked[length] = attacks_sliding_slow(square, occ, pt);

                length++;
                occ = (occ - magic.mask) & magic.mask;
            } while (occ != 0);

            for (size_t i = 0; i < length; i++) {
                U64 index = get_magic_index(magic, occupied[i]);

                magic.ptr[index] = attacked[i];
            }
        }
    }

    /*
 * Fancy magic bitboards
 * To generate the attackTables from them use initMagic
 * To generate new magic numbers use findMagics
 */
    constexpr Magic magic_rook[64] = {
            {attack_table_rook + 0, 0x101010101017eULL, 0x200102084420100ULL, 12},
            {attack_table_rook + 4096, 0x202020202027cULL, 0x40200040001000ULL, 11},
            {attack_table_rook + 6144, 0x404040404047aULL, 0x4100082000104300ULL, 11},
            {attack_table_rook + 8192, 0x8080808080876ULL, 0x480049000080080ULL, 11},
            {attack_table_rook + 10240, 0x1010101010106eULL, 0x100040211000800ULL, 11},
            {attack_table_rook + 12288, 0x2020202020205eULL, 0x2500240002080100ULL, 11},
            {attack_table_rook + 14336, 0x4040404040403eULL, 0x280120001000080ULL, 11},
            {attack_table_rook + 16384, 0x8080808080807eULL, 0x200004086002b04ULL, 12},
            {attack_table_rook + 20480, 0x1010101017e00ULL, 0x401800280400020ULL, 11},
            {attack_table_rook + 22528, 0x2020202027c00ULL, 0x8601400050002000ULL, 10},
            {attack_table_rook + 23552, 0x4040404047a00ULL, 0x802801000200280ULL, 10},
            {attack_table_rook + 24576, 0x8080808087600ULL, 0x411001001002008ULL, 10},
            {attack_table_rook + 25600, 0x10101010106e00ULL, 0x11000410080300ULL, 10},
            {attack_table_rook + 26624, 0x20202020205e00ULL, 0x20a000804108200ULL, 10},
            {attack_table_rook + 27648, 0x40404040403e00ULL, 0x84006850240102ULL, 10},
            {attack_table_rook + 28672, 0x80808080807e00ULL, 0x24800049000080ULL, 11},
            {attack_table_rook + 30720, 0x10101017e0100ULL, 0x208000400080ULL, 11},
            {attack_table_rook + 32768, 0x20202027c0200ULL, 0x101020020804202ULL, 10},
            {attack_table_rook + 33792, 0x40404047a0400ULL, 0x20828010022000ULL, 10},
            {attack_table_rook + 34816, 0x8080808760800ULL, 0x801230009001000ULL, 10},
            {attack_table_rook + 35840, 0x101010106e1000ULL, 0x5608808004020801ULL, 10},
            {attack_table_rook + 36864, 0x202020205e2000ULL, 0x3086008080040002ULL, 10},
            {attack_table_rook + 37888, 0x404040403e4000ULL, 0x40041221008ULL, 10},
            {attack_table_rook + 38912, 0x808080807e8000ULL, 0x8000020000811044ULL, 11},
            {attack_table_rook + 40960, 0x101017e010100ULL, 0x21c00180002081ULL, 11},
            {attack_table_rook + 43008, 0x202027c020200ULL, 0xa010024140002000ULL, 10},
            {attack_table_rook + 44032, 0x404047a040400ULL, 0x1040200280100080ULL, 10},
            {attack_table_rook + 45056, 0x8080876080800ULL, 0x2100100200b00ULL, 10},
            {attack_table_rook + 46080, 0x1010106e101000ULL, 0x8014008080040800ULL, 10},
            {attack_table_rook + 47104, 0x2020205e202000ULL, 0x840200120008904cULL, 10},
            {attack_table_rook + 48128, 0x4040403e404000ULL, 0x10020400811058ULL, 10},
            {attack_table_rook + 49152, 0x8080807e808000ULL, 0x8280040200004081ULL, 11},
            {attack_table_rook + 51200, 0x1017e01010100ULL, 0xa000408001002100ULL, 11},
            {attack_table_rook + 53248, 0x2027c02020200ULL, 0x210904000802000ULL, 10},
            {attack_table_rook + 54272, 0x4047a04040400ULL, 0x200204082001200ULL, 10},
            {attack_table_rook + 55296, 0x8087608080800ULL, 0x2204201042000a00ULL, 10},
            {attack_table_rook + 56320, 0x10106e10101000ULL, 0x6c80040801001100ULL, 10},
            {attack_table_rook + 57344, 0x20205e20202000ULL, 0x8040080800200ULL, 10},
            {attack_table_rook + 58368, 0x40403e40404000ULL, 0x2b0900804001663ULL, 10},
            {attack_table_rook + 59392, 0x80807e80808000ULL, 0x4074800040800100ULL, 11},
            {attack_table_rook + 61440, 0x17e0101010100ULL, 0x4000400080208000ULL, 11},
            {attack_table_rook + 63488, 0x27c0202020200ULL, 0x1a40500020004001ULL, 10},
            {attack_table_rook + 64512, 0x47a0404040400ULL, 0x1004020010018ULL, 10},
            {attack_table_rook + 65536, 0x8760808080800ULL, 0x20201200420008ULL, 10},
            {attack_table_rook + 66560, 0x106e1010101000ULL, 0xc24008008008005ULL, 10},
            {attack_table_rook + 67584, 0x205e2020202000ULL, 0x4002010804020010ULL, 10},
            {attack_table_rook + 68608, 0x403e4040404000ULL, 0xb015081002040001ULL, 10},
            {attack_table_rook + 69632, 0x807e8080808000ULL, 0x4000408c020029ULL, 11},
            {attack_table_rook + 71680, 0x7e010101010100ULL, 0xb840004020800080ULL, 11},
            {attack_table_rook + 73728, 0x7c020202020200ULL, 0x60804001002100ULL, 10},
            {attack_table_rook + 74752, 0x7a040404040400ULL, 0x210810a285420200ULL, 10},
            {attack_table_rook + 75776, 0x76080808080800ULL, 0xa000080010008080ULL, 10},
            {attack_table_rook + 76800, 0x6e101010101000ULL, 0x800050010080100ULL, 10},
            {attack_table_rook + 77824, 0x5e202020202000ULL, 0x4040002008080ULL, 10},
            {attack_table_rook + 78848, 0x3e404040404000ULL, 0x80b4011042080400ULL, 10},
            {attack_table_rook + 79872, 0x7e808080808000ULL, 0x6014004114008200ULL, 11},
            {attack_table_rook + 81920, 0x7e01010101010100ULL, 0x1001002018408202ULL, 12},
            {attack_table_rook + 86016, 0x7c02020202020200ULL, 0x2400104128421ULL, 11},
            {attack_table_rook + 88064, 0x7a04040404040400ULL, 0x407600010408901ULL, 11},
            {attack_table_rook + 90112, 0x7608080808080800ULL, 0x108448a01001000dULL, 11},
            {attack_table_rook + 92160, 0x6e10101010101000ULL, 0x8402011008842002ULL, 11},
            {attack_table_rook + 94208, 0x5e20202020202000ULL, 0x11000204000801ULL, 11},
            {attack_table_rook + 96256, 0x3e40404040404000ULL, 0x4026000108208452ULL, 11},
            {attack_table_rook + 98304, 0x7e80808080808000ULL, 0x800081004c2c06ULL, 12},
    };

    constexpr Magic magic_bishop[64] = {
            {attack_table_bishop + 0, 0x40201008040200ULL, 0x4100216240212ULL, 6},
            {attack_table_bishop + 64, 0x402010080400ULL, 0x8080110420002ULL, 5},
            {attack_table_bishop + 96, 0x4020100a00ULL, 0x4280091000005ULL, 5},
            {attack_table_bishop + 128, 0x40221400ULL, 0x24410020801400ULL, 5},
            {attack_table_bishop + 160, 0x2442800ULL, 0x4242000000311ULL, 5},
            {attack_table_bishop + 192, 0x204085000ULL, 0x882021006148000ULL, 5},
            {attack_table_bishop + 224, 0x20408102000ULL, 0xb440a0210260800ULL, 5},
            {attack_table_bishop + 256, 0x2040810204000ULL, 0x80840c0a011c00ULL, 6},
            {attack_table_bishop + 320, 0x20100804020000ULL, 0x1000040488080100ULL, 5},
            {attack_table_bishop + 352, 0x40201008040000ULL, 0x800a200202284112ULL, 5},
            {attack_table_bishop + 384, 0x4020100a0000ULL, 0xcc00098401020000ULL, 5},
            {attack_table_bishop + 416, 0x4022140000ULL, 0x8000080a00202000ULL, 5},
            {attack_table_bishop + 448, 0x244280000ULL, 0x8821210000824ULL, 5},
            {attack_table_bishop + 480, 0x20408500000ULL, 0xc000088230400020ULL, 5},
            {attack_table_bishop + 512, 0x2040810200000ULL, 0x2904494808a41024ULL, 5},
            {attack_table_bishop + 544, 0x4081020400000ULL, 0x2302882301004ULL, 5},
            {attack_table_bishop + 576, 0x10080402000200ULL, 0x910200610100104ULL, 5},
            {attack_table_bishop + 608, 0x20100804000400ULL, 0x910800850008080ULL, 5},
            {attack_table_bishop + 640, 0x4020100a000a00ULL, 0x30080010004d4009ULL, 7},
            {attack_table_bishop + 768, 0x402214001400ULL, 0x4108000c20222001ULL, 7},
            {attack_table_bishop + 896, 0x24428002800ULL, 0x22000400942005ULL, 7},
            {attack_table_bishop + 1024, 0x2040850005000ULL, 0xa021100512400ULL, 7},
            {attack_table_bishop + 1152, 0x4081020002000ULL, 0xa001000041301024ULL, 5},
            {attack_table_bishop + 1184, 0x8102040004000ULL, 0x8000420206021981ULL, 5},
            {attack_table_bishop + 1216, 0x8040200020400ULL, 0x1008480004606800ULL, 5},
            {attack_table_bishop + 1248, 0x10080400040800ULL, 0x4a8280003100100ULL, 5},
            {attack_table_bishop + 1280, 0x20100a000a1000ULL, 0x3480010182240ULL, 7},
            {attack_table_bishop + 1408, 0x40221400142200ULL, 0x2048080102820042ULL, 9},
            {attack_table_bishop + 1920, 0x2442800284400ULL, 0x4001020004008400ULL, 9},
            {attack_table_bishop + 2432, 0x4085000500800ULL, 0x204004048080200ULL, 7},
            {attack_table_bishop + 2560, 0x8102000201000ULL, 0x2008200040212a0ULL, 5},
            {attack_table_bishop + 2592, 0x10204000402000ULL, 0x10c013002430400ULL, 5},
            {attack_table_bishop + 2624, 0x4020002040800ULL, 0x4300a5082214480ULL, 5},
            {attack_table_bishop + 2656, 0x8040004081000ULL, 0x401041000215900ULL, 5},
            {attack_table_bishop + 2688, 0x100a000a102000ULL, 0x104804048040408ULL, 7},
            {attack_table_bishop + 2816, 0x22140014224000ULL, 0x800400808208200ULL, 9},
            {attack_table_bishop + 3328, 0x44280028440200ULL, 0x8002400054101ULL, 9},
            {attack_table_bishop + 3840, 0x8500050080400ULL, 0x2001004502020102ULL, 7},
            {attack_table_bishop + 3968, 0x10200020100800ULL, 0x1988080110006100ULL, 5},
            {attack_table_bishop + 4000, 0x20400040201000ULL, 0x1282009200102201ULL, 5},
            {attack_table_bishop + 4032, 0x2000204081000ULL, 0xa208010420001280ULL, 5},
            {attack_table_bishop + 4064, 0x4000408102000ULL, 0x4004010809000200ULL, 5},
            {attack_table_bishop + 4096, 0xa000a10204000ULL, 0x43008150006100ULL, 7},
            {attack_table_bishop + 4224, 0x14001422400000ULL, 0x2410145000801ULL, 7},
            {attack_table_bishop + 4352, 0x28002844020000ULL, 0x280104006040ULL, 7},
            {attack_table_bishop + 4480, 0x50005008040200ULL, 0x4012042000902ULL, 7},
            {attack_table_bishop + 4608, 0x20002010080400ULL, 0x28100482080a82ULL, 5},
            {attack_table_bishop + 4640, 0x40004020100800ULL, 0x80040c2400240240ULL, 5},
            {attack_table_bishop + 4672, 0x20408102000ULL, 0x80c1101101044a0ULL, 5},
            {attack_table_bishop + 4704, 0x40810204000ULL, 0x180804802310808ULL, 5},
            {attack_table_bishop + 4736, 0xa1020400000ULL, 0x8048080064ULL, 5},
            {attack_table_bishop + 4768, 0x142240000000ULL, 0x8c8400020880000ULL, 5},
            {attack_table_bishop + 4800, 0x284402000000ULL, 0x30001010020a2000ULL, 5},
            {attack_table_bishop + 4832, 0x500804020000ULL, 0x80600282220010ULL, 5},
            {attack_table_bishop + 4864, 0x201008040200ULL, 0x120228228010000ULL, 5},
            {attack_table_bishop + 4896, 0x402010080400ULL, 0xc08020802042300ULL, 5},
            {attack_table_bishop + 4928, 0x2040810204000ULL, 0x2a008048221000ULL, 6},
            {attack_table_bishop + 4992, 0x4081020400000ULL, 0x4601204100901002ULL, 5},
            {attack_table_bishop + 5024, 0xa102040000000ULL, 0x821200104052400ULL, 5},
            {attack_table_bishop + 5056, 0x14224000000000ULL, 0x8200084208810ULL, 5},
            {attack_table_bishop + 5088, 0x28440200000000ULL, 0x8c022040a80b0408ULL, 5},
            {attack_table_bishop + 5120, 0x50080402000000ULL, 0x2140201012100512ULL, 5},
            {attack_table_bishop + 5152, 0x20100804020000ULL, 0x10210240128120aULL, 5},
            {attack_table_bishop + 5184, 0x40201008040200ULL, 0x208600082060020ULL, 6},
    };

    void init_magic() {
        init_magic(magic_rook, ROOK);
        init_magic(magic_bishop, BISHOP);
    }
} // namespace core