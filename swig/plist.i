 /* swig.i */
 %module plist
 %feature("autodoc", "1");
 %{
 /* Includes the header in the wrapper code */
 #include <plist/plist++.h>
 %}

%include "std_string.i"
%include "stdint.i"

%typemap(out) std::vector<char> {
   $result = SWIG_FromCharPtrAndSize((const char*)&($1[0]),(int)($1.size()));
}

%typemap(in) (const std::vector<char>& v)
{
    char* buffer = NULL;
    int length = 0;
    SWIG_AsCharPtrAndSize($input, &buffer, &length, NULL);
    $1 = std::vector<char>(buffer, buffer + length);
}

#if SWIGPYTHON
//for datetime in python
%{
#include <ctime>
#include <datetime.h>
%}

%typemap(typecheck,precedence=SWIG_TYPECHECK_POINTER) timeval {
    PyDateTime_IMPORT;
    $1 = PyDateTime_Check($input) ? 1 : 0;
}

%typemap(out) timeval {
    struct tm* t = gmtime ( &$1.tv_sec );
    if (t)
    {
	PyDateTime_IMPORT;
	$result = PyDateTime_FromDateAndTime(t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, $1.tv_usec);
    }
}

%typemap(in) (timeval t)
{
    PyDateTime_IMPORT;
    if (!PyDateTime_Check($input)) {
	PyErr_SetString(PyExc_ValueError,"Expected a datetime");
	return NULL;
    }
    struct tm t = {
	PyDateTime_DATE_GET_SECOND($input),
	PyDateTime_DATE_GET_MINUTE($input),
	PyDateTime_DATE_GET_HOUR($input),
	PyDateTime_GET_DAY($input),
	PyDateTime_GET_MONTH($input)-1,
	PyDateTime_GET_YEAR($input)-1900,
	0,0,0
    };
    timeval ret = {(int)mktime(&t), PyDateTime_DATE_GET_MICROSECOND($input)};
    $1 = ret;
}
#endif

%apply SWIGTYPE *DYNAMIC { PList::Node* };
%apply SWIGTYPE *DYNAMIC { PList::Structure* };

%{
static swig_type_info *Node_dynamic(void **ptr)
{
    PList::Node* node = dynamic_cast<PList::Node *>((PList::Node *) *ptr);
    if (node)
    {
	plist_type type = node->GetType();
	switch(type)
	{
	    case PLIST_DICT:
		*ptr = dynamic_cast<PList::Dictionary *>(node);
		return SWIGTYPE_p_PList__Dictionary;
	    case PLIST_ARRAY:
		*ptr = dynamic_cast<PList::Array *>(node);
		return SWIGTYPE_p_PList__Array;
	    case PLIST_BOOLEAN:
		*ptr = dynamic_cast<PList::Boolean *>(node);
		return SWIGTYPE_p_PList__Boolean;
	    case PLIST_UINT:
		*ptr = dynamic_cast<PList::Integer *>(node);
		return SWIGTYPE_p_PList__Integer;
	    case PLIST_REAL:
		*ptr = dynamic_cast<PList::Real *>(node);
		return SWIGTYPE_p_PList__Real;
	    case PLIST_STRING:
		*ptr = dynamic_cast<PList::String *>(node);
		return SWIGTYPE_p_PList__String;
	    case PLIST_DATE:
		*ptr = dynamic_cast<PList::Date *>(node);
		return SWIGTYPE_p_PList__Date;
	    case PLIST_DATA:
		*ptr = dynamic_cast<PList::Data *>(node);
		return SWIGTYPE_p_PList__Data;
	    default:
		break;
	}
    }
    return 0;
}
%}

// Register the above casting function
DYNAMIC_CAST(SWIGTYPE_p_PList__Node, Node_dynamic);
DYNAMIC_CAST(SWIGTYPE_p_PList__Structure, Node_dynamic);

%include "std_map.i"
// Instantiate templates used by example
namespace std {
    %template(PairStringNodePtr) std::pair<string, PList::Node*>;
    %template(MapStringNodePtr) map<string,PList::Node*>;
}

#if SWIGPYTHON
%rename(__assign__) *::operator=;
%rename(__getitem__) *::operator[];
%rename(__delitem__) *::Remove;
%rename(__setitem__) PList::Dictionary::Insert;
%rename(__deepcopy__) *::Clone;
%rename(__len__) *::GetSize;
%rename(get_type) *::GetType;
%rename(set_value) *::SetValue;
%rename(get_value) *::GetValue;
%rename(to_xml) *::ToXml;
%rename(to_bin) *::ToBin;
%rename(from_xml) *::FromXml;
%rename(from_bin) *::FromBin;
%rename(append) *::Append;
%rename(insert) PList::Array::Insert;
#endif

%ignore GetPlist();
%ignore Boolean(plist_t);
%ignore Integer(plist_t);
%ignore Real(plist_t);
%ignore String(plist_t);
%ignore Data(plist_t);
%ignore Date(plist_t);
%ignore Array(plist_t);
%ignore Dictionary(plist_t);
%ignore Begin();
%ignore End();
%ignore Find();

%include <plist/Node.h>
%include <plist/Boolean.h>
%include <plist/Integer.h>
%include <plist/Real.h>
%include <plist/String.h>
%include <plist/Data.h>
%include <plist/Date.h>
%include <plist/Structure.h>
%include <plist/Array.h>
%include <plist/Dictionary.h>

typedef enum {
	PLIST_BOOLEAN,
	PLIST_UINT,
	PLIST_REAL,
	PLIST_STRING,
	PLIST_ARRAY,
	PLIST_DICT,
	PLIST_DATE,
	PLIST_DATA,
	PLIST_KEY,
	PLIST_NONE
} plist_type;

#if SWIGPYTHON

#if SWIG_VERSION <= 0x010336
#define SwigPyIterator PySwigIterator
#endif

%extend PList::Dictionary {

    %newobject key_iterator(PyObject **PYTHON_SELF);
    swig::SwigPyIterator* key_iterator(PyObject **PYTHON_SELF) {
	return swig::make_output_key_iterator(self->Begin(), self->Begin(), self->End(), *PYTHON_SELF);
    }

    %newobject value_iterator(PyObject **PYTHON_SELF);
    swig::SwigPyIterator* value_iterator(PyObject **PYTHON_SELF) {
	return swig::make_output_value_iterator(self->Begin(), self->Begin(), self->End(), *PYTHON_SELF);
    }

    iterator iteritems()
    {
	return self->Begin();
    }

    bool has_key(const std::string& key) const {
	PList::Dictionary* dict = const_cast<PList::Dictionary*>(self);
	PList::Dictionary::iterator i = dict->Find(key);
	return i != dict->End();
    }

    PyObject* keys() {
	uint32_t size = self->GetSize();
	int pysize = (size <= (uint32_t) INT_MAX) ? (int) size : -1;
	if (pysize < 0) {
	    SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	    PyErr_SetString(PyExc_OverflowError,
			    "map size not valid in python");
			    SWIG_PYTHON_THREAD_END_BLOCK;
			    return NULL;
	}
	PyObject* keyList = PyList_New(pysize);
	PList::Dictionary::iterator i = self->Begin();
	for (int j = 0; j < pysize; ++i, ++j) {
	    PyList_SET_ITEM(keyList, j, swig::from(i->first));
	}
	return keyList;
    }

    PyObject* values() {
	uint32_t size = self->GetSize();
	int pysize = (size <= (uint32_t) INT_MAX) ? (int) size : -1;
	if (pysize < 0) {
	    SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	    PyErr_SetString(PyExc_OverflowError,
			    "map size not valid in python");
			    SWIG_PYTHON_THREAD_END_BLOCK;
			    return NULL;
	}
	PyObject* valList = PyList_New(pysize);
	PList::Dictionary::iterator i = self->Begin();
	for (int j = 0; j < pysize; ++i, ++j) {
	    PList::Node *second = i->second;
	    PyObject *down = SWIG_NewPointerObj(SWIG_as_voidptr(second), SWIG_TypeDynamicCast(SWIGTYPE_p_PList__Node, SWIG_as_voidptrptr(&second)), 0 |  0 );
	    PyList_SET_ITEM(valList, j, down);
	}
	return valList;
    }

    PyObject* items() {
	uint32_t size = self->GetSize();
	int pysize = (size <= (uint32_t) INT_MAX) ? (int) size : -1;
	if (pysize < 0) {
	    SWIG_PYTHON_THREAD_BEGIN_BLOCK;
	    PyErr_SetString(PyExc_OverflowError,
			    "map size not valid in python");
			    SWIG_PYTHON_THREAD_END_BLOCK;
			    return NULL;
	}
	PyObject* itemList = PyList_New(pysize);
	PList::Dictionary::iterator i = self->Begin();
	for (int j = 0; j < pysize; ++i, ++j) {
	    PyObject *item = PyTuple_New(2);
	    PList::Node *second = i->second;
	    PyObject *down = SWIG_NewPointerObj(SWIG_as_voidptr(second), SWIG_TypeDynamicCast(SWIGTYPE_p_PList__Node, SWIG_as_voidptrptr(&second)), 0 |  0 );
	    PyTuple_SetItem(item, 0, swig::from(i->first));
	    PyTuple_SetItem(item, 1, down);
	    PyList_SET_ITEM(itemList, j, item);
	}
	return itemList;
    }

    %pythoncode {def __iter__(self): return self.key_iterator()}
    %pythoncode {def iterkeys(self): return self.key_iterator()}
    %pythoncode {def itervalues(self): return self.value_iterator()}
}

#undef SwigPyIterator
#endif
