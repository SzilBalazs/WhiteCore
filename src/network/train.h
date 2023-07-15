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

#include "../utils/logger.h"
#include "data_parser.h"
#include "adam.h"

#include <filesystem>
#include <optional>
#include <string>

namespace nn {

    constexpr unsigned int PROGRESS_BAR_WIDTH = 25;
    constexpr float EVAL_INFLUENCE = 0.5;

    class Trainer {
    public:

        Trainer(const std::string &training_data, const std::optional<std::string> &network_path, float learning_rate, int epochs, int batch_size, int thread_count) : adam(learning_rate), parser(training_data), batch_size(batch_size), thread_count(thread_count) {

            if (!std::filesystem::exists("networks")) {
                std::filesystem::create_directory("networks");
            }

            index_training_data(training_data);

            if (network_path) {
                network = Network(network_path.value());
            } else {
                network = Network();
            }

            entries = new TrainingEntry[batch_size];
            entries_next = new TrainingEntry[batch_size];

            bool _;
            parser.read_batch(batch_size, entries_next, _);

            int64_t iter = 0;
            for (int epoch = 1; epoch <= epochs; epoch++) {
                int64_t start_time = now();
                bool is_new_epoch = false;
                float total_error = 0.0f;
                int64_t epoch_iter = 0;

                while (!is_new_epoch) {
                    iter++;
                    epoch_iter++;

                    delete[] entries;
                    entries = entries_next;
                    entries_next = new TrainingEntry[batch_size];

                    gradients.assign(thread_count, Gradient());
                    errors.assign(thread_count, 0.0f);

                    std::thread th_loading = std::thread(&DataParser::read_batch, &parser, batch_size, entries_next, std::ref(is_new_epoch));

                    std::vector<std::thread> ths;
                    for (int id = 0; id < thread_count; id++) {
                        ths.emplace_back(&Trainer::process_batch, this, id);
                    }

                    for (std::thread &th : ths) {
                        if (th.joinable()) th.join();
                    }

                    adam.update(gradients, network);

                    for (int id = 0; id < thread_count; id++) {
                        total_error += errors[id];
                    }

                    if (th_loading.joinable()) th_loading.join();

                    if (iter % 10 == 0) {
                        float average_error = total_error / float(batch_size * epoch_iter);

                        int64_t current_time = now();
                        int64_t elapsed_time = current_time - start_time;

                        int64_t pos_per_s = (epoch_iter * batch_size * 1000) / elapsed_time;

                        float progress = float(epoch_iter) / float(entry_count / batch_size);
                        unsigned int progress_position = PROGRESS_BAR_WIDTH * progress;

                        int64_t total_time = elapsed_time / progress;
                        int64_t eta = (1.0f - progress) * total_time;

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
                        std::cout << "] - Epoch " << epoch << " - Iteration " << iter << " - Error " << average_error << " - ETA " << (eta / 1000) << "s - " << pos_per_s << " pos/s \r" << std::flush;

                    }
                }
                std::cout << std::endl;
                network.write_to_file("networks/" + std::to_string(epoch) + ".bin");
            }
        }

    private:
        Network network;
        Adam adam;
        DataParser parser;
        unsigned int entry_count;
        int batch_size, thread_count;
        std::vector<Gradient> gradients;
        std::vector<float> errors;
        TrainingEntry *entries, *entries_next;

        void process_batch(int id) {
            Gradient &g = gradients[id];

            for (int i = id; i < batch_size; i += thread_count) {
                TrainingEntry &entry = entries[i];

                std::array<float, L1_SIZE> l0_output;
                std::array<float, L1_SIZE> l0_loss;
                std::array<float, 1> l1_output;

                network.forward(entry.features, l0_output, l1_output, entry.phase);
                float prediction = l1_output[0];

                float error = (1.0f - EVAL_INFLUENCE) * (prediction - entry.wdl) * (prediction - entry.wdl)
                              + EVAL_INFLUENCE * (prediction - entry.eval) * (prediction - entry.eval);
                errors[id] += error;

                std::array<float, 1> l1_loss = {(1 - EVAL_INFLUENCE) * 2.0f * (prediction - entry.wdl) + EVAL_INFLUENCE * 2.0f * (prediction - entry.eval)};

                network.l1.backward(l1_loss, l0_output, l1_output, l0_loss, g.l1);
                network.l0.backward(l0_loss, entry.features, l0_output, g.l0);
            }
        }

        void index_training_data(const std::string &training_data) {
            logger.print("Indexing training data...");

            entry_count = 0;

            std::string tmp;
            std::ifstream file(training_data, std::ios::in);
            while (std::getline(file, tmp)) {
                entry_count++;
            }
            file.close();

            logger.print("Found", entry_count, "positions");
        }
    };
}