#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm> 

//implement a simple command-line interface (CLI) application in C++ that can handle user input and 
// execute commands. The application should support the following commands: "help", "status", and "exit". The "help" 
// command should display a list of available commands, the "status" command should display information about the application, 
// and the "exit" command should terminate the application. The application should also handle unknown commands gracefully 
// by displaying an error message and prompting the user to enter a valid command.

namespace {
    
    std::string trim(const std::string& text)
    {
        //find the first non-whitespace character in the string
        const auto first = find_if_not(
            text.begin(),
            text.end(),
            [](unsigned char character){
                return std::isspace(character) != 0;} );
        //if the string is empty or contains only whitespace, return an empty string
        if (first == text.end())
        {
            return "";
        }

        // find the last non-whitespace character in the string
        const auto last = find_if_not(
            text.rbegin(),
            text.rend(),
            [](unsigned char character){
                return std::isspace(character) != 0;} );
        
        return std::string(first,last.base());
    }

    void printWelcomeMessage()
    {
        std::cout << "Welcome to C++ Local Inference Workbench!\n\n";
        std::cout << "Available commands.\n";
        std::cout << "- help\n";
        std::cout << "- status\n";
        std::cout << "- exit\n";   
    }
    
    std::string normalizeCommand(const std::string& command) {
        std::string normalized = trim(command);
        std::transform(
            normalized.begin(),
            normalized.end(),
            normalized.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return normalized;
    }

    void printHelp() {
        std::cout << "Available commands.\n";
        std::cout << "- help\n";
        std::cout << "- status\n";
        std::cout << "- exit\n";   
    }

    void printStatus() {
        std::cout<<"\nApplication : C++ Local LLM Infernece Workbench\n";
        std::cout<<"Project: C++ AI Inference & GPU Acceleration Lab\n";
        std::cout<<"Developement stage : phase 02 - Building a CLI Application\n";
        std::cout<<"Status: Initialization\n";
        std::cout<<"Inference backend: Not integrated yet\n";
    }
    bool handleCommand(const std::string& input) {
        const std::string command = normalizeCommand(input);
        if (command.empty()) return true; 
        if ( command == "help")
        {
            printHelp();
        }
        else if ( command == "status")
        {
            printStatus();  
        }
        else if (command == "exit")
        {
            std::cout<< "\n shutting down the application... "<< std::endl;
            return false;
        }
        else 
        {
            std::cout<< "\nUnknown Command.\n";
            std::cout<< "Type 'help' to see the list of available commands.\n";
        }
        return true; 
    }
    
    void runApplication()
    {
        printWelcomeMessage();
        std::string input;

        while (true)
        {
            std::cout<< "\n>" <<std::flush;
            if(!std::getline(std::cin, input))
            {
                std::cout<< "\nEnd of input detected. Shutting down the application... "<< std::endl;
                break;
            }
            if (!handleCommand(input)) //false == Exit command was entered
            {
                break;
            }
        }
    }
}
int main()
{
    runApplication();

    return EXIT_SUCCESS;
}
