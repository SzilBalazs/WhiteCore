// WhiteCore is a C++ chess engine
// Copyright (c) 2022-2025 Balázs Szilágyi
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
            std::stringstream res;
            res << "option"
                << " name " << name
                << " type " << type
                << " default " << default_value;
            if (min_value && max_value) {
                res << " min " << min_value.value()
                    << " max " << max_value.value();
            }
            return res.str();
        }

        void set_value(const std::optional<std::string> &new_value) {
            value = new_value.value_or(default_value);
            update();
        }

        template<typename T>
        T get_value() const {
            if constexpr (std::is_same_v<T, int>)
                return std::stoi(value);
            else if constexpr (std::is_same_v<T, bool>)
                return value == "true";
            else
                return value;
        }

        void update() {
            if (func) {
                func.value()();
            }
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