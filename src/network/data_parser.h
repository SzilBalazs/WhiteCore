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

#include "../chess/constants.h"
#include "../utils/utilities.h"
#include "activations/sigmoid.h"
#include "network.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace nn {

    struct TrainingEntry {
        std::vector<unsigned int> white_features;
        std::vector<unsigned int> black_features;
        float wdl;
        float eval;

        explicit TrainingEntry(const std::string &entry) {
            std::stringstream ss(entry);
            unsigned int sq = 56, idx = 0;

            std::string s_fen, s_ply, s_bm, s_eval, s_wdl;
            std::getline(ss, s_fen, ';');
            std::getline(ss, s_ply, ';');
            std::getline(ss, s_bm, ';');
            std::getline(ss, s_eval, ';');
            std::getline(ss, s_wdl, ';');

            if (s_wdl == "1") {
                wdl = 1.0f;
            } else if (s_wdl == "0") {
                wdl = 0.5f;
            } else {
                wdl = 0.0f;
            }

            for (; idx < s_fen.size() && s_fen[idx] != ' '; idx++) {
                char c = s_fen[idx];
                if ('1' <= c && c <= '8') {
                    sq += c - '0';
                } else if (c == '/') {
                    sq -= 16;
                } else {
                    Piece p = piece_from_char(c);
                    white_features.emplace_back(Network::get_feature_index(p, sq));
                    p.color = color_enemy(p.color);
                    black_features.emplace_back(Network::get_feature_index(p, sq ^ 56));

                    sq++;
                }
            }

            Color stm;
            idx++;
            if (s_fen[idx] == 'w')
                stm = WHITE;
            else if (s_fen[idx] == 'b')
                stm = BLACK;
            else {
                throw std::runtime_error("Invalid color specified in FEN " + entry + ".");
            }

            int eval_int = std::stoi(s_eval);
            if (stm == BLACK) eval_int *= -1;
            eval = activations::sigmoid::forward(float(eval_int) / 400.0f);
        }
    };

    class DataParser {
    public:
        explicit DataParser(const std::string &path) {
            file.open(path, std::ios::in);

            if (!file.is_open()) {
                print("Unable to open:", path);
                throw std::runtime_error("Unable to open: " + path);
            }
        }

        void read_batch(size_t batch_size, std::string *lines, bool &is_new_epoch) {
            std::string line;
            for (size_t i = 0; i < batch_size; i++) {
                if (std::getline(file, line)) {
                    lines[i] = line;
                } else {
                    i--;
                    file.clear();
                    file.seekg(0);
                    is_new_epoch = true;
                }
            }
        }

    private:
        std::ifstream file;
    };
} // namespace nn