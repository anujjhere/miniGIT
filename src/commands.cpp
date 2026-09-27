#include "sha1.h"
#include<sstream>
#include "commands.h"
#include <fstream>
#include <sys/stat.h>
#include <map>


#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #define MKDIR(path) mkdir(path, 0755)
#endif

bool folderExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

Result cmd_init() {
    if (folderExists(".minigit")) {
        return {false, "minigit repository already exists here"};
    }

    MKDIR(".minigit");
    MKDIR(".minigit/objects");
    MKDIR(".minigit/refs");
    MKDIR(".minigit/refs/heads");

    std::ofstream head(".minigit/HEAD");
    head << "ref: refs/heads/main\n";

    return {true, "Initialized empty minigit repository in .minigit/"};
}

    Result cmd_hash_object(const std::string& filename, bool write) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return {false, "could not open file: " + filename};
    }

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    std::string content = contentStream.str();

    std::string header = "blob " + std::to_string(content.size());
    std::string store = header;
    store += '\0';
    store += content;

    std::string hash = sha1(store);

    if (write) {
        std::string dir = ".minigit/objects/" + hash.substr(0, 2);
        std::string objectPath = dir + "/" + hash.substr(2);

        if (!folderExists(dir)) {
            MKDIR(dir.c_str());
        }

        std::ofstream outFile(objectPath, std::ios::binary);
        outFile << store;
    }

    return {true, hash};
}




Result cmd_cat_file(const std::string& hash) {
    if (hash.size() < 3) {
        return {false, "hash too short"};
    }

    std::string objectPath = ".minigit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);

    std::ifstream file(objectPath, std::ios::binary);
    if (!file) {
        return {false, "object not found: " + hash};
    }

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    std::string stored = contentStream.str();

    size_t nullPos = stored.find('\0');
    if (nullPos == std::string::npos) {
        return {false, "corrupt object: no header separator found"};
    }

    std::string content = stored.substr(nullPos + 1);

    return {true, content};
}


// thats for add in the repositories

Result cmd_add(const std::string& filename) {
    Result hashResult = cmd_hash_object(filename, true);
    if (!hashResult.ok) {
        return hashResult;
    }
    std::string hash = hashResult.message;

    std::map<std::string, std::string> entries;

    std::ifstream indexIn(".minigit/index");
    std::string existingHash, existingName;
    while (indexIn >> existingHash >> existingName) {
        entries[existingName] = existingHash;
    }
    indexIn.close();

    entries[filename] = hash;

    std::ofstream indexOut(".minigit/index");
    for (const auto& entry : entries) {
        indexOut << entry.second << " " << entry.first << "\n";
    }

    return {true, "added " + filename};
}


// commit for adding
std::string storeObject(const std::string& type, const std::string& content) {
    std::string header = type + " " + std::to_string(content.size());
    std::string store = header;
    store += '\0';
    store += content;

    std::string hash = sha1(store);
    std::string dir = ".minigit/objects/" + hash.substr(0, 2);
    if (!folderExists(dir)) {
        MKDIR(dir.c_str());
    }
    std::ofstream outFile(dir + "/" + hash.substr(2), std::ios::binary);
    outFile << store;

    return hash;
}


//cmd commit
Result cmd_commit(const std::string& message) {
    std::ifstream indexIn(".minigit/index");
    if (!indexIn) {
        return {false, "nothing to commit (index is empty)"};
    }

    std::string hash, name;
    std::string treeContent;
    bool hasEntries = false;
    while (indexIn >> hash >> name) {
        treeContent += "blob " + hash + " " + name + "\n";
        hasEntries = true;
    }
    indexIn.close();

    if (!hasEntries) {
        return {false, "nothing to commit (index is empty)"};
    }

    std::string treeHash = storeObject("tree", treeContent);

    std::ifstream headIn(".minigit/HEAD");
    std::string headLine;
    std::getline(headIn, headLine);
    headIn.close();
    std::string refPath = ".minigit/" + headLine.substr(5);

    std::string parentHash;
    std::ifstream refIn(refPath);
    if (refIn) {
        std::getline(refIn, parentHash);
    }
    refIn.close();

    std::string commitContent = "tree " + treeHash + "\n";
    if (!parentHash.empty()) {
        commitContent += "parent " + parentHash + "\n";
    }
    commitContent += "\n" + message + "\n";

    std::string commitHash = storeObject("commit", commitContent);

    std::ofstream refOut(refPath);
    refOut << commitHash << "\n";
    refOut.close();

    return {true, "[" + commitHash.substr(0, 7) + "] " + message};
}
std::string readObject(const std::string& hash) {
    std::string objectPath = ".minigit/objects/" + hash.substr(0, 2) + "/" + hash.substr(2);
    std::ifstream file(objectPath, std::ios::binary);
    if (!file) return "";

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    std::string stored = contentStream.str();

    size_t nullPos = stored.find('\0');
    if (nullPos == std::string::npos) return "";

    return stored.substr(nullPos + 1);
}


Result cmd_log() {
    std::ifstream headIn(".minigit/HEAD");
    std::string headLine;
    std::getline(headIn, headLine);
    headIn.close();
    std::string refPath = ".minigit/" + headLine.substr(5);

    std::ifstream refIn(refPath);
    std::string currentHash;
    if (!refIn || !std::getline(refIn, currentHash) || currentHash.empty()) {
        return {false, "no commits yet"};
    }
    refIn.close();

    std::string output;

    while (!currentHash.empty()) {
        std::string commitContent = readObject(currentHash);
        if (commitContent.empty()) break;

        std::istringstream lines(commitContent);
        std::string line;
        std::string parentHash;
        std::string message;
        bool readingMessage = false;

        while (std::getline(lines, line)) {
            if (readingMessage) {
                message += line + "\n";
            } else if (line.rfind("parent ", 0) == 0) {
                parentHash = line.substr(7);
            } else if (line.empty()) {
                readingMessage = true;
            }
        }

        output += "commit " + currentHash + "\n";
        output += "    " + message + "\n";

        currentHash = parentHash;
    }

    return {true, output};
}