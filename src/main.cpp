#include <iostream>
#include <string>
#include "commands.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: minigit <command>\n";
        return 1;
    }

    std::string command = argv[1];  

    if (command == "init") {
        Result r = cmd_init();
        std::cout << r.message << "\n";
        return r.ok ? 0 : 1;
    }
        if (command == "hash-object") {
        if (argc < 3) {
            std::cout << "usage: minigit hash-object <file> [-w]\n";
            return 1;
        }
        bool write = (argc >= 4 && std::string(argv[3]) == "-w");
        Result r = cmd_hash_object(argv[2], write);
        std::cout << r.message << "\n";
        return r.ok ? 0 : 1;
    }
// cat file block
        if (command == "cat-file") {
        if (argc < 3) {
            std::cout << "usage: minigit cat-file <hash>\n";
            return 1;
        }
        Result r = cmd_cat_file(argv[2]);
        std::cout << r.message << "\n";
        return r.ok ? 0 : 1;
    }
// add file block

    if (command == "add") {
        if (argc < 3) {
            std::cout << "usage: minigit add <file>\n";
            return 1;
        }
        Result r = cmd_add(argv[2]);
        std::cout << r.message << "\n";
        return r.ok ? 0 : 1;
    }
    // commit 
        if (command == "commit") {
        if (argc < 4 || std::string(argv[2]) != "-m") {
            std::cout << "usage: minigit commit -m \"message\"\n";
            return 1;
        }
        Result r = cmd_commit(argv[3]);
        std::cout << r.message << "\n";
        return r.ok ? 0 : 1;
    }

    // log
        if (command == "log") {
        Result r = cmd_log();
        std::cout << r.message;
        return r.ok ? 0 : 1;
    }

    std::cout << "unknown command: " << command << "\n";
    return 1;
}