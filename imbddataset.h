#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "tokenizer.h"
#include "imbdreader.h"


using ImbdData = std::pair<torch::Tensor, torch::Tensor>; // input_ids, attention_mask
using ImbdExample = torch::data::Example<ImbdData, torch::Tensor>; // data, label


class ImbdDataset : public torch::data::Dataset<ImbdDataset, ImbdExample> {
public:
    ImbdDataset (const std::string& dataset_path, std::shared_ptr<Tokenizer> tokenizer);
    // torch::data::Dataset implementations
    ImbdExample get(size_t index) override;
    torch::optional<size_t> size() const override;

private:
    ImbdReader reader_;
    std::shared_ptr<Tokenizer> tokenizer_;
};