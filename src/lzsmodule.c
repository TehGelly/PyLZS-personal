#include "lzsmodule.h"

//lzs.compress(data -> bytes) -> bytes
static PyObject * 
py_lzs_compress(PyObject *self, PyObject *args)
{
	byte *data;
	Py_ssize_t data_len;
	int optimal_int = 0;
	
	if(!PyArg_ParseTuple(args,"s#|p", &data, &data_len, &optimal_int))
		return NULL;
	size_t out_size = 0;
	byte* out_bytes = lzs_compress(data, data_len, (bool)optimal_int, &out_size);
	if(out_bytes == NULL)
		return NULL;

	PyObject *retval = PyBytes_FromStringAndSize(out_bytes,(Py_ssize_t)out_size);
	free(out_bytes);
	return retval;
}

//lzs.decompress(data -> bytes) -> bytes
static PyObject *
py_lzs_decompress(PyObject *self, PyObject *args)
{
	byte *data;
	Py_ssize_t data_len;
	size_t expected_size = 0;
	
	if(!PyArg_ParseTuple(args,"s#|n", &data, &data_len, &expected_size))
		return NULL;
	
	//give a liiiittle room to the expected size
	if(!expected_size)
		//worst-case decompression size is 30 bytes per compressed byte (0xff -> clen=30)
		expected_size = 30*data_len;
	expected_size += 10;
	size_t out_size = 0;

	byte *out_bytes = lzs_decompress(data, data_len, expected_size, &out_size);
	if(out_bytes == NULL)
		return NULL;
	
	PyObject *retval = PyBytes_FromStringAndSize(out_bytes,(Py_ssize_t)out_size);
	free(out_bytes);
	return retval;
}

static PyMethodDef LZSMethods[] = {
	{"compress", py_lzs_compress, METH_VARARGS,
	"Compress a bytes object using Lempel-Ziv-Stac compression"},
	{"decompress", py_lzs_decompress, METH_VARARGS, 
	"Decompress a bytes object using Lempel-Ziv-Stac compress."},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef lzsmodule = {
    PyModuleDef_HEAD_INIT,
    "LZS",   /* name of module */
    NULL,    /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    LZSMethods
};

PyMODINIT_FUNC
PyInit_LZS(void)
{
	DEBUG_PRINT(("Debugging is enabled."));
	return PyModule_Create(&lzsmodule);
}