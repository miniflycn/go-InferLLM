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
#include<iostream>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <signal.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <signal.h>
#include <windows.h>
#endif

#include "InferLLM/include/model.h"

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

void llm_binding_free_model(void *state_ptr) {
    fprintf(stderr, "state_ptr= == %d\n", state_ptr);
    // inferllm::Model* __model = reinterpret_cast<inferllm::Model*>(state_ptr);
    // std::shared_ptr<inferllm::Model> model;
    // model.reset(__model);
    // fprintf(stderr, "model= == %c\n", model);
    // int a = model->get_remain_token();
    // fprintf(stderr, "a === %d\n", a);
    // try {
    //     inferllm::Model* model = reinterpret_cast<inferllm::Model*>(state_ptr);
    //     int a = model->get_remain_token();
    // } catch(std::runtime_error& e) {
    //     fprintf(stderr, "failed %s",e.what());
    // }

    // inferllm::Model* model = (inferllm::Model*) state_ptr;
    // try {
    //     fprintf(stderr, "ptr == %s\n", state_ptr);
    //     fprintf(stderr, "ctx == %s\n", model);
    //     model->get_remain_token();
    // } catch (std::runtime_error& e) {
    //     fprintf(stderr, "failed %s",e.what());
    // }
    // printf("{}", ctx);
    // delete &ctx;
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

    void* res = nullptr;

    try {
        std::shared_ptr<inferllm::Model> model =
            std::make_shared<inferllm::Model>(config, params.mtype);
        model->load(params.model);
        model->init(
                params.top_k, params.top_p, params.temp, params.repeat_penalty,
                params.repeat_last_n, params.seed, 130005);
        res = model.get();
    } catch (std::runtime_error& e) {
        fprintf(stderr, "failed %s",e.what());
        return res;
    }

    // 打印model

    inferllm::Model *m = (inferllm::Model*) res;
    m->get_remain_token();

    fprintf(stderr, "res= == %d\n", res);

    return res;
}