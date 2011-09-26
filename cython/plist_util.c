#include "plist_util.h"

#include <time.h>
#include <datetime.h>

void datetime_to_ints(PyObject* obj, int32_t* sec, int32_t* usec) {
    PyDateTime_IMPORT;
    if (!PyDateTime_Check(obj)) {
		PyErr_SetString(PyExc_ValueError,"Expected a datetime");
		sec = NULL;
		usec = NULL;
		return;
    }
    struct tm t = {
		PyDateTime_DATE_GET_SECOND(obj),
		PyDateTime_DATE_GET_MINUTE(obj),
		PyDateTime_DATE_GET_HOUR(obj),
		PyDateTime_GET_DAY(obj),
		PyDateTime_GET_MONTH(obj)-1,
		PyDateTime_GET_YEAR(obj)-1900,
		0,0,0
    };
	*sec = (int32_t)mktime(&t);
	*usec = PyDateTime_DATE_GET_MICROSECOND(obj);
}
PyObject* ints_to_datetime(int32_t sec, int32_t usec) {
	time_t sec_tt = sec;
    struct tm* t = gmtime(&sec_tt);
    if(t){
		PyDateTime_IMPORT;
		return PyDateTime_FromDateAndTime(t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, usec);
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
