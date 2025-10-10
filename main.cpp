// #include "imbddataset.h"
// #include "model.h"

// #include <filesystem>
// #include <iostream>
// #include <torch/torch.h>

// namespace fs = std::filesystem;

// struct Stack : public torch::data::transforms::Collation<ImbdExample> {
//   ImbdExample apply_batch(std::vector<ImbdExample> examples) override {
//     std::vector<torch::Tensor> input_ids;
//     std::vector<torch::Tensor> attention_masks;
//     std::vector<torch::Tensor> labels;
//     input_ids.reserve(examples.size());
//     attention_masks.reserve(examples.size());
//     labels.reserve(examples.size());
//     for (auto& example : examples) {
//       input_ids.push_back(std::move(example.data.first));
//       attention_masks.push_back(std::move(example.data.second));
//       labels.push_back(std::move(example.target));
//     }
//     return {{torch::stack(input_ids), torch::stack(attention_masks)}, torch::stack(labels)};
//   }
// };

// int main(int argc, char** argv) {
//   if (argc == 4) {
//     auto dataset_path = fs::path(argv[1]);
//     if (!fs::exists(dataset_path)) {
//       std::cerr << "Incorrect dataset path!\n";
//       return 1;
//     }
//     auto vocab_path = fs::path(argv[2]);
//     if (!fs::exists(vocab_path)) {
//       std::cerr << "Incorrect vocabulary path!\n";
//       return 1;
//     }
//     auto model_path = fs::path(argv[3]);
//     if (!fs::exists(model_path)) {
//       std::cerr << "Incorrect model path!\n";
//       return 1;
//     }

//     torch::DeviceType device = torch::cuda::is_available()
//                                    ? torch::DeviceType::CUDA
//                                    : torch::DeviceType::CPU;

//     std::cout << "Using device: " << (device == torch::DeviceType::CUDA ? "CUDA" : "CPU") << std::endl;

//     std::cout << "Loading tokenizer..." << std::endl;
//     auto tokenizer = std::make_shared<Tokenizer>(vocab_path, 512);
//     std::cout << "Tokenizer loaded successfully." << std::endl;

//     std::cout << "Loading dataset from: " << (dataset_path / "train").string() << std::endl;
//     ImbdDataset train_dataset(dataset_path / "train", tokenizer);
//     std::cout << "Dataset loaded. Size: " << train_dataset.size().value() << std::endl;

//     int batch_size = 8;
//     std::cout << "Creating data loader with batch_size=" << batch_size << std::endl;
//     auto train_loader = torch::data::make_data_loader(
//         train_dataset.map(Stack()),
//         torch::data::DataLoaderOptions().batch_size(batch_size).workers(8));

//     std::cout << "Loading BERT model from: " << model_path.string() << std::endl;
//     Model model(model_path);
//     model->to(device);
//     std::cout << "Model loaded and moved to device." << std::endl;

//     torch::optim::AdamW optimizer(model->parameters(),
//                                   torch::optim::AdamWOptions(1e-5));

//     int epochs = 100;
//     std::cout << "\nStarting training for " << epochs << " epochs..." << std::endl;

//     for (int epoch = 0; epoch < epochs; ++epoch) {
//       model->train();

//       float epoch_loss = 0.0;
//       float epoch_correct = 0.0;
//       int total_samples = 0;
//       int batch_index = 0;
//       for (auto& batch : (*train_loader)) {
//         optimizer.zero_grad();

//         auto batch_label = batch.target.to(device);
//         auto batch_input_ids = batch.data.first.squeeze(1).to(device);
//         auto batch_attention_mask = batch.data.second.squeeze(1).to(device);

//         auto output = model(batch_input_ids, batch_attention_mask);

//         torch::Tensor loss =
//             torch::nn::functional::cross_entropy(output, batch_label);

//         loss.backward();
//         torch::nn::utils::clip_grad_norm_(model->parameters(), 1.0);
//         optimizer.step();

//         // Calculate batch statistics
//         auto predictions = output.argmax(/*dim=*/1);
//         auto correct = (predictions == batch_label).sum().item<float>();
//         epoch_correct += correct;
//         epoch_loss += loss.item<float>() * batch_label.size(0);
//         total_samples += batch_label.size(0);
//         std::cout << "Epoch [" << (epoch + 1) << "/" << epochs << "], Batch [" << (++batch_index) << "], Loss: " << loss.item<float>() << ", Batch Accuracy: " << (correct / batch_label.size(0)) << std::endl;
//       }

//       // Calculate epoch statistics
//       float epoch_accuracy = epoch_correct / total_samples;
//       float average_loss = epoch_loss / total_samples;

//       // Print epoch progress
//       std::cout << "Epoch: " << (epoch + 1)
//                 << " | Loss: " << average_loss
//                 << " | Acc: " << epoch_accuracy << std::endl;
//     }
//     return 0;
//   }

//   std::cerr << "Please specify dataset folder, vocabulary file path, and model file path\n";
//   return 1;
// }

#include "imdbdataset.h"
#include "model.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

struct Stack : public torch::data::transforms::Collation<ImdbExample>
{
  ImdbExample apply_batch(std::vector<ImdbExample> examples) override
  {
    std::vector<torch::Tensor> input_ids;
    std::vector<torch::Tensor> attention_masks;
    std::vector<torch::Tensor> labels;
    input_ids.reserve(examples.size());
    attention_masks.reserve(examples.size());
    labels.reserve(examples.size());
    for (auto &example : examples)
    {
      input_ids.push_back(std::move(example.data.first));
      attention_masks.push_back(std::move(example.data.second));
      labels.push_back(std::move(example.target));
    }
    return {{torch::stack(input_ids), torch::stack(attention_masks)}, torch::stack(labels)};
  }
};

int main(int argc, char **argv)
{
  if (argc == 4)
  {
    auto dataset_path = fs::path(argv[1]);
    if (!fs::exists(dataset_path))
    {
      std::cerr << "Incorrect dataset path!\n";
    }
    auto vocab_path = fs::path(argv[2]);
    if (!fs::exists(vocab_path))
    {
      std::cerr << "Incorrect vocabulary path!\n";
    }
    auto model_path = fs::path(argv[3]);
    if (!fs::exists(model_path))
    {
      std::cerr << "Incorrect model path!\n";
    }

    torch::DeviceType device = torch::cuda::is_available()
                                   ? torch::DeviceType::CUDA
                                   : torch::DeviceType::CPU;

    auto tokenizer = std::make_shared<Tokenizer>(vocab_path, 128);
    ImdbDataset train_dataset(dataset_path / "train", tokenizer);

    int batch_size = 8;
    auto train_loader = torch::data::make_data_loader(
        train_dataset.map(Stack()),
        torch::data::DataLoaderOptions().batch_size(batch_size).workers(8)); // random sampler is default

    Model model(model_path);
    model->to(device);

    torch::optim::AdamW optimizer(model->parameters(),
                                  torch::optim::AdamWOptions(1e-5));

    // -------------------- REPLACE FROM HERE --------------------
    int epochs = 100;

    // Compute steps per epoch once (ceil division).
    const auto dataset_size = train_dataset.size().value();
    const auto steps_per_epoch = (dataset_size + batch_size - 1) / batch_size;

    std::cout << "Steps per epoch: " << steps_per_epoch
              << " (dataset_size=" << dataset_size
              << ", batch_size=" << batch_size << ")\n";

    for (int epoch = 0; epoch < epochs; ++epoch)
    {
      std::cout << "\n=== START EPOCH " << (epoch + 1) << "/" << epochs
                << " ===" << std::endl;

      model->train();

      size_t batch_index = 0;
      int64_t running_loss = 0.0;
      int64_t running_correct = 0;
      int64_t running_seen = 0;

      for (auto &batch : (*train_loader))
      {
        optimizer.zero_grad();

        auto batch_label = batch.target.to(device);
        auto batch_input_ids = batch.data.first.squeeze(1).to(device);
        auto batch_attention_mask = batch.data.second.squeeze(1).to(device);

        // Optional debug: token ID range per batch
        // auto min_id = batch_input_ids.min().item<int64_t>();
        // auto max_id = batch_input_ids.max().item<int64_t>();
        // std::cout << "Token ID range: " << min_id << " .. " << max_id << std::endl;

        auto output = model(batch_input_ids, batch_attention_mask);
        torch::Tensor loss = torch::cross_entropy_loss(output, batch_label);

        loss.backward();
        torch::nn::utils::clip_grad_norm_(model->parameters(), 1.0);
        optimizer.step();

        // Metrics
        auto predictions = output.argmax(/*dim=*/1);
        running_loss += loss.item<double>();
        running_correct += (predictions == batch_label).sum().item<int64_t>();
        running_seen += batch_label.size(0);

        ++batch_index;

        // Periodic logging
        if (batch_index % 10 == 0 || batch_index == steps_per_epoch) //batch size is 10 not 8 sorry for that !!
        {
          double avg_loss = running_loss / static_cast<double>(batch_index);
          double avg_acc = static_cast<double>(running_correct) / static_cast<double>(running_seen);
          std::cout << "Epoch " << (epoch + 1) << "/" << epochs
                    << " | Step " << batch_index << "/" << steps_per_epoch
                    << " | Loss " << avg_loss
                    << " | Acc " << avg_acc
                    << std::endl;
        }

        // If the DataLoader yields more than steps_per_epoch due to rounding, stop at epoch boundary.
        if (batch_index >= steps_per_epoch)
          break;
      }

      // End-of-epoch summary
      double epoch_loss = running_loss / static_cast<double>(batch_index > 0 ? batch_index : 1);
      double epoch_acc = static_cast<double>(running_correct) / static_cast<double>(running_seen > 0 ? running_seen : 1);

      std::cout << ">>> END EPOCH " << (epoch + 1) << "/" << epochs
                << " | Loss " << epoch_loss
                << " | Acc " << epoch_acc
                << std::endl;
    }

    return 0;
    // -------------------- REPLACE UNTIL HERE --------------------
  }

  std::cerr << "Please specify dataset folder, vocablary file path, and model file path\n";
  return 1;
}
