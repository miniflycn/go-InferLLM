## Usage

Note: This repository uses git submodules to keep track of [InferLLM](https://github.com/MegEngine/InferLLM).

Clone the repository locally:

```bash
git clone --recurse-submodules https://github.com/miniflycn/InferLLM-go
```

To build the bindings locally, run:

```
cd InferLLM-go
make libbinding.a
```

Download chatglm model：https://huggingface.co/kewin4933/InferLLM-Model/blob/main/chatglm-q4.bin

Now you can run the example with:

```
LIBRARY_PATH=$PWD C_INCLUDE_PATH=$PWD go run ./examples/run.go
```
