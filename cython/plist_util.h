#include <Python.h>

int64_t datetime_to_timestamp(PyObject* obj);
PyObject* timestamp_to_datetime(int64_t sec);
int check_datetime(PyObject* obj);
