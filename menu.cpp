/*
 * menu.cpp - Traffic Analysis Menu System
 * Build: g++ -std=c++17 -Wall -O2 menu.cpp -o menu
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <cstdio>

namespace fs = std::filesystem;

// Function declarations
void clearScreen();
void showHeader();
int showMainMenu();
int getUserChoice(int min, int max);
void createScenario();
std::string listScenarios();
bool runAnalysis(const std::string& inputFile, const std::string& scenarioName);
void showPlotForFile(const std::string& csvName);
void showSummaryAndContinue(const std::string& scenarioName);
void printLine(char ch = '=', int length = 50);

int main() {
    // Create directories
    fs::create_directory("input");
    fs::create_directory("output");
    
    // Check if traffic_dsl exists
    std::string trafficExe = "traffic_dsl.exe";
    if (!fs::exists(trafficExe)) {
        std::cout << "\n";
        printLine('-');
        std::cout << "  WARNING: traffic_dsl.exe not found!\n";
        std::cout << "  To compile: g++ -std=c++17 -Wall -O2 main.cpp -o traffic_dsl.exe\n";
        printLine('-');
        std::cout << "\nPress Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
    
    // Main loop
    while (true) {
        showHeader();
        int choice = showMainMenu();
        
        switch (choice) {
            case 1:
                createScenario();
                break;
            case 2:
                {
                    std::string selectedFile = listScenarios();
                    if (!selectedFile.empty()) {
                        std::string scenarioName = fs::path(selectedFile).stem().string();
                        runAnalysis(selectedFile, scenarioName);
                    }
                }
                break;
            case 3:
                // Exit
                showHeader();
                std::cout << "\n";
                printLine('=');
                std::cout << "  Thank you for using Traffic Analysis System\n";
                printLine('=');
                std::cout << "\n  Your analysis files:\n";
                std::cout << "    - input/  : Scenario files (.txt)\n";
                std::cout << "    - output/ : Results (.csv) and plots (.png)\n";
                printLine('=');
                std::cout << "\n  Goodbye!\n\n";
                return 0;
        }
    }
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printLine(char ch, int length) {
    std::cout << std::string(length, ch) << std::endl;
}

void showHeader() {
    clearScreen();
    
    // Clean ASCII Art Title
    std::cout << "\n";
    printLine('=', 60);
    std::cout << R"(
 ______  _______   _______  ______  ______  __   _______
|_    _||   _   \ |   _   ||   ___||   ___||  | |   __  |
  |  |  |  |_|   ||  |_|  ||  |___ |  |___ |  | |  |  |_|
  |  |  |   __  \ |   _   ||   ___||   ___||  | |  |   _ 
  |  |  |  |  \  \|  | |  ||  |    |  |    |  | |  |__| |   
  |__|  |__|   \__|__| |__||__|    |__|    |__| |_______|
)" << "\n";


    
    printLine('=', 60);
    std::cout << "            Greenshields Traffic Flow Analysis\n";
    printLine('-', 60);
}

int showMainMenu() {
    std::cout << "\n";
    std::cout << "                         MAIN MENU\n";
    printLine('-', 60);
    std::cout << "\n";
    
    std::cout << "  1. Create & Run New Scenario\n";
    std::cout << "  2. Run Existing Scenario\n";
    std::cout << "  3. Exit\n";
    
    std::cout << "\n";
    std::cout << "  0. Back\n";
    
    printLine('=', 60);
    
    return getUserChoice(1, 3);
}

int getUserChoice(int min, int max) {
    int choice;
    while (true) {
        std::cout << "\n  Select option (" << min << "-" << max << "): ";
        std::string input;
        std::getline(std::cin, input);
        
        // Check for 0 (back) option
        if (input == "0") {
            return 0;
        }
        
        try {
            choice = std::stoi(input);
            if (choice >= min && choice <= max) {
                return choice;
            }
        }
        catch (...) {
            // Not a valid number
        }
        std::cout << "  Error: Enter number between " << min << " and " << max << "\n";
    }
}

void createScenario() {
    showHeader();
    std::cout << "\n";
    std::cout << "  CREATE NEW SCENARIO\n";
    printLine('=', 60);
    std::cout << "\n";
    
    // Get scenario name
    std::cout << "  Scenario name: ";
    std::string scenarioName;
    std::getline(std::cin, scenarioName);
    
    if (scenarioName.empty()) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&now_time);
        std::stringstream ss;
        ss << std::put_time(&tm, "scenario_%Y%m%d_%H%M%S");
        scenarioName = ss.str();
        std::cout << "  Using default: " << scenarioName << "\n";
    }
    
    printLine('-', 50);
    std::cout << "\n  Enter parameters:\n";
    
    double v_free, k_jam, start, end, step;
    
    std::cout << "\n  Free-flow speed (km/h) [100]: ";
    std::string input;
    std::getline(std::cin, input);
    v_free = input.empty() ? 100.0 : std::stod(input);
    
    std::cout << "  Jam density (veh/km) [200]: ";
    std::getline(std::cin, input);
    k_jam = input.empty() ? 200.0 : std::stod(input);
    
    std::cout << "  Start density [0]: ";
    std::getline(std::cin, input);
    start = input.empty() ? 0.0 : std::stod(input);
    
    std::cout << "  End density [" << k_jam << "]: ";
    std::getline(std::cin, input);
    end = input.empty() ? k_jam : std::stod(input);
    
    std::cout << "  Step size [5]: ";
    std::getline(std::cin, input);
    step = input.empty() ? 5.0 : std::stod(input);
    
    // Create input file
    std::string filename = "input/" + scenarioName + ".txt";
    std::ofstream fout(filename);
    
    if (!fout) {
        std::cout << "\n";
        printLine('-');
        std::cout << "  Error creating file: " << filename << "\n";
        printLine('-');
        std::cout << "\n  Press Enter to continue...";
        std::getline(std::cin, input);
        return;
    }
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    std::stringstream time_ss;
    time_ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    fout << "# Traffic Scenario: " << scenarioName << "\n";
    fout << "# Created: " << time_ss.str() << "\n";
    fout << "FREE_FLOW        " << v_free << "\n";
    fout << "JAM_DENSITY      " << k_jam << "\n";
    fout << "DENSITY_RANGE    " << start << " " << end << " " << step << "\n";
    fout << "COMPUTE_SPEED\n";
    fout << "COMPUTE_FLOW\n";
    fout << "CAPACITY\n";
    fout << "EXPORT_CSV       " << scenarioName << "\n";
    fout << "PRINT_RESULTS\n";
    
    fout.close();
    
    std::cout << "\n";
    printLine('-');
    std::cout << "  Scenario saved: " << filename << "\n";
    printLine('-');
    
    // Ask to run it
    std::cout << "\n  Run this scenario now? (y/n): ";
    std::string runNow;
    std::getline(std::cin, runNow);
    
    if (runNow == "y" || runNow == "Y") {
        runAnalysis(filename, scenarioName);
    } else {
        std::cout << "\n  Press Enter to return to menu...";
        std::getline(std::cin, input);
    }
}

std::string listScenarios() {
    std::vector<std::string> files;
    
    if (fs::exists("input")) {
        for (const auto& entry : fs::directory_iterator("input")) {
            if (entry.path().extension() == ".txt") {
                files.push_back(entry.path().filename().string());
            }
        }
    }
    
    if (files.empty()) {
        showHeader();
        std::cout << "\n";
        std::cout << "  AVAILABLE SCENARIOS\n";
        printLine('=', 60);
        std::cout << "\n  No scenarios found in 'input/' folder\n";
        std::cout << "\n  Create a new scenario first.\n";
        printLine('-');
        std::cout << "\n  Press Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return "";
    }
    
    showHeader();
    std::cout << "\n";
    std::cout << "  AVAILABLE SCENARIOS\n";
    printLine('=', 60);
    std::cout << "\n  Select a scenario:\n\n";
    
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << files[i] << "\n";
    }
    
    std::cout << "\n";
    std::cout << "  0. Back\n";
    printLine('=', 60);
    
    int choice;
    while (true) {
        std::cout << "\n  Select (1-" << files.size() << " or 0): ";
        std::string input;
        std::getline(std::cin, input);
        
        try {
            choice = std::stoi(input);
            if (choice == 0) {
                return "";
            }
            if (choice >= 1 && choice <= static_cast<int>(files.size())) {
                return "input/" + files[choice - 1];
            }
        }
        catch (...) {
            // Not a valid number
        }
        std::cout << "  Error: Enter number between 1 and " << files.size() << "\n";
    }
}

bool runAnalysis(const std::string& inputFile, const std::string& scenarioName) {
    showHeader();
    std::cout << "\n";
    std::cout << "  RUNNING ANALYSIS\n";
    printLine('=', 60);
    std::cout << "\n";
    
    std::cout << "  Input: " << inputFile << "\n";
    std::cout << "  Scenario: " << scenarioName << "\n\n";
    
    printLine('-');
    std::cout << "  Starting analysis...\n\n";
    
    std::string cmd = "traffic_dsl.exe \"" + inputFile + "\"";
    int result = system(cmd.c_str());
    
    if (result == 0) {
        showSummaryAndContinue(scenarioName);
        return true;
    } else {
        std::cout << "\n";
        printLine('-');
        std::cout << "  Analysis failed with code: " << result << "\n";
        std::cout << "  Check if traffic_dsl.exe is compiled.\n";
        printLine('-');
        std::cout << "\n  Press Enter to continue...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return false;
    }
}

void showPlotForFile(const std::string& csvName) {
    std::string csvPath = "output/" + csvName + ".csv";
    if (!fs::exists(csvPath)) {
        std::cout << "  File not found: " << csvPath << "\n";
        return;
    }
    
    std::cout << "\n";
    printLine('-');
    std::cout << "  Launching MATLAB Plotter...\n";
    std::cout << "  Please wait, opening MATLAB...\n";
    printLine('-');
    
    // Call MATLAB plot script
    std::string plotCmd = "matlab -batch \"quick_plot('" + csvName + "')\"";
    
    int result = system(plotCmd.c_str());
    
    if (result != 0) {
        std::cout << "\n";
        printLine('-');
        std::cout << "  Plotting issues detected\n";
        std::cout << "  Ensure MATLAB is installed and the quick_plot.m script is available.\n";
        printLine('-');
    }
}

void showSummaryAndContinue(const std::string& scenarioName) {
    std::cout << "\n";
    printLine('=');
    std::cout << "  ANALYSIS COMPLETE\n";
    printLine('=');
    
    // Show files created
    std::cout << "\n  Generated Files:\n";
    std::cout << "    - output/" << scenarioName << ".csv\n";
    
    // Check if plot exists or generate it
    std::string plotFile = "output/" + scenarioName + "_plot.png";
    if (!fs::exists(plotFile)) {
        std::cout << "    - Generating plot...\n";
        showPlotForFile(scenarioName);
    } else {
        std::cout << "    - output/" << scenarioName << "_plot.png\n";
    }
    
    // Read and display summary from CSV
    std::string csvPath = "output/" + scenarioName + ".csv";
    if (fs::exists(csvPath)) {
        try {
            std::ifstream csv(csvPath);
            std::string line;
            int lineCount = 0;
            double q_max = 0, k_opt = 0;
            
            // Skip header
            std::getline(csv, line);
            
            while (std::getline(csv, line)) {
                lineCount++;
                std::stringstream ss(line);
                std::string value;
                
                // Read k, v, q
                std::getline(ss, value, ',');
                double k = std::stod(value);
                std::getline(ss, value, ','); // v value
                std::getline(ss, value, ',');
                double q = std::stod(value);
                
                if (q > q_max) {
                    q_max = q;
                    k_opt = k;
                }
            }
            csv.close();
            
            if (lineCount > 0) {
                std::cout << "\n";
                printLine('-');
                std::cout << "  Summary:\n";
                std::cout << "    - Data points: " << lineCount << "\n";
                std::cout << "    - Max flow: " << std::fixed << std::setprecision(0) 
                          << q_max << " veh/h\n";
                std::cout << "    - Opt density: " << std::fixed << std::setprecision(1) 
                          << k_opt << " veh/km\n";
            }
        }
        catch (...) {
            // Ignore errors in reading CSV
        }
    }
    
    printLine('-');
    
    // Ask user if they want to go back to menu
    std::cout << "\n  Press Enter to return to menu...";
    std::string dummy;
    std::getline(std::cin, dummy);
}