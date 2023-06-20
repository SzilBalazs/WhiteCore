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

#include "command.h"
#include "../utils/logger.h"
#include "../utils/utilities.h"
#include "../core/board.h"
#include "../tests/perft.h"
#include "../eval/eval.h"
#include "../search/search.h"

#include <iostream>

inline Move move_from_string(Board &board, const std::string &str) {
	Move moves[200];
	Move *moves_end = Movegen::gen_moves(board, moves, false);
	for (Move *it = moves; it != moves_end; it++) {
		if (it->to_uci() == str) {
			return *it;
		}
	}
	return NULL_MOVE;
}

class UCI {

	using context = std::vector<std::string>;

public:

	void start();

private:

	std::vector<Command> commands;
	bool should_continue = true;
	Board board;
	Searcher searcher;

	void register_commands();

	static std::vector<std::string> convert_to_tokens(const std::string &line);
};

void UCI::register_commands() {
	commands.emplace_back("uci", [&](context tokens){
		std::cout << "id name WhiteCore v0.1\nid author Balazs Szilagyi\nuciok\n" << std::flush;
	});
	commands.emplace_back("isready", [&](context tokens) {
		std::cout << "readyok" << std::endl;
	});
	commands.emplace_back("quit", [&](context tokens) {
		should_continue = false;
	});
	commands.emplace_back("position", [&](context tokens){
		unsigned int idx = 2;
		if (tokens[1] == "startpos") {
			board.load(STARTING_FEN);
		} else {
			std::string fen;
			for (;idx < tokens.size() && tokens[idx] != "moves"; idx++) {
				fen += tokens[idx] + " ";
			}
			board.load(fen);
		}
		if (idx < tokens.size() && tokens[idx] == "moves") idx++;
		for (;idx < tokens.size(); idx++) {
			Move move = move_from_string(board, tokens[idx]);
			if (move == NULL_MOVE) {
				logger.error("load_position", "Invalid move", tokens[idx]);
				break;
			} else {
				board.make_move(move);
			}
		}
	});
	commands.emplace_back("display", [&](context tokens){
		board.display();
	});
	commands.emplace_back("perft", [&](context tokens) {
		int depth = std::stoi(tokens[1]);
		U64 node_count = Tests::perft<true, false>(board, depth);
		std::cout << "Total node count: " << node_count << std::endl;
	});
	commands.emplace_back("go", [&](context tokens){
		searcher.load_board(board);
		Move best_move = searcher.search(1'000'000);
		std::cout << "bestmove " << best_move << std::endl;
	});
	commands.emplace_back("stop", [&](context tokens){
		// TODO
	});
	commands.emplace_back("ucinewgame", [&](context tokens){
		// TODO
	});

	logger.info("UCI::register_commands", "Registered ", commands.size(), "commands");
}

void UCI::start() {

	register_commands();

	board.load(STARTING_FEN);

	logger.info("UCI::start", "UCI Loop has started!");

	while (should_continue) {
		std::string line;
		getline(std::cin, line);

		if (std::cin.eof()) {
			break;
		}

		logger.info("UCI::start", "in>", line);

		std::vector<std::string> tokens = convert_to_tokens(line);

		for (Command cmd : commands) {
			if (cmd.is_match(tokens)) {
				cmd.func(tokens);
			}
		}
	}
}

std::vector<std::string> UCI::convert_to_tokens(const std::string& line) {
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
