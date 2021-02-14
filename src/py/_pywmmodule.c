#include <Python.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <wlr/util/log.h>
#include "wm/wm.h"
#include "wm/wm_config.h"
#include "py/_pywm_callbacks.h"
#include "py/_pywm_view.h"
#include "py/_pywm_widget.h"

static void sig_handler(int sig) {
    void *array[10];
    size_t size;

    size = backtrace(array, 100);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}


static void handle_update(){
    PyGILState_STATE gil = PyGILState_Ensure();

    PyObject* args = Py_BuildValue("()");
    PyObject* res = PyObject_Call(_pywm_callbacks_get_all()->query, args, NULL);
    Py_XDECREF(args);

    int update_cursor;
    int terminate;

    if(!PyArg_ParseTuple(res, 
                "pp",
                &update_cursor,
                &terminate)){
        PyErr_SetString(PyExc_TypeError, "Cannot parse query return");
    }else{
        if(update_cursor){
            wm_update_cursor();
        }
        if(terminate){
            wm_terminate();
        }
    }
    Py_XDECREF(res);


    _pywm_widgets_update();
    
    _pywm_views_update();


    PyGILState_Release(gil);
}



static PyObject* _pywm_run(PyObject* self, PyObject* args, PyObject* kwargs){
    /* Dubug: Print stacktrace upon segfault etc. */
    signal(SIGSEGV, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);

    fprintf(stderr, "Running PyWM...\n");

    int status = 0;

    struct wm_config conf;
    wm_config_init_default(&conf);

    char* kwlist[] = {
        "output_scale",
        "xcursor_theme",
        "xcursor_name",
        "xcursor_size",
        "focus_follows_mouse",
        "constrain_popups_to_toplevel",
        "encourage_csd",
        "touchpad_device_name",
        NULL
    };
    char* ignore;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "|dssippps", kwlist,
                &conf.output_scale,
                &conf.xcursor_theme,
                &conf.xcursor_name,
                &conf.xcursor_size,
                &conf.focus_follows_mouse,
                &conf.constrain_popups_to_toplevel,
                &conf.encourage_csd,
                &ignore)){
        PyErr_SetString(PyExc_TypeError, "Cannot parse run arguments");
        return NULL;
    }

    /* Register callbacks immediately, might be called during init */
    get_wm()->callback_update = handle_update;
    _pywm_callbacks_init();

    wm_init(&conf);

    Py_BEGIN_ALLOW_THREADS;
    status = wm_run();
    Py_END_ALLOW_THREADS;

    fprintf(stderr, "...finished\n");

    return Py_BuildValue("i", status);
}

static PyObject* _pywm_register(PyObject* self, PyObject* args){
    const char* name;
    PyObject* callback;

    if(!PyArg_ParseTuple(args, "sO", &name, &callback)){
        PyErr_SetString(PyExc_TypeError, "Invalid parameters");
        return NULL;
    }

    if(!PyCallable_Check(callback)){
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return NULL;
    }
    
    PyObject** target = _pywm_callbacks_get(name);
    if(!target){
        PyErr_SetString(PyExc_TypeError, "Unknown callback");
        return NULL;
    }

    Py_XDECREF(*target);
    *target = callback;
    Py_INCREF(*target);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef _pywm_methods[] = {
    { "run",                       (PyCFunction)_pywm_run,           METH_VARARGS | METH_KEYWORDS,   "Start the compositor in this thread" },
    { "register",                  _pywm_register,                   METH_VARARGS,                   "Register callback"  },

    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef _pywm = {
    PyModuleDef_HEAD_INIT,
    "_pywm",
    "",
    -1,
    _pywm_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC PyInit__pywm(void){
    return PyModule_Create(&_pywm);
}
