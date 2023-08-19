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

#include "../utils/utilities.h"
#include "adam.h"
#include "data_parser.h"

#include <filesystem>
#include <optional>
#include <string>

namespace nn {

    constexpr unsigned int PROGRESS_BAR_WIDTH = 25;

    class Trainer {
    public:
        Trainer(const std::string &training_data, const std::string &validation_data, const std::optional<std::string> &network_path, float learning_rate, float eval_influence, size_t epochs, size_t batch_size, size_t thread_count) : adam(learning_rate), training_parser(training_data), validation_parser(validation_data), entry_count(0), batch_size(batch_size), thread_count(thread_count), eval_influence(eval_influence) {

            if (!std::filesystem::exists("networks")) {
                std::filesystem::create_directory("networks");
            }

            index_training_data(training_data);

            std::ofstream log_file("log.txt", std::ios::out);

            if (network_path) {
                network = Network(network_path.value());
            } else {
                network = Network();
            }

            entries = new std::string[batch_size];
            entries_next = new std::string[batch_size];

            bool _;
            training_parser.read_batch(batch_size, entries_next, _);

            int64_t iter = 0;
            for (size_t epoch = 1; epoch <= epochs; epoch++) {
                int64_t start_time = now();
                bool is_new_epoch = false;
                float checkpoint_error = 0.0f;
                int checkpoint_accuracy = 0;
                int64_t epoch_iter = 0;
                int64_t checkpoint_iter = 0;

                if (epoch == 15) {
                    adam.reduce_learning_rate(0.1);
                }

                while (!is_new_epoch) {
                    iter++;
                    epoch_iter++;
                    checkpoint_iter++;

                    delete[] entries;
                    entries = entries_next;
                    entries_next = new std::string[batch_size];

                    gradients.assign(thread_count, Gradient());
                    errors.assign(thread_count, 0.0f);
                    accuracy.assign(thread_count, 0);

                    std::thread th_loading = std::thread(&DataParser::read_batch, &training_parser, batch_size, entries_next, std::ref(is_new_epoch));

                    std::vector<std::thread> ths;
                    for (size_t id = 0; id < thread_count; id++) {
                        ths.emplace_back(&Trainer::process_batch<true>, this, id);
                    }

                    for (std::thread &th : ths) {
                        if (th.joinable()) th.join();
                    }

                    adam.update(gradients, network);

                    for (size_t id = 0; id < thread_count; id++) {
                        checkpoint_error += errors[id];
                        checkpoint_accuracy += accuracy[id];
                    }

                    if (th_loading.joinable()) th_loading.join();

                    if (iter % 10 == 0) {
                        float average_error = checkpoint_error / float(batch_size * checkpoint_iter * 2);
                        float average_accuracy = float(checkpoint_accuracy) / float(batch_size * checkpoint_iter * 2);
                        auto [val_loss, val_acc] = test_validation();

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


                        log_file << iter << " " << average_error << " " << pos_per_s << " " << average_accuracy << " " << val_loss << " " << val_acc << "\n";
                        log_file.flush();
                        checkpoint_error = 0.0f;
                        checkpoint_accuracy = 0;
                        checkpoint_iter = 0;
                    }
                }
                std::cout << std::endl;
                network.write_to_file("networks/epoch-" + std::to_string(epoch) + ".bin");
            }

            log_file << "END\n";
            log_file.close();
        }

    private:
        Network network;
        Adam adam;
        DataParser training_parser;
        DataParser validation_parser;
        size_t entry_count;
        size_t batch_size, thread_count;
        float eval_influence;
        std::vector<Gradient> gradients;
        std::vector<float> errors;
        std::vector<int> accuracy;
        std::string *entries, *entries_next;

        std::pair<float, float> test_validation() {
            bool _;
            delete[] entries;
            entries = new std::string[batch_size];
            validation_parser.read_batch(batch_size, entries, _);

            errors.assign(thread_count, 0.0f);
            accuracy.assign(thread_count, 0);

            std::vector<std::thread> ths;
            for (size_t id = 0; id < thread_count; id++) {
                ths.emplace_back(&Trainer::process_batch<false>, this, id);
            }

            for (std::thread &th : ths) {
                if (th.joinable()) th.join();
            }

            float val_loss = 0.0f;
            int correct = 0.0f;

            for (size_t id = 0; id < thread_count; id++) {
                val_loss += errors[id];
                correct += accuracy[id];
            }

            val_loss /= batch_size * 2;

            float val_acc = float(correct) / float(batch_size * 2);
            return {val_loss, val_acc};
        }

        template<bool train>
        void process_batch(int id) {

            for (size_t i = id; i < batch_size; i += thread_count) {
                TrainingEntry entry(entries[i]);

                process_entry<train>(id, entry.white_features, entry.wdl, entry.eval);
                process_entry<train>(id, entry.black_features, 1.0f - entry.wdl, 1.0f - entry.eval);
            }
        }

        template<bool train>
        void process_entry(int id, const std::vector<unsigned int> &features, float wdl, float eval) {
            std::array<float, L1_SIZE> l0_output;
            std::array<float, L1_SIZE> l0_loss;
            std::array<float, 1> l1_output;

            network.forward(features, l0_output, l1_output);
            float prediction = l1_output[0];

            float error = (1.0f - eval_influence) * (prediction - wdl) * (prediction - wdl) +
                          eval_influence * (prediction - eval) * (prediction - eval);
            errors[id] += error;
            accuracy[id] += ((wdl - 0.5f) * (prediction - 0.5f) > 0.0f) || std::abs(wdl - prediction) < 0.05f;

            if constexpr (train) {
                std::array<float, 1> l1_loss = {(1 - eval_influence) * 2.0f * (prediction - wdl) + eval_influence * 2.0f * (prediction - eval)};

                network.l1.backward(l1_loss, l0_output, l1_output, l0_loss, gradients[id].l1);
                network.l0.backward(l0_loss, features, l0_output, gradients[id].l0);
            }
        }

        void index_training_data(const std::string &training_data) {
            print("Indexing training data...");

            std::string tmp;
            std::ifstream file(training_data, std::ios::in);
            while (std::getline(file, tmp)) {
                entry_count++;
            }
            file.close();

            print("Found", entry_count, "positions");
        }
    };
} // namespace nn