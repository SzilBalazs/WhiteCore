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

#include "../core/board.h"
#include "../network/train.h"
#include "../network/eval.h"
#include "../search/search_manager.h"
#include "../selfplay/selfplay.h"
#include "../tests/perft.h"
#include "../utils/logger.h"
#include "../utils/split.h"
#include "../utils/utilities.h"
#include "command.h"
#include "option.h"

namespace uci {

    core::Move move_from_string(core::Board &board, const std::string &str) {
        core::Move moves[200];
        core::Move *moves_end = core::gen_moves(board, moves, false);
        for (core::Move *it = moves; it != moves_end; it++) {
            if (it->to_uci() == str) {
                return *it;
            }
        }
        return core::NULL_MOVE;
    }

    class UCI {

        using context = std::vector<std::string>;

    public:
        void start();

        template<typename T>
        T get_option(const std::string &name);

    private:
        std::vector<Command> commands;
        std::vector<Option> options;
        bool should_continue = true;
        core::Board board;
        search::SearchManager sm;

        void register_commands();

        void register_options();

        void greetings();

        search::Limits parse_limits(context tokens);

        void parse_position(context tokens);

        static std::vector<std::string> convert_to_tokens(const std::string &line);

        template<typename T>
        static std::optional<T> find_element(const context &haystack, std::string_view needle);
    };

    void UCI::register_commands() {
        commands.emplace_back("uci", [&](context tokens) {
            greetings();
            search::report::set_pretty_output(false);
        });
        commands.emplace_back("pretty", [&](context tokens){
            search::report::set_pretty_output(true);
        });
        commands.emplace_back("isready", [&](context tokens) {
            Logger("readyok");
        });
        commands.emplace_back("position", [&](context tokens) {
            parse_position(tokens);
        });
        commands.emplace_back("display", [&](context tokens) {
            board.display();
        });
        commands.emplace_back("eval", [&](context tokens) {
            nn::NNUE network{};
            network.refresh(board.to_features());
            Logger("Eval:", eval::evaluate(network, board));
        });
        commands.emplace_back("gen", [&](context tokens) {
            search::Limits limits;
            limits.max_nodes = find_element<int64_t>(tokens, "nodes");
            limits.depth = find_element<int64_t>(tokens, "depth");
            std::optional<int> thread_count = find_element<int>(tokens, "threads");
            std::optional<int> games_to_play = find_element<int>(tokens, "games");
            selfplay::start_generation(limits, games_to_play.value_or(100'000), thread_count.value_or(1));
        });
        commands.emplace_back("split", [&](context tokens) {
            std::optional<std::string> input_data = find_element<std::string>(tokens, "input");
            std::optional<std::string> output_data1 = find_element<std::string>(tokens, "output1");
            std::optional<std::string> output_data2 = find_element<std::string>(tokens, "output2");
            std::optional<int> rate = find_element<int>(tokens, "rate");
            split_data(input_data.value_or("data.plain"), output_data1.value_or("train.plain"), output_data2.value_or("validation.plain"), rate.value_or(10));
        });
        commands.emplace_back("quantize", [&](context tokens) {
            std::optional<std::string> input = find_element<std::string>(tokens, "input");
            std::optional<std::string> output = find_element<std::string>(tokens, "output");
            nn::Network network_file(input.value_or("input.bin"));
            network_file.quantize<int16_t, nn::NNUE::QSCALE>(output.value_or("output.bin"));
        });
        commands.emplace_back("train", [&](context tokens) {
            std::optional<std::string> network_path = find_element<std::string>(tokens, "network");
            std::optional<std::string> training_data = find_element<std::string>(tokens, "training_data");
            std::optional<std::string> validation_data = find_element<std::string>(tokens, "validation_data");
            std::optional<float> learning_rate = find_element<float>(tokens, "lr");
            std::optional<size_t> epochs = find_element<size_t>(tokens, "epochs");
            std::optional<size_t> batch_size = find_element<size_t>(tokens, "batch");
            std::optional<size_t> threads = find_element<size_t>(tokens, "threads");
            nn::Trainer trainer(training_data.value_or("train.plain"), validation_data.value_or("validation.plain"), network_path, learning_rate.value_or(0.001f),
                                epochs.value_or(10), batch_size.value_or(16384), threads.value_or(4));
        });
        commands.emplace_back("perft", [&](context tokens) {
            int depth = find_element<int>(tokens, "perft").value_or(5);
            U64 node_count = test::perft<true, false>(board, depth);
            Logger("Total node count: ", node_count);
        });
        commands.emplace_back("go", [&](context tokens) {
            search::Limits limits = parse_limits(tokens);
            sm.set_limits(limits);
            sm.search<false>(board);
        });
        commands.emplace_back("stop", [&](context tokens) {
            sm.stop();
        });
        commands.emplace_back("quit", [&](context tokens) {
            should_continue = false;
            sm.stop();
        });
        commands.emplace_back("ucinewgame", [&](context tokens) {
            sm.tt_clear();
        });
        commands.emplace_back("setoption", [&](context tokens) {
            const std::string name = find_element<std::string>(tokens, "name").value_or("none");
            const std::optional<std::string> value = find_element<std::string>(tokens, "value");
            for (Option &opt : options) {
                if (opt.get_name() == name) {
                    opt.set_value(value);
                }
            }
        });
    }

    void UCI::register_options() {
        options.emplace_back(
                "Hash", "32", "spin", [&]() {
                    sm.allocate_hash(get_option<int>("Hash"));
                },
                1, 65536);
        sm.allocate_hash(32);

        options.emplace_back(
                "Threads", "1", "spin", [&]() {
                    sm.allocate_threads(get_option<int>("Threads"));
                },
                1, 128);
        sm.allocate_threads(1);
    }

    void UCI::start() {

        register_commands();
        register_options();

        board.load(STARTING_FEN);

        while (should_continue) {
            std::string line;
            getline(std::cin, line);

            if (std::cin.eof()) {
                break;
            }

            std::vector<std::string> tokens = convert_to_tokens(line);

            for (const Command &cmd : commands) {
                if (cmd.is_match(tokens)) {
                    cmd.func(tokens);
                }
            }
        }
    }

    void UCI::greetings() {
        Logger("id", "name", "WhiteCore", VERSION);
        Logger("id author Balazs Szilagyi");
        for (const Option &opt : options) {
            Logger(opt.to_string());
        }
        Logger("uciok");
    }

    search::Limits UCI::parse_limits(UCI::context tokens) {

        const std::string time_token = (board.get_stm() == WHITE) ? "wtime" : "btime";
        const std::string increment_token = (board.get_stm() == WHITE) ? "winc" : "binc";

        search::Limits limits;

        limits.time_left = find_element<int64_t>(tokens, time_token);
        limits.increment = find_element<int64_t>(tokens, increment_token);

        limits.moves_to_go = find_element<int64_t>(tokens, "movestogo");
        limits.depth = find_element<int64_t>(tokens, "depth");
        limits.move_time = find_element<int64_t>(tokens, "movetime");
        limits.max_nodes = find_element<int64_t>(tokens, "nodes");

        return limits;
    }

    void UCI::parse_position(UCI::context tokens) {
        unsigned int idx = 2;
        if (tokens[1] == "startpos") {
            board.load(STARTING_FEN);
        } else {
            std::string fen;
            for (; idx < tokens.size() && tokens[idx] != "moves"; idx++) {
                fen += tokens[idx] + " ";
            }
            board.load(fen);
        }
        if (idx < tokens.size() && tokens[idx] == "moves") idx++;
        for (; idx < tokens.size(); idx++) {
            core::Move move = move_from_string(board, tokens[idx]);
            if (move == core::NULL_MOVE) {
                break;
            } else {
                board.make_move(move);
            }
        }
    }

    std::vector<std::string> UCI::convert_to_tokens(const std::string &line) {
        std::vector<std::string> res = {""};
        for (char c : line) {
            if (c == ' ') {
                res.emplace_back("");
            } else {
                res.back() += c;
            }
        }
        return res;
    }

    template<typename T>
    T UCI::get_option(const std::string &name) {
        for (const Option &opt : options) {
            if (opt.get_name() == name) {
                return opt.get_value<T>();
            }
        }
        throw std::invalid_argument("UCI::get_option(): unable to find option " + name);
    }

    // Copied from fast-chess, I'm the co-author of the following code
    template<typename T>
    std::optional<T> UCI::find_element(const UCI::context &haystack, std::string_view needle) {
        auto position = std::find(haystack.begin(), haystack.end(), needle);
        auto index = position - haystack.begin();
        if (position == haystack.end()) return std::nullopt;
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>)
            return std::stoi(haystack[index + 1]);
        else if constexpr (std::is_same_v<T, float>)
            return std::stof(haystack[index + 1]);
        else if constexpr (std::is_same_v<T, int64_t>)
            return std::stoll(haystack[index + 1]);
        else if constexpr (std::is_same_v<T, uint64_t>)
            return std::stoull(haystack[index + 1]);
        else
            return haystack[index + 1];
    }

} // namespace uci
