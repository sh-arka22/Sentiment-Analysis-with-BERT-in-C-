// #include "imbddataset.h"
// #include <cassert>
// #include <fstream>

// ImbdDataset::ImbdDataset(const std::string& dataset_path,
//                          std::shared_ptr<Tokenizer> tokenizer)
//     : reader_(dataset_path), tokenizer_(std::move(tokenizer)) {}

// ImbdExample ImbdDataset::get(size_t index) {
//   torch::Tensor target;
//   const std::string* review{nullptr};
//   if (index < reader_.get_pos_size()) {
//     review = &reader_.get_pos(index);
//     target = torch::tensor(1, torch::dtype(torch::kLong));
//   } else {
//     review = &reader_.get_neg(index - reader_.get_pos_size());
//     target = torch::tensor(0, torch::dtype(torch::kLong));
//   }
//   // encode text
//   auto tokenizer_out = tokenizer_->tokenize(*review);

//   // Validate token IDs
//   for (const auto& id : tokenizer_out) {
//     if (id < 0 || id >= 30522) {
//       throw std::runtime_error("Token ID out of range: " + std::to_string(id));
//     }
//   }
  
//   // Convert vector to tensor
//   auto input_ids = torch::tensor(tokenizer_out, torch::dtype(torch::kLong));
  
//   // Create attention mask (1 for all tokens, assuming no padding or handle padding separately)
//   auto attention_mask = torch::ones_like(input_ids);
  
//   // Return as pair of tensors
//   return {std::make_pair(input_ids, attention_mask), target.squeeze()};
// }

// torch::optional<size_t> ImbdDataset::size() const {
//   return reader_.get_pos_size() + reader_.get_neg_size();
// }



#include "imdbdataset.h"
#include <cassert>
#include <fstream>

ImdbDataset::ImdbDataset(const std::string& dataset_path,
                         std::shared_ptr<Tokenizer> tokenizer)
    : reader_(dataset_path), tokenizer_(std::move(tokenizer)) {}

ImdbExample ImdbDataset::get(size_t index) {
  torch::Tensor target;
  const std::string* review{nullptr};
  if (index < reader_.get_pos_size()) {
    review = &reader_.get_pos(index);
    target = torch::tensor(1, torch::dtype(torch::kLong));
  } else {
    review = &reader_.get_neg(index - reader_.get_pos_size());
    target = torch::tensor(0, torch::dtype(torch::kLong));
  }
  // encode text
  // encode text
  auto tokenizer_out = tokenizer_->tokenize(*review);            // std::vector<int64_t>

  // Convert vector -> tensor (int64)
  auto input_ids = torch::tensor(tokenizer_out, torch::kLong);

  // Attention mask: 1 for non-PAD (assuming PAD id == 0), else 0
  auto attention_mask = input_ids.ne(0).to(torch::kLong);

  // Return Example<{ids, mask}, label>
  return { std::make_pair(input_ids, attention_mask), target.squeeze() };
}

torch::optional<size_t> ImdbDataset::size() const {
  return reader_.get_pos_size() + reader_.get_neg_size();
}

