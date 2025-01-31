#ifndef _LZS_MODULE
#define _LZS_MODULE

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "bitstream.h"
#include "lzs_decompress.h"
#include "lzs_compress.h"
#include <stdint.h>

#define DEBUG_MODULE
#undef DEBUG_MODULE

#ifdef DEBUG_MODULE
# define DEBUG_PRINT(x) PySys_WriteStdout x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

static PyObject * py_lzs_compress(PyObject *self, PyObject *args);

static PyObject * py_lzs_decompress(PyObject *self, PyObject *args);

PyMODINIT_FUNC PyInit_LZS(void);

#endif