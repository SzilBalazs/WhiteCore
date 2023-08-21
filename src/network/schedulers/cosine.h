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

namespace nn::schedulers {

    class CosineScheduler {
    public:
        CosineScheduler(int start_epoch, int end_epoch, float start_lr, float end_lr) : start(start_epoch), end(end_epoch), base_lr(start_lr), final_lr(end_lr) {}

        float operator()(int epoch) const {

            if (epoch < start) {
                return base_lr;
            }
            if (epoch > end) {
                return final_lr;
            }

            return final_lr + (base_lr - final_lr) * (1 + std::cos(M_PI * float(epoch - start) / float(end - start))) / 2;
        }

    private:
        int start, end;
        float base_lr, final_lr;

    };
}
