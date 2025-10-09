#pragma once
#include <torch/torch.h>
#include <string>
#include <vector>
#include <unordered_map>

class Tokenizer {
public:
    // Keep VS Code signatures exactly
    Tokenizer(const std::string& vocab_file_path, int max_length);
    std::vector<int64_t> tokenize(const std::string& text);

private:
    std::unordered_map<std::string, int> vocab_;
    int max_len_{0};
    int pad_id_{0}, cls_id_{0}, sep_id_{0}, unk_id_{0};
    bool do_lower_case_{true};

    void load_vocab(const std::string& path);
    std::vector<std::string> basic_tokenize(const std::string& text) const;
    void wordpiece_tokenize(const std::string& word, std::vector<int>& out_ids) const;
};