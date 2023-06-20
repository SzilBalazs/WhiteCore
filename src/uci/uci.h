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

#include <iostream>

class UCI {
private:

	std::vector<Command> commands;

	static std::vector<std::string> convert_to_tokens(const std::string &line);

	static void is_ready(std::vector<std::string> tokens);

public:

	void start();
};

void UCI::start() {

	commands.emplace_back("isready", is_ready);

	logger.info("UCI", "UCI loop has started");

	while (true) {
		std::string line;
		getline(std::cin, line);

		if (std::cin.eof()) {
			break;
		}

		logger.info("UCI", "in> " + line);

		std::vector<std::string> tokens = convert_to_tokens(line);

		if (tokens[0] == "stop") {
			break;
		}

		for (Command cmd : commands) {
			if (cmd.is_match(tokens)) {
				cmd.func(tokens);
			}
		}
	}
}

void UCI::is_ready(std::vector<std::string> tokens) {
	std::cout << "readyok" << std::endl;
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
