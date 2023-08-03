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
#include "../utils/rng.h"
#include "data_entry.h"
#include "engine.h"

#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>

namespace selfplay {

    constexpr unsigned int DEFAULT_HASH_SIZE = 32;
    constexpr unsigned int DEFAULT_THREAD_COUNT = 1;
    constexpr unsigned int PROGRESS_BAR_WIDTH = 25;
    constexpr unsigned int BLOCK_SIZE = 100000;

    std::atomic<uint64_t> game_count, position_count;

    std::optional<GameResult> get_game_result(const core::Board &board) {

        if (board.is_draw()) return DRAW;

        core::Move moves[200];
        int cnt = core::gen_moves(board, moves, false) - moves;

        if (cnt == 0) {
            if (board.is_check()) {
                return board.get_stm() == WHITE ? BLACK_WIN : WHITE_WIN;
            } else {
                return DRAW;
            }
        }

        return std::nullopt;
    }

    void run_game(Engine &engine, const search::Limits &limits, const std::string &starting_fen, std::vector<DataEntry> &entries, unsigned int hash_size = DEFAULT_HASH_SIZE, const unsigned int thread_count = DEFAULT_THREAD_COUNT) {
        engine.init(hash_size, thread_count);
        core::Board board;
        board.load(starting_fen);
        unsigned int ply = 0;
        std::optional<GameResult> result;
        std::vector<DataEntry> tmp;
        while (ply <= 500 && !result) {

            auto [move, eval] = engine.search(board, limits);

            if (!board.is_check() && move.is_quiet() && std::abs(eval) < WORST_MATE) {
                tmp.emplace_back(board.get_fen(), ply, move, eval, std::nullopt);
                position_count++;
            }

            board.make_move(move);

            ply++;
            result = get_game_result(board);
        }

        for (DataEntry &entry : tmp) {
            entry.result = result;
            entries.emplace_back(entry);
        }
    }

    void gen_games(const search::Limits &limits, const std::vector<std::string> &starting_fens, const std::string &output_path) {

        Engine engine;

        std::ofstream file(output_path, std::ios::out | std::ios::app);
        std::random_device rd;
        std::mt19937 g(rd());

        std::vector<DataEntry> entries;
        for (const std::string &fen : starting_fens) {
            run_game(engine, limits, fen, entries);
            game_count++;

            if (entries.size() >= BLOCK_SIZE) {
                std::shuffle(entries.begin(), entries.end(), g);

                for (const DataEntry &entry : entries) {
                    file << entry.to_string() << "\n";
                }
                file.flush();

                entries.clear();
            }
        }

        std::shuffle(entries.begin(), entries.end(), g);
        for (const DataEntry &entry : entries) {
            file << entry.to_string() << "\n";
        }
        file.close();
    }

    void combine_data(const std::string &path, const std::string &output_file) {

        Logger("Combining files...");

        std::ofstream file(output_file, std::ios::app | std::ios::out);
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            std::ifstream in(entry.path());
            std::string tmp;
            while (std::getline(in, tmp)) {
                file << tmp << "\n";
            }
            in.close();
        }
        file.close();

        Logger("Finished combining");
    }

    void compress_data(const std::string &input_path, const std::string &output_file) {

        std::stringstream ss;
        ss << "zstd " << input_path << " -o " << output_file << " --rm #19";

        const std::string cmd = ss.str();
        Logger(">", cmd);

        system(cmd.c_str());

        Logger("Finished compressing");
    }

    std::string get_run_name(const search::Limits &limits, const std::string &id) {
        std::stringstream ss;
        ss << id << "_" << limits.to_string() << "_" << (position_count / 1000) << "k";

        return ss.str();
    }

    void populate_starting_fens(const uint64_t games_to_play, std::vector<std::string> &fens) {
        for (uint64_t i = 0; i < games_to_play; i++) {
            fens.emplace_back(rng::gen_fen());
        }
    }

    void try_to_create_directory(const std::string &path) {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }
    }

    void print_progress(const uint64_t games_to_play) {

        const int64_t start_time = now();

        while (game_count != games_to_play) {

            std::this_thread::sleep_for(std::chrono::seconds(1));

            const int64_t current_time = now();
            const int64_t elapsed_time = current_time - start_time + 1;

            const float progress = float(game_count + 1) / float(games_to_play);

            const int64_t total_time = elapsed_time / progress;
            const int64_t eta = (1.0f - progress) * total_time;

            const unsigned int percentage = progress * 100;
            const uint64_t pos_per_s = position_count * 1000ULL / elapsed_time;

            const unsigned int progress_position = PROGRESS_BAR_WIDTH * progress;

            std::cout << "[";
            for (unsigned int i = 0; i < PROGRESS_BAR_WIDTH; i++) {
                if (i < progress_position) {
                    std::cout << "=";
                } else if (i == progress_position) {
                    std::cout << ">";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "] " << percentage << "% - " << game_count << "/" << games_to_play << " games - ETA " << (eta / 1000) << "s - " << pos_per_s << " pos/s \r" << std::flush;
        }

        std::cout << std::endl;
    }

    void start_generation(const search::Limits &limits, const uint64_t games_to_play, const unsigned int thread_count) {

        game_count = 0;
        position_count = 0;

        const std::string run_id = rng::gen_id();
        const std::string directory_path = "selfplay/" + run_id;

        try_to_create_directory("selfplay");
        try_to_create_directory("data");
        try_to_create_directory(directory_path);

        std::vector<std::string> starting_fens;
        std::vector<std::thread> workers;

        populate_starting_fens(games_to_play, starting_fens);

        for (unsigned int id = 0; id < thread_count; id++) {
            std::vector<std::string> workload;
            for (unsigned int i = id; i < starting_fens.size(); i += thread_count) {
                workload.emplace_back(starting_fens[i]);
            }
            workers.emplace_back(gen_games, limits, workload, directory_path + "/" + std::to_string(id) + ".plain");
        }

        print_progress(games_to_play);

        for (std::thread &th : workers) {
            if (th.joinable())
                th.join();
        }

        const std::string output_path = "data/" + get_run_name(limits, run_id);

        combine_data(directory_path, output_path + ".plain");
        compress_data(output_path + ".plain", output_path + ".zst");

        exit(0);
    }

} // namespace selfplay