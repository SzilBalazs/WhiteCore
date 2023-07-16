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
#include "data_entry.h"
#include "engine.h"

#include <ctime>
#include <random>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace selfplay {

    const unsigned int DEFAULT_HASH_SIZE = 32;
    const unsigned int DEFAULT_THREAD_COUNT = 1;
    const unsigned int BLOCK_SIZE = 100000;

    std::atomic<uint64_t> game_count, position_count;

    const unsigned int PROGRESS_BAR_WIDTH = 25;

    inline std::string get_date() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
        return ss.str();
    }

    inline std::optional<GameResult> get_game_result(const core::Board &board) {

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

    inline void run_game(Engine &engine, const search::Limits &limits, const std::string &starting_fen, std::vector<DataEntry> &entries, unsigned int hash_size = DEFAULT_HASH_SIZE, const unsigned int thread_count = DEFAULT_THREAD_COUNT) {
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

    inline void gen_games(const search::Limits &limits, const std::vector<std::string> &starting_fens, const std::string &output_path) {

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

    inline void combine_data(const std::string &path, const std::string &output_file) {

        logger.print("Combining files...");

        std::ofstream file(output_file, std::ios::app | std::ios::out);
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::ifstream in(entry.path());
            std::string tmp;
            while (std::getline(in, tmp)) {
                file << tmp << "\n";
            }
            in.close();
        }
        file.close();

        logger.print("Finished combining");
    }

    inline void start_generation(const search::Limits &limits, const std::string &book_path, const std::string &output_path, unsigned int thread_count, int dropout) {
        game_count = 0;
        position_count = 0;

        // TODO argument for network file
        if (!std::filesystem::exists(book_path)) {
            logger.print("Invalid book path: " + book_path);
            throw std::invalid_argument("Invalid book path: " + book_path);
        }

        if (!std::filesystem::exists("selfplay")) {
            std::filesystem::create_directory("selfplay");
        }

        std::string date = get_date();
        std::string directory_path = "selfplay/" + date;
        if (!std::filesystem::exists(directory_path)) {
            std::filesystem::create_directory(directory_path);
        }

        std::ifstream book(book_path, std::ios::in);
        std::vector<std::string> starting_fens;
        std::string tmp;

        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<int> dist(0, dropout-1);

        while (std::getline(book, tmp)) {
            if (dist(g) == 0)
                starting_fens.emplace_back(tmp);
        }

        book.close();

        std::vector<std::thread> threads;

        logger.print("Starting", thread_count, "threads...");
        for (unsigned int id = 0; id < thread_count; id++) {
            std::vector<std::string> fens;
            for (unsigned int i = id; i < starting_fens.size(); i += thread_count) {
                fens.emplace_back(starting_fens[i]);
            }
            threads.emplace_back(gen_games, limits, fens, directory_path + "/" + std::to_string(id) + ".plain");
        }

        const unsigned int total_games = starting_fens.size();
        int64_t start_time = now();

        while (game_count != total_games) {

            std::this_thread::sleep_for(std::chrono::seconds(1));

            int64_t current_time = now();
            int64_t elapsed_time = current_time - start_time + 1;

            float progress = float(game_count + 1) / float(total_games);

            int64_t total_time = elapsed_time / progress;
            int64_t eta = (1.0f - progress) * total_time;

            unsigned int percentage = progress * 100;
            uint64_t pos_per_s = position_count * 1000ULL / elapsed_time;

            unsigned int progress_position = PROGRESS_BAR_WIDTH * progress;

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
            std::cout << "] " << percentage << "% - " << game_count << "/" << total_games << " games - ETA " << (eta / 1000) << "s - " << pos_per_s << " pos/s \r" << std::flush;
        }

        std::cout << std::endl;

        for (std::thread &th : threads) {
            if (th.joinable())
                th.join();
        }

        logger.print("Finished generating data");

        combine_data(directory_path, output_path);
    }

} // namespace selfplay