#include "ConsoleManager.hpp"
#include "ConsoleScreen.hpp"
#include "Globals.hpp"

#include <map>
#include <ctime>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <iostream>

ProcessManager* GLOBAL_PM = nullptr;
bool GLOBAL_SHUTTING_DOWN = false;

int main() {
    ConsoleManager manager;
    ConsoleScreen console;
    std::string command;

    system("cls");
    console.displayHeader();

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        if (command == "exit") {
            GLOBAL_SHUTTING_DOWN = true;
            manager.getInput("exit");
            manager.getInput("scheduler-stop");
            break;
        }

        manager.getInput(command);
    }

    return 0;
}
