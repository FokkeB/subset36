// Python module and example written by Pierretsc(https://github.com/Pierretsc), small updates by FokkeB

#include "python_api.h"

int verbose = VERB_QUIET;
   
extern "C" EXPORT   // export this function in the .dll / .so file    
int encode_telegram_line(const char* input_line,
                         unsigned int max_cpu,
                         bool calc_all,
                         char* out_buf,
                         size_t out_buf_size)
{
    // Basic argument validation
    if (!input_line || !out_buf || out_buf_size == 0)
        return -1;   // invalid parameters

    try {
        // 1) Parse the input line into a linked list of telegram objects
        telegram* t = parse_input_line(input_line);
        if (!t) {
            // empty line or comment-only line
            return 1;
        }

        // 2) Process / encode telegram(s)
        convert_telegrams_multithreaded(t, max_cpu, calc_all);

        // 3) Produce the output string
        // format = "hex" → default formatting style of the library
        std::string out =
            output_telegrams_to_string(t,
                                       "hex",     // format
                                       false,     // error_only
                                       false,     // include_header
                                       calc_all);

        // 4) Copy result to the output buffer supplied by Python
        if (out.size() + 1 > out_buf_size) {
            return -2; // output buffer is too small
        }

        memcpy(out_buf, out.c_str(), out.size() + 1);

        // 5) Free the linked list of telegrams
        telegram* p = t;
        while (p) {
            telegram* next = p->next;   // next pointer (assumed public)
            delete p;
            p = next;
        }

        return 0;   // success
    }
    catch (...) {
        return -99; // unexpected exception
    }
}