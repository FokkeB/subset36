#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <Windows.h>
#include "balise_codec.h"
#include <stdio.h>

int verbose = VERB_QUIET;

static PyObject* convert(PyObject* self, PyObject* args)
{
    const char* orig_balise_info = NULL;
    t_telegramlist telegrams;
    int result = 0;
    string output_string;

    if ( (!PyArg_ParseTuple(args, "s", &orig_balise_info)) || (orig_balise_info == NULL) )
        return NULL;

    eprintf(VERB_GLOB, "Received from Python:\n %s\n", orig_balise_info);

    telegrams = parse_content_string((string)orig_balise_info);
    result = convert_telegrams_multithreaded(telegrams, 1);
    output_string = output_telegrams_to_string(telegrams, "base64", false, false);

    eprintf(VERB_GLOB, "Returning to Python:\n %s\n", output_string.c_str());

    return PyUnicode_FromString(output_string.c_str());
}

static PyMethodDef balise_codec_methods[] = {
    { "convert", (PyCFunction)convert, METH_VARARGS,
"This module is part of the balise_codec-package at https://github.com/FokkeB/subset36.\n\
The function 'convert' receives unshaped and/or shaped balise data from Python in a string of lines. \n\
Each line is terminated by a newline('\\n') - character and contains one of the following fields, or both (separated by a semicolon) : \n\
- unshaped hex data (output of balise design process) \n\
- shaped data (either hex or base64, either short or long) \n\
If only one field is given, the module will calculate the other field. \n\
If both input fields are given, the module will verify the correctness (is the shaped data correct? do the two fields match? etc). \n\
Comments are preceded with a '#', to allow loading balise data from files with comments (see example). \n\
The module returns to Python another string containing one of these lines for each line in the input data: \n\
<unshaped hex short/long>;<shaped hex/base64 short/long>;<errorcode>\\n \n\
The definition of the error codes (0=OK) is given in 'telegram.h', see the github page. "},
    { nullptr, nullptr, 0, nullptr } 
};

static PyModuleDef balise_codec_module = {
    PyModuleDef_HEAD_INIT,
    "balise_codec",                        
    "'balise_codec.pyd': Python module for (de)coding ETCS Eurobalise contents, version 5, August 2025. See: https://github.com/FokkeB/subset36.",      
    0,
    balise_codec_methods                  
};

PyMODINIT_FUNC PyInit_balise_codec() {
    return PyModule_Create(&balise_codec_module);
}