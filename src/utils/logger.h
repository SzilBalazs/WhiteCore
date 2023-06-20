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

#include <string>
#include <fstream>

static void exception_handler();

struct Logger {

	std::ofstream file;

	Logger() = default;

	void init(const std::string& filename) {
		file.open(filename, std::ios::out);
		std::set_terminate(exception_handler);
	}

	void info(const std::string& str) {
		file << "[GLOBAL][INFO] " << str << std::endl;
	}

	void warning(const std::string& str) {
		file << "[GLOBAL][WARNING] " << str << std::endl;
	}

	void error(const std::string& str) {
		file << "[GLOBAL][ERROR] " << str << std::endl;
	}

	void info(const std::string& module, const std::string& str) {
		file << "[" << module << "][INFO] " << str << std::endl;
	}

	void warning(const std::string& module, const std::string& str) {
		file << "[" << module << "][WARNING] " << str << std::endl;
	}

	void error(const std::string& module, const std::string& str) {
		file << "[" << module << "][ERROR] " << str << std::endl;
	}
};

static Logger logger;

static void exception_handler() {
	if (logger.file.is_open()) {
		logger.error("exception_handler was called: closing log file and aborting");
		logger.file.close();
	}
	std::abort();
}

