#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <stdlib.h>


static PyObject* path_expr(PyObject* self, PyObject* args) {
    PyObject* obj;
    PyObject* result;
    char* path;
    Py_ssize_t path_size;

    if (!PyArg_ParseTuple(args, "Os#", &obj, &path, &path_size)) {
        return NULL;
    }
    result = obj;
    const char* end = path + path_size;
    
    while (path < end) {
        if(PySequence_Check(result)) {
            char *no_error;
            const Py_ssize_t index = (Py_ssize_t) strtol(path, &no_error, 10);
            if (!no_error) {
                PyErr_SetString(PyExc_ValueError, "Index not an integer");
            }
            result = PySequence_GetItem(result, index);
            if (!result) return NULL;
        }
        else {
            const PyObject* result_save = result;
            result = PyMapping_GetItemString(result, path);
            if (!result) {
                char *no_error;
                const long int index = strtol(path, &no_error, 10);
                if (!no_error) return NULL;

                PyErr_Clear();
                const PyObject* index_obj = PyLong_FromLong(index);
                if (!index_obj) return NULL;

                result = PyObject_GetItem(result_save, index_obj);
                Py_DECREF(index_obj);
                if (!result) return NULL;
            };
        }
        path += strlen(path) + 1;
    }

    Py_INCREF(result);
    return result;
}

static PyMethodDef ExtensionMethods[] = {
    {"path_expr", path_expr, METH_VARARGS, "Return element from a path-expression."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef extensionmodule = {
    PyModuleDef_HEAD_INIT,
    "leep.extension",
    NULL,
    -1,
    ExtensionMethods
};

PyMODINIT_FUNC PyInit_extension(void) {
    return PyModule_Create(&extensionmodule);
}
