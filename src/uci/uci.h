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

#include <iostream>

class UCI {

	using context = std::vector<std::string>;

public:

	void start();

private:

	std::vector<Command> commands;
	bool should_continue = true;
	Board board;

	void register_commands();

	static std::vector<std::string> convert_to_tokens(const std::string &line);
};

void UCI::register_commands() {
	commands.emplace_back("isready", [&](context tokens) {
		std::cout << "readyok" << std::endl;
	});
	commands.emplace_back("quit", [&](context tokens) {
		should_continue = false;
	});
	commands.emplace_back("pos", [&](context tokens){
		std::string fen;
		for (unsigned int idx = 1; idx < tokens.size(); idx++) {
			fen += tokens[idx] + " ";
		}
		board.board_load(fen);
	});
	commands.emplace_back("d", [&](context tokens){
		board.display();
	});
	commands.emplace_back("perft", [&](context tokens) {
		int depth = std::stoi(tokens[1]);
		U64 node_count = perft<true, false>(board, depth);
		std::cout << "Total node count: " << node_count << std::endl;
	});
	commands.emplace_back("pd", [&](context tokens) {
		int depth = std::stoi(tokens[1]);
		U64 node_count = perft<false, true>(board, depth);
		std::cout << "Total node count: " << node_count << std::endl;
	});

	logger.info("UCI::register_commands", "Registered ", commands.size(), "commands");
}

void UCI::start() {

	init_all();
	register_commands();

	board.board_load(STARTING_FEN);

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
