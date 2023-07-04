#include "binding.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <iostream>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <signal.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <signal.h>
#include <windows.h>
#endif

#include "InferLLM/include/model.h"

// int count = 0;
static std::map<void*, std::shared_ptr<inferllm::Model>> model_map;

struct app_params {
    int32_t seed = -1;  // RNG seed
    int32_t n_threads = std::min(4, (int32_t)std::thread::hardware_concurrency());
    int32_t n_predict = 128;     // new tokens to predict
    int32_t repeat_last_n = 64;  // last n tokens to penalize
    int32_t n_ctx = 2048;        // context size

    // sampling parameters
    int32_t top_k = 40;
    float top_p = 0.95f;
    float temp = 0.10f;
    float repeat_penalty = 1.30f;

    std::string model;  // model path

    bool use_color = true;          // use color to distinguish generations and inputs
    bool use_mmap = false;          // use mmap to load model
    std::string dtype = "float32";  // configure the compute dtype
    std::string mtype = "chatglm";  // the model type name, llama
};

std::shared_ptr<inferllm::Model> find_model(void *state_ptr) {
    std::map<void *, std::shared_ptr<inferllm::Model>>::iterator it = model_map.find(state_ptr);
    if (it != model_map.end()) {
        return it->second;
    } else {
        throw "没有找到该模型\n";
    }
}

void llm_binding_free_model(void *state_ptr) {
    std::map<void *, std::shared_ptr<inferllm::Model>>::iterator it = model_map.find(state_ptr);
    if (it != model_map.end()) {
        // fprintf(stderr, "成功清理指针\n");
        model_map.erase(it);
    } else {
        fprintf(stderr, "清理指针失败\n");
    }
}

auto fix_word = [](std::string& word) {
    auto ret = word;
    if (word == "<n>" || word == "<n><n>")
        word = "\n";
    if (word == "<|tab|>")
        word = "\t";
    int pos = word.find("<|blank_");
    if (pos != -1) {
        int space_num = atoi(word.substr(8, word.size() - 10).c_str());
        word = std::string(space_num, ' ');
    }
    pos = word.find("▁");
    if (pos != -1) {
        word.replace(pos, pos + 3, " ");
    }
    // Fix utf-8 garbled characters
    if (word.length() == 6 && word[0] == '<' && word[word.length() - 1] == '>' &&
        word[1] == '0' && word[2] == 'x') {
        int num = std::stoi(word.substr(3, 2), nullptr, 16);
        word = static_cast<char>(num);
    }
};

char *ask_sync(void *state_ptr, const char *user_input) {
    std::shared_ptr<inferllm::Model> model = find_model(state_ptr);
    std::string output;
    while (model->get_remain_token() > 0) {
        if (output.empty()) {
            int token;
            output = model->decode(user_input, token);
            fix_word(output);
        } else {
            int token;
            auto o = model->decode_iter(token);
            fix_word(o);
            output += o;
            if (token == 130005) {
                printf("\n");
                break;
                // running_summary = model->decode_summary();
            }
        }
    }
    char *c = (char*) malloc(output.length());
    strcpy(c, output.c_str());
    return c;
}

char *ask(void *state_ptr, const char *user_input) {
    std::shared_ptr<inferllm::Model> model = find_model(state_ptr);
    std::string output;
    if (model->get_remain_token() > 0) {
        int token;
        output = model->decode(user_input, token);
        fix_word(output);
    }
    char *c = (char*) malloc(output.length());
    strcpy(c, output.c_str());
    return c;
}

char *get_continue(void *state_ptr) {
    std::shared_ptr<inferllm::Model> model = find_model(state_ptr);
    std::string output;
    if (model->get_remain_token() > 0) {
        int token;
        auto o = model->decode_iter(token);
        fix_word(o);
        output += o;
        if (token == 130005) {
            output += "\n\u200b";
            // running_summary = model->decode_summary();
        }
    }
    char *c = (char*) malloc(output.length());
    strcpy(c, output.c_str());
    return c;
}


void* load_model(const char *fname) {
    app_params params;
    params.model = fname;

    if (params.seed < 0) {
        params.seed = time(NULL);
    }

    std::mt19937 rng(params.seed);

    inferllm::ModelConfig config;
    config.compt_type = params.dtype;
    config.nr_thread = params.n_threads;
    config.enable_mmap = params.use_mmap;
    config.nr_ctx = params.n_ctx;

    fprintf(stderr, "threads === %d\n", config.nr_thread);

    void* res = nullptr;
    // std::shared_ptr<inferllm::Model> model;

    try {
        std::shared_ptr<inferllm::Model> model =
            std::make_shared<inferllm::Model>(config, params.mtype);
        model->load(params.model);
        model->init(
                params.top_k, params.top_p, params.temp, params.repeat_penalty,
                params.repeat_last_n, params.seed, 130005);
        res = model.get();
        // save
        model_map.insert(std::pair<void*, std::shared_ptr<inferllm::Model>>(res, model));
    } catch (std::runtime_error& e) {
        fprintf(stderr, "failed %s",e.what());
        return res;
    }

    return res;
}