#include "balise_codec.h"

#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


extern "C"
{

    EXPORT int encode_telegram_line(const char* input_line,
        unsigned int max_cpu,
        bool calc_all,
        char* out_buf,
        size_t out_buf_size);

    // tbd: add get_version
}