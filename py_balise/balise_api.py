# Python module and example written by Pierretsc (https://github.com/Pierretsc), small updates by FokkeB

import os
import ctypes
from ctypes import c_char_p, c_uint, c_bool, c_size_t, c_int, create_string_buffer

# Path to the compiled shared library
# Adjust depending on your project layout (e.g. ../build/libbalise_codec.so)
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# on Win-x64:
LIB_PATH = os.path.join(BASE_DIR, "..", "out", "build", "WIN-x64-Release","py_balise_codec","py_balise_codec.dll")

# on Linux-ARM:
#LIB_PATH = os.path.join(BASE_DIR, "..","..","out","build","Linux-GCC-Release-ARM","py_balise_codec","libpy_balise_codec.so")

# on Linux-x64:
#LIB_PATH = os.path.join(BASE_DIR, "..","..","out","build","Linux-GCC-Release-x64","py_balise_codec","libpy_balise_codec.so")

print ("Loading module ",LIB_PATH)
lib = ctypes.CDLL(LIB_PATH)
print ("Loading module complete")

# C function prototype:
# int encode_telegram_line(const char* input_line,
#                          unsigned int max_cpu,
#                          bool calc_all,
#                          char* out_buf,
#                          size_t out_buf_size);

lib.encode_telegram_line.argtypes = [
    c_char_p,    # input_line
    c_uint,      # max_cpu
    c_bool,      # calc_all
    c_char_p,    # out_buf
    c_size_t,    # out_buf_size
]
lib.encode_telegram_line.restype = c_int


def encode_telegram(input_line: str,
                    max_cpu: int = 0,
                    calc_all: bool = False,
                    buf_size: int = 40960) -> str:
    """
    Calls the C++ library to encode a single telegram line.

    :param input_line: The raw telegram line (same format as in the original input files)
    :param max_cpu: 0 = auto, otherwise CPU/thread limit (ignored in the mono-threaded version but kept for compatibility)
    :param calc_all: True = generate all variations, False = generate a single result
    :param buf_size: Maximum size of the output buffer
    :return: Encoded telegram string (format defined by output_telegrams_to_string)
    """
    if not isinstance(input_line, str):
        raise TypeError("input_line must be a string")

    buf = create_string_buffer(buf_size)

    rc = lib.encode_telegram_line(
        input_line.encode("ascii"),
        max_cpu,
        calc_all,
        buf,
        buf_size,
    )

    if rc == 0:
        return decode_encoded_output(buf.value.decode("ascii"))

    if rc == 1:
        raise ValueError("Input line is empty or comment-only")
    if rc == -2:
        raise ValueError("Output buffer too small (increase buf_size)")
    if rc == -1:
        raise ValueError("Invalid parameters (input_line or buffer)")

    raise RuntimeError(f"encode_telegram_line failed with error code {rc}")


def decode_encoded_output(encoded: str):
    """
    Split the encoded telegram output into separate fields.

    Example input:
        "AAAABBBBCCC;DDDDEEEEFFFF;0"

    Returns:
        (first, second, status)
    """
    parts = encoded.strip().split(";")

    if len(parts) != 3:
        raise ValueError(f"Unexpected encoded format: {encoded}")

    telegram = parts[0]
    telegram_encoded = parts[1]
    status = int(parts[2])

    return  telegram_encoded,telegram, status