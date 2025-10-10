# Sentiment Analysis with BERT in C++

Classify IMDB movie reviews using a TorchScript BERT encoder from Hugging Face and a LibTorch-based C++ pipeline. Python is used **once** to export the pretrained encoder; the rest of the pipeline (tokenization, data loading, and fine‑tuning) runs natively in C++.

---

## Features
- Trace `bert-base-cased` to TorchScript for deployment without Python.
- Custom **WordPiece** tokenizer and attention‑mask builder implemented in C++.
- Multi‑threaded IMDB data loader reading positive/negative review splits.
- Configurable training loop with gradient clipping, AdamW, and live metrics.
- CMake build that links directly against **LibTorch** (CPU or CUDA).

---

## Repository Structure

```text
Sentiment-Analysis-BERT/
├── CMakeLists.txt
├── main.cpp                 # Entry point / training loop
├── model.{h,cpp}            # Classifier head + TorchScript wrapper
├── tokenizer.{h,cpp}        # WordPiece tokenizer implementation
├── imdbreader.{h,cpp}       # Review loader (std::filesystem + async)
├── imdbdataset.{h,cpp}      # LibTorch dataset adapter
├── export.py                # TorchScript export + vocab dump
├── vocab.txt                # Token → id map aligned with traced model
├── bert_model.pt            # Traced BERT encoder (generated)
├── dataset/
│   └── aclImdb/...          # IMDB reviews (pos/neg, train/test)
├── IMDB Dataset.csv         # Optional CSV copy of reviews
└── build/                   # CMake build artifacts
```

---

## Prerequisites

### Python (one‑time export)
- Python 3.8+
- `torch`, `transformers`, `tqdm`
- Internet access (once) to download Hugging Face weights (or use cached files).

Install example:

```bash
pip install torch torchvision torchaudio transformers tqdm
```

### C++ build environment
- **CMake** ≥ 3.22
- **Clang** or **GCC** with C++17 support
- **LibTorch** (matching your compiler and CUDA/CPU choice). Extract it inside the repo or point `Torch_DIR`/`CMAKE_PREFIX_PATH` to your installation.

---

## 1) Export TorchScript model & vocab

Run once to generate `bert_model.pt` and `vocab.txt`:

```bash
python export.py
```

Optional helper (inside `export.py`) to force vocab regeneration:

```python
from transformers import BertTokenizer
tok = BertTokenizer.from_pretrained("bert-base-cased")
tok.save_vocabulary(".")  # writes vocab.txt
```

> ⚠️ If you **re‑trace** with a different tokenizer or sequence length, regenerate **both** files so the numeric IDs stay in sync.

---

## 2) Prepare the IMDB dataset

This project expects the original **ACL IMDB** dataset layout:

```text
dataset/
└── aclImdb/
    ├── train/
    │   ├── pos/*.txt
    │   └── neg/*.txt
    └── test/
        ├── pos/*.txt
        └── neg/*.txt
```

Download the archive from Stanford/IMDB or Kaggle, unzip, and place it under `dataset/aclImdb`.

---

## 3) Build the C++ project

Create a build directory and invoke CMake (update `Torch_DIR` or `CMAKE_PREFIX_PATH` if LibTorch lives elsewhere):

```bash
mkdir -p build
cmake -S . -B build   -DCMAKE_PREFIX_PATH="/path/to/libtorch/share/cmake/Torch"
cmake --build build --config Release
```

The executable is named `sentiment-pytorch` by default (based on the `project()` name in `CMakeLists.txt`).

---

## 4) Run training / inference

```bash
./build/sentiment-pytorch   dataset/aclImdb   vocab.txt   bert_model.pt
```

**Arguments (in order):**
1. Root of the IMDB dataset (must contain `train/pos` and `train/neg`).
2. WordPiece vocab file generated during export.
3. TorchScript BERT model.

If a CUDA build is present, the program will use GPU automatically when `torch::cuda::is_available()` returns true.

---

## Troubleshooting

- **`index out of range` inside TorchScript embedding**  
  Token IDs no longer match the traced model. Regenerate **both** `bert_model.pt` and `vocab.txt` with the **same** tokenizer and max length (the default in `export.py` is 128 tokens).

- **CMake cannot find Torch**  
  Confirm `Torch_DIR` or `CMAKE_PREFIX_PATH` points to the `share/cmake/Torch` directory inside your LibTorch install.

- **Slow dataset loading**  
  `ImdbReader` already parallelizes pos/neg splits; ensure the dataset path is correct and stored on fast storage (SSD recommended).

- **Out‑of‑memory on GPU**  
  Lower `batch_size` in `main.cpp`, reduce sequence length at export, or switch to CPU build.

---

## Extending the Project
- Swap in `bert-base-uncased` or a **distilled** model by editing `export.py`.
- Add validation/test loaders using the existing dataset abstractions.
- Implement a prediction CLI that reads raw text and prints sentiment scores.
- Try dynamic quantization of the classifier head for faster CPU inference.

---

## Acknowledgements
- **Hugging Face** Transformers
- **LibTorch** (PyTorch C++ API)
- **IMDB** Movie Review Dataset
