#pragma once
#include <string>

struct Result {
    bool ok;
    std::string message;
};

Result cmd_init(); 
Result cmd_hash_object(const std::string& filename, bool write);
Result cmd_cat_file(const std::string& hash);
Result cmd_add(const std::string& filename);
Result cmd_commit(const std::string& message);
Result cmd_log();