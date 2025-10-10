// #include "imbdreader.h"
// #include <filesystem>
// #include <fstream>
// #include <sstream>
// #include <stdexcept>
// #include <future>
// #include <iostream>

// namespace fs = std::filesystem;

// ImbdReader::ImbdReader(const std::string& root_path){
//     auto root = fs::path(root_path);
//     auto neg_path = root / "neg";
//     auto pos_path = root / "pos";
//     if(fs::exists(neg_path) && fs::exists(pos_path)){
//         auto neg = std::async(
//             std::launch::async, [this, neg_path](){
//                 read_directory(neg_path.string(), neg_reviews_);
//             }
//         );
//         auto pos = std::async(
//             std::launch::async, [this, pos_path](){
//                 read_directory(pos_path.string(), pos_reviews_);
//             }
//         );
//         neg.get();
//         pos.get();
//     }
//     else{
//         throw std::invalid_argument("Invalid path: " + root_path);
//     }
// }

// const std::string& ImbdReader::get_pos(size_t index) const {
//     return pos_reviews_.at(index);
//   }
  
//   const std::string& ImbdReader::get_neg(size_t index) const {
//     return neg_reviews_.at(index);
//   }
  
//   size_t ImbdReader::get_pos_size() const {
//     return pos_reviews_.size();
//   }
  
//   size_t ImbdReader::get_neg_size() const {
//     return neg_reviews_.size();
//   }

// void ImbdReader::read_directory(const std::string& path, Reviews& reviews) {
//     size_t count = 0;
//     for(auto& entry: fs::directory_iterator(path)){
//         if(fs::is_regular_file(entry)){
//             std::ifstream file(entry.path());
//             if (file) {
//                 std::stringstream buffer;
//                 buffer << file.rdbuf();
//                 reviews.push_back(buffer.str());
//                 if (++count % 1000 == 0) {
//                     std::cout << "  Loaded " << count << " reviews from " << path << std::endl;
//                 }
//             }
//         }
//     }
//     std::cout << "  Finished loading " << count << " reviews from " << path << std::endl;
// }


#include "imdbreader.h"

#include <filesystem>
#include <fstream>
#include <future>
#include <regex>

namespace fs = std::filesystem;

ImdbReader::ImdbReader(const std::string& root_path) {
  auto root = fs::path(root_path);
  auto neg_path = root / "neg";
  auto pos_path = root / "pos";
  if (fs::exists(neg_path) && fs::exists(pos_path)) {
    auto neg = std::async(std::launch::async,
                          [&]() { read_directory(neg_path, neg_samples_); });
    auto pos = std::async(std::launch::async,
                          [&]() { read_directory(pos_path, pos_samples_); });
    neg.get();
    pos.get();
  } else {
    throw std::invalid_argument("ImdbReader incorrect path");
  }
}

const std::string& ImdbReader::get_pos(size_t index) const {
  return pos_samples_.at(index);
}

const std::string& ImdbReader::get_neg(size_t index) const {
  return neg_samples_.at(index);
}

size_t ImdbReader::get_pos_size() const {
  return pos_samples_.size();
}

size_t ImdbReader::get_neg_size() const {
  return neg_samples_.size();
}

void ImdbReader::read_directory(const std::string& path, Reviews& reviews) {
  for (auto& entry : fs::directory_iterator(path)) {
    if (fs::is_regular_file(entry)) {
      std::ifstream file(entry.path());
      if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        reviews.push_back(buffer.str());
      }
    }
  }
}
