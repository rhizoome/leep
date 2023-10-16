#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <stdlib.h>

#define MAXPATH 512

static char* default_delim = ".";

typedef struct {
    PyObject* result;
    char* path;
    Py_ssize_t path_size;
    char* delim;
} get_args;

static PyObject* _get(char* buffer, get_args* pargs, char** last_token, int get_container) {
    if (pargs->path_size > MAXPATH) {
        PyErr_Format(PyExc_RuntimeError, "Path to long maxpath is %d", MAXPATH);
        return NULL;
    }

    // path_size does not include the \0-byte which we want to copy too
    memcpy(buffer, pargs->path, pargs->path_size + 1);
    
    char *saveptr;
    char* token = strtok_r(buffer, pargs->delim, &saveptr);
    char* preview = strtok_r(NULL, pargs->delim, &saveptr);
    while (token && (preview || !get_container)) {
        if(PySequence_Check(pargs->result)) {
            char *no_error;
            const Py_ssize_t index = (Py_ssize_t) strtol(token, &no_error, 10);
            if (!no_error) {
                PyErr_SetString(PyExc_ValueError, "Index not an integer.");
            }
            pargs->result = PySequence_GetItem(pargs->result, index);
            if (!pargs->result) return NULL;
        } else {
            PyObject* result_save = pargs->result;
            pargs->result = PyMapping_GetItemString(pargs->result, token);
            if (!pargs->result) {
                char *no_error;
                const long int index = strtol(token, &no_error, 10);
                if (!no_error) return NULL;

                PyErr_Clear();
                PyObject* index_obj = PyLong_FromLong(index);
                if (!index_obj) return NULL;

                pargs->result = PyObject_GetItem(result_save, index_obj);
                Py_DECREF(index_obj);
                if (!pargs->result) return NULL;
            };
        }
        token = preview;
        if (token) preview = strtok_r(NULL, pargs->delim, &saveptr);
    }
    *last_token = token;
    return pargs->result;
}

static PyObject* get(PyObject* self, PyObject* args) {
    get_args pargs;
    pargs.delim = default_delim;

    if (!PyArg_ParseTuple(args, "Os#|s", &pargs.result, &pargs.path, &pargs.path_size, &pargs.delim)) {
        return NULL;
    }

    char buffer[MAXPATH];
    char* last_token;
    PyObject* result = _get(buffer, &pargs, &last_token, 0);

    if (result) Py_INCREF(result);
    return result;
}

static PyObject* set(PyObject* self, PyObject* args) {
    get_args pargs;
    PyObject* value;
    pargs.delim = default_delim;
    if (!PyArg_ParseTuple(args, "Os#O|s", &pargs.result, &pargs.path, &pargs.path_size, &value, &pargs.delim)) {
        return NULL;
    }

    char buffer[MAXPATH];
    char* last_token;
    PyObject* result = _get(buffer, &pargs, &last_token, 1);

    if(PySequence_Check(result)) {
        char *no_error;
        const Py_ssize_t index = (Py_ssize_t) strtol(last_token, &no_error, 10);
        if (!no_error) {
            PyErr_SetString(PyExc_ValueError, "Index not an integer.");
        }
        if (PySequence_SetItem(result, index, value)) return NULL;
    } else {
        if (PyMapping_HasKeyString(result, last_token)) {
            if (PyMapping_SetItemString(result, last_token, value)) return NULL;
        } else {
            char *no_error;
            const long int index = strtol(last_token, &no_error, 10);
            if (!no_error) goto error;

            PyObject* index_obj = PyLong_FromLong(index);
            if (!index_obj) return NULL;

            if (PyMapping_HasKey(result, index_obj)) {
                if (PyObject_SetItem(result, index_obj, value)) return NULL;
            } else {
                Py_DECREF(index_obj);
                goto error;
            }
            Py_DECREF(index_obj);
        }
    }
    Py_INCREF(Py_None);
    return Py_None;

    error:
    PyErr_SetString(PyExc_KeyError, last_token);
    return NULL;
}

static PyMethodDef ExtensionMethods[] = {
    {"get", get, METH_VARARGS, "Get element defined by a path-expression."},
    {"set", set, METH_VARARGS, "Set existing element defined by a path-expression."},
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
