#include <string>
#include <vector>
#include <cstdint>
#include "tokenizer.h"

using ImbdData = std::pair<torch::Tensor, torch::Tensor>; // input_ids, attention_mask
using ImbdExample = torch::data::Example<ImbdData, torch::Tensor>; // data, label

