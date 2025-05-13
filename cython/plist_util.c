#include "plist_util.h"

#include <time.h>
#include <datetime.h>

int64_t datetime_to_timestamp(PyObject* obj) {
    PyDateTime_IMPORT;
    if (!PyDateTime_Check(obj)) {
        PyErr_SetString(PyExc_ValueError,"Expected a datetime");
        return 0;
    }
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_sec = PyDateTime_DATE_GET_SECOND(obj);
    t.tm_min = PyDateTime_DATE_GET_MINUTE(obj);
    t.tm_hour = PyDateTime_DATE_GET_HOUR(obj);
    t.tm_mday = PyDateTime_GET_DAY(obj);
    t.tm_mon = PyDateTime_GET_MONTH(obj)-1;
    t.tm_year = PyDateTime_GET_YEAR(obj)-1900;
    return mktime(&t);
}
PyObject* timestamp_to_datetime(int64_t sec) {
    time_t sec_tt = sec;
    struct tm* t = gmtime(&sec_tt);
    if(t){
        PyDateTime_IMPORT;
        return PyDateTime_FromDateAndTime(t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, 0);
    }
    return NULL;
}
int check_datetime(PyObject* ob) {
    if(ob){
        PyDateTime_IMPORT;
        return PyDateTime_Check(ob);
    }
    return 0;
}
