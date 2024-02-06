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

#include <string>
#include <utility>
#include <vector>

#include "../chess/constants.h"

namespace stat_tracker {

#ifdef TRACK_STATS

    struct StatInfo {
        std::string name;
        uint64_t total;
        uint64_t success;

        StatInfo(std::string name) : name(std::move(name)), total(0), success(0) {}
    };

    std::vector<StatInfo> data;

    void add_stat(const std::string &name) {
        data.emplace_back(name);
    }

    void print_stats() {
        for (const StatInfo &info : data) {
            std::cout << info.name << " - " << info.success << "/" << info.total << " - " << std::fixed << std::setprecision(3) << (float(info.success) / float(info.total)) * 100.0f << "%" << std::endl;
        }
    }

    void record_success(const std::string &name) {
        for (StatInfo &info : data) {
            if (info.name == name) {
                info.success++;
                info.total++;
                return;
            }
        }
        throw std::runtime_error("Unrecognised statistic: " + name);
    }

    void record_fail(const std::string &name) {
        for (StatInfo &info : data) {
            if (info.name == name) {
                info.total++;
                return;
            }
        }
        throw std::runtime_error("Unrecognised statistic: " + name);
    }
#else
    void add_stat(const std::string &name) {}
    void print_stats() {}
    void record_success(const std::string &name) {}
    void record_fail(const std::string &name) {}
#endif
} // namespace stat_tracker
