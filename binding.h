#ifdef __cplusplus
#include <vector>
#include <string>
extern "C" {
#endif

#include <stdbool.h>
void* load_model(const char *fname);
#ifdef __cplusplus
}

std::vector<std::string> create_vector(const char** strings, int count);
void delete_vector(std::vector<std::string>* vec);
#endif