#include "imbdreader.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

ImbdReader::ImbdReader(const std::string& root_path){
    auto root = fs::path(root_path);
    auto neg_path = root / "neg";
    auto pos_path = root / "pos";
    if(fs::exists(neg_path) && fs::exists(pos_path)){
        auto neg = std::async   (
            std::launch::async, [&](){
            read_directory(neg_path.string(), neg_reviews_);
        }
    );
        auto pos = std::async   (
            std::launch::async, [&](){
            read_directory(pos_path.string(), pos_reviews_);
        }
    );
        neg.get();
        pos.get();
    }
    else{
        throw std::invalid_argument("Invalid path: " + root_path);
    }
}


void ImbdReader::read_directory(const std::string& path, Reviews& reviews) {
    for(auto& entry: fs::directory_iterator(path)){
        if(fs::is_regular_file(entry)){
            std::ifstream file(entry.path());
            if (file) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                reviews.push_back(buffer.str());
            }
        }
    }
}