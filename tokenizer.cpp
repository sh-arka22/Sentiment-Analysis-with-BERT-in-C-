

#include "tokenizer.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace {
    inline std::string trim(const std::string& s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) return "";
        size_t e = s.find_last_not_of(" \t\r\n");
        return s.substr(b, e - b + 1);
    }
}

Tokenizer::Tokenizer(const std::string& vocab_file_path, int max_length)
: max_len_(max_length) {
    load_vocab(vocab_file_path);

    auto need = [&](const char* tok) -> int {
        auto it = vocab_.find(tok);
        if (it == vocab_.end()) throw std::runtime_error(std::string("Token missing in vocab: ") + tok);
        return it->second;
    };

    pad_id_ = need("[PAD]");
    cls_id_ = need("[CLS]");
    sep_id_ = need("[SEP]");
    auto unk_it = vocab_.find("[UNK]");
    if (unk_it == vocab_.end()) throw std::runtime_error("Token missing in vocab: [UNK]");
    unk_id_ = unk_it->second;
}

void Tokenizer::load_vocab(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Failed to open vocab file: " + path);

    std::string line;
    int next_id = 0; // for token-only lines
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token; int id = -1;
        ss >> token;
        if (!(ss >> id)) {
            id = next_id++; // assign incrementally if no id given
        } else if (id >= next_id) {
            next_id = id + 1; // keep in sync with explicit ids
        }
        vocab_[token] = id;
    }
}

std::vector<std::string> Tokenizer::basic_tokenize(const std::string& text) const {
    std::vector<std::string> out; out.reserve(text.size()/2 + 8);
    std::string cur;
    auto flush = [&](){ if (!cur.empty()) { out.push_back(cur); cur.clear(); } };

    for (unsigned char ch : text) {
        char c = static_cast<char>(ch);
        if (do_lower_case_) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (std::isspace(static_cast<unsigned char>(c))) {
            flush();
        } else if (!std::isalnum(static_cast<unsigned char>(c))) {
            flush();
            out.emplace_back(1, c); // punctuation as separate token
        } else {
            cur.push_back(c);
        }
    }
    flush();
    return out;
}

void Tokenizer::wordpiece_tokenize(const std::string& word, std::vector<int>& out_ids) const {
    size_t start = 0; const size_t L = word.size();
    while (start < L) {
        size_t end = L; bool matched = false;
        while (end > start) {
            std::string sub = word.substr(start, end - start);
            if (start > 0) sub = "##" + sub; // continuation piece
            auto it = vocab_.find(sub);
            if (it != vocab_.end()) {
                out_ids.push_back(it->second);
                start = end; matched = true; break;
            }
            --end;
        }
        if (!matched) { out_ids.push_back(unk_id_); break; }
    }
}

std::vector<int64_t> Tokenizer::tokenize(const std::string& text) {
    std::vector<int64_t> ids(static_cast<size_t>(max_len_), static_cast<int64_t>(pad_id_));

    int cur = 0;
    // [CLS]
    ids[cur++] = cls_id_;

    // tokens (keep room for [SEP])
    for (const auto& w : basic_tokenize(text)) {
        if (cur >= max_len_ - 1) break;
        std::vector<int> pieces; pieces.reserve(4);
        wordpiece_tokenize(w, pieces);
        for (int pid : pieces) {
            if (cur >= max_len_ - 1) break;
            ids[cur++] = static_cast<int64_t>(pid);
        }
    }

    // [SEP]
    if (cur < max_len_) ids[cur++] = sep_id_;

    // remaining are already [PAD]
    return ids;
}