#include <bits/stdc++.h>

using namespace std;
using ll = long long;

const int CNT = 10;

int main() {
    ifstream input_data("data.plain");
    vector<ofstream> output_data;
    for (int i = 0; i < CNT; i++) {
        output_data.emplace_back("block" + std::to_string(i) + ".plain");
    }
    mt19937 rng(42);
    string line;
    while (getline(input_data, line)) {
        int block = rng() % CNT;
        output_data[block] << line << "\n";
    }
    for (int i = 0; i < CNT; i++) {
        output_data[i].close();
    }
    for (int i = 0; i < CNT; i++) {
        system(("shuf block" + to_string(i) + ".plain -o shuffled_block" + to_string(i) + ".plain").c_str());
    }
    for (int i = 0; i < CNT; i++) {
        system(("rm -f block" + to_string(i) + ".plain").c_str());
    }
    ofstream output("shuffled.plain");
    for (int i = 0; i < CNT; i++) {
        ifstream inp("shuffled_block" + to_string(i) + ".plain");
        string l;
        while (getline(inp, l)) {
            output << l << "\n";
        }
    }
    for (int i = 0; i < CNT; i++) {
        system(("rm -f shuffled_block" + to_string(i) + ".plain").c_str());
    }
    output.close();
    return 0;
}