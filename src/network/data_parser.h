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

#include "../core/constants.h"
#include "../utils/utilities.h"
#include "network.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace nn {
    struct TrainingEntry {
        std::vector<unsigned int> features;
        float wdl;
    };

    class DataParser {
    public:

        DataParser(const std::string &path) {
            file.open(path, std::ios::in);
        }

        void read_batch(unsigned int batch_size, TrainingEntry *entries, bool &is_new_epoch) {
            std::string line;
            for (unsigned int i = 0; i < batch_size; i++) {
                if (std::getline(file, line)) {
                    entries[i] = parse_entry(line);
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

        TrainingEntry parse_entry(const std::string &entry) {
            TrainingEntry res;
            std::stringstream ss(entry);
            unsigned int sq = 56, idx = 0;

            std::string fen, ply, bm, eval, wdl;
            std::getline(ss, fen, ';');
            std::getline(ss, ply, ';');
            std::getline(ss, bm, ';');
            std::getline(ss, eval, ';');
            std::getline(ss, wdl, ';');

            if (wdl == "1") {
                res.wdl = 1.0f;
            } else if (wdl == "0") {
                res.wdl = 0.5f;
            } else {
                res.wdl = 0.0f;
            }

            for (; idx < fen.size() && fen[idx] != ' '; idx++) {
                char c = fen[idx];
                if ('1' <= c && c <= '8') {
                    sq += c-'0';
                } else if (c == '/') {
                    sq -= 16;
                } else {
                    Piece p = piece_from_char(c);
                    res.features.emplace_back(Network::get_feature_index(p, sq));
                    sq++;
                }
            }

            return res;
        }
    };
}