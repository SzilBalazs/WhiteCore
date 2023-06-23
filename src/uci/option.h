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

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace uci {

    class Option {
    public:
        Option(const std::string &name, const std::string &default_value, const std::string &type,
               const std::optional<std::function<void()>> &func = std::nullopt,
               const std::optional<int> &min_value = std::nullopt, const std::optional<int> &max_value = std::nullopt) {
            this->name = name;
            this->type = type;
            this->func = func;
            this->default_value = default_value;
            this->value = default_value;
            this->min_value = min_value;
            this->max_value = max_value;
        }

        std::string get_name() const {
            return name;
        }

        std::string to_string() const {
            std::string res = "option";

            res += " name " + name;
            res += " type " + type;
            res += " default " + default_value;
            if (min_value && max_value) {
                res += " min " + std::to_string(min_value.value());
                res += " max " + std::to_string(max_value.value());
            }

            return res;
        }

        void set_value(const std::optional<std::string> &new_value) {
            value = new_value.value_or(default_value);
            if (func) {
                func.value()();
            }
        }

        template<typename T>
        T get_value() const {
            if constexpr (std::is_same_v<T, int>)
                return std::stoi(value);
            else
                return value;
        }


    private:
        std::string name;
        std::string value;
        std::string default_value;
        std::string type;
        std::optional<int> min_value, max_value;
        std::optional<std::function<void()>> func;
    };

} // namespace uci