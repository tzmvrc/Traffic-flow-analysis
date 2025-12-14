/*
 * main.cpp - Traffic Analysis System Core
 * Build: g++ -std=c++17 -Wall -O2 main.cpp -o traffic_dsl
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct Task {
    std::string keyword;
    std::vector<std::string> operands;
};

struct Globals {
    double v_free = 0.0;
    double k_jam  = 0.0;
    std::vector<double> k_vec;
    std::vector<double> v_vec;
    std::vector<double> q_vec;
    double q_max  = 0.0;
    double k_opt  = 0.0;
    std::string csv_filename;
} g;

std::vector<Task> readSymbolicProgram(const std::string& filename);
void executeTasks(const std::vector<Task>& prog);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Traffic Analysis System (CLI mode)\n";
        std::cout << "==================================\n";
        std::cout << "Usage: traffic_dsl.exe <program.txt>\n\n";
        std::cout << "Example: traffic_dsl.exe input/sample.txt\n\n";
        std::cout << "For interactive menu, run: menu.exe\n";
        return 1;
    }

    // Create directories if they don't exist
    fs::create_directory("input");
    fs::create_directory("output");

    try {
        auto prog = readSymbolicProgram(argv[1]);
        executeTasks(prog);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 2;
    }

    return 0;
}

std::vector<Task> readSymbolicProgram(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) {
        fin.open("input/" + filename);
        if (!fin) throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<Task> tasks;
    std::string line;
    int line_num = 0;

    while (std::getline(fin, line)) {
        line_num++;
        std::stringstream ss(line);
        std::string kw;
        ss >> kw;

        if (kw.empty() || kw[0] == '#') continue;

        Task t;
        t.keyword = kw;

        std::string op;
        while (ss >> op) t.operands.push_back(op);

        tasks.push_back(t);
    }

    if (tasks.empty()) throw std::runtime_error("No valid commands in file");
    return tasks;
}

void executeTasks(const std::vector<Task>& prog) {
    for (size_t i = 0; i < prog.size(); ++i) {
        const auto& t = prog[i];

        try {
            if (t.keyword == "FREE_FLOW") {
                if (t.operands.empty()) throw std::runtime_error("FREE_FLOW requires speed value");
                g.v_free = std::stod(t.operands[0]);
                std::cout << "[INFO] Free-flow speed: " << g.v_free << " km/h\n";
            }
            else if (t.keyword == "JAM_DENSITY") {
                if (t.operands.empty()) throw std::runtime_error("JAM_DENSITY requires density value");
                g.k_jam = std::stod(t.operands[0]);
                std::cout << "[INFO] Jam density: " << g.k_jam << " veh/km\n";
            }
            else if (t.keyword == "DENSITY_RANGE") {
                if (t.operands.size() < 3) throw std::runtime_error("DENSITY_RANGE requires start, end, step");
                double s = std::stod(t.operands[0]);
                double e = std::stod(t.operands[1]);
                double step = std::stod(t.operands[2]);
                g.k_vec.clear();
                for (double k = s; k <= e + 1e-6; k += step)
                    g.k_vec.push_back(k);
                std::cout << "[INFO] Density range: " << s << " to " << e 
                         << " step " << step << " (" << g.k_vec.size() << " points)\n";
            }
            else if (t.keyword == "COMPUTE_SPEED") {
                if (g.k_vec.empty()) throw std::runtime_error("Need density values first");
                if (g.v_free == 0.0 || g.k_jam == 0.0) throw std::runtime_error("Set FREE_FLOW and JAM_DENSITY first");
                
                g.v_vec.clear();
                for (double k : g.k_vec)
                    g.v_vec.push_back(g.v_free * (1.0 - k / g.k_jam));
                std::cout << "[INFO] Speed computed for " << g.k_vec.size() << " points\n";
            }
            else if (t.keyword == "COMPUTE_FLOW") {
                if (g.k_vec.empty() || g.v_vec.empty()) throw std::runtime_error("Need density and speed values first");
                
                g.q_vec.clear();
                for (size_t j = 0; j < g.k_vec.size(); ++j)
                    g.q_vec.push_back(g.k_vec[j] * g.v_vec[j]);
                std::cout << "[INFO] Flow computed for " << g.k_vec.size() << " points\n";
            }
            else if (t.keyword == "CAPACITY") {
                if (g.q_vec.empty()) throw std::runtime_error("Need flow values first");
                
                auto it = std::max_element(g.q_vec.begin(), g.q_vec.end());
                g.q_max = *it;
                g.k_opt = g.k_vec[it - g.q_vec.begin()];
                std::cout << "[INFO] Capacity: q_max = " << g.q_max 
                         << " veh/h at k = " << g.k_opt << " veh/km\n";
            }
            else if (t.keyword == "EXPORT_CSV") {
                if (t.operands.empty()) throw std::runtime_error("EXPORT_CSV requires filename");
                if (g.k_vec.empty() || g.v_vec.empty() || g.q_vec.empty()) 
                    throw std::runtime_error("Need data to export");
                
                g.csv_filename = t.operands[0];
                std::ofstream csv("output/" + g.csv_filename + ".csv");
                csv << "k,v,q\n";
                for (size_t j = 0; j < g.k_vec.size(); ++j)
                    csv << g.k_vec[j] << ","
                        << g.v_vec[j] << ","
                        << g.q_vec[j] << "\n";
                csv.close();
                std::cout << "[INFO] CSV exported: output/" << g.csv_filename << ".csv\n";
            }
            else if (t.keyword == "PRINT_RESULTS") {
                if (g.q_vec.empty()) throw std::runtime_error("No results to print");
                
                std::cout << "\n" << std::string(50, '=') << "\n";
                std::cout << "FINAL ANALYSIS RESULTS:\n";
                std::cout << std::string(50, '=') << "\n";
                std::cout << "Free-flow speed: " << g.v_free << " km/h\n";
                std::cout << "Jam density: " << g.k_jam << " veh/km\n";
                std::cout << "Maximum flow: " << g.q_max << " veh/h\n";
                std::cout << "Optimal density: " << g.k_opt << " veh/km\n";
                std::cout << "Number of data points: " << g.k_vec.size() << "\n";
                std::cout << "CSV file: output/" << g.csv_filename << ".csv\n";
                std::cout << std::string(50, '=') << "\n";
                
                // Output for Python plotter to find
                std::cout << "PLOT_DATA:" << g.csv_filename << "\n";
            }
            else {
                std::cout << "[WARNING] Unknown command: " << t.keyword << "\n";
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Line " + std::to_string(i + 1) + ": " + e.what());
        }
        catch (...) {
            throw std::runtime_error("Line " + std::to_string(i + 1) + ": Unknown error");
        }
    }
}