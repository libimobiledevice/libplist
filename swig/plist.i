 /* swig.i */
 %module(package="libplist") PList
 %feature("autodoc", "1");
 %{
 /* Includes the header in the wrapper code */
 #include <plist/plist.h>
 #include <plist/plist++.h>

typedef struct {
	plist_t node;
	char should_keep_plist;
} PListNode;

PListNode *allocate_plist_wrapper(plist_t plist, char should_keep_plist) {
	PListNode* wrapper = (PListNode*) malloc(sizeof(PListNode));
	if (wrapper) {
		memset(wrapper, 0, sizeof(PListNode));
		wrapper->node = plist;
		wrapper->should_keep_plist = should_keep_plist;
		return wrapper;
	}
	return NULL;
}
 %}

%include "std_string.i"


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
%include <plist/Utils.h>

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


//deprecated wrapper below

%include "stdint.i"
%include "cstring.i"

/* Parse the header file to generate wrappers */
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

typedef struct {
} PListNode;

%extend PListNode {             // Attach these functions to struct Vector
	PListNode(plist_type t) {
		PListNode* node = NULL;
		switch (t) {
			case PLIST_ARRAY :
				node = allocate_plist_wrapper( plist_new_array(), 0 );
				break;
			case PLIST_DICT :
				node = allocate_plist_wrapper( plist_new_dict(), 0 );
				break;
			default :
				node = NULL;
				break;
		}
		return node;
	}

	%cstring_input_binary(char *data, uint64_t len);
	PListNode(char *data, uint64_t len) {
		//first check input
		if (len > 8) {
			plist_t plist = NULL;
			if (memcmp(data, "bplist00", 8) == 0) {
				plist_from_bin(data, len, &plist);
			} else {
				plist_from_xml(data, len, &plist);
			}
			if (plist)
				return allocate_plist_wrapper( plist, 0 );
		}
		return NULL;
	}

	~PListNode() {
		if (!$self->should_keep_plist) {
			plist_free($self->node);
		}
		$self->node = NULL;
		$self->should_keep_plist = 0;
		free($self);
		$self = NULL;
	}

	void add_sub_node(PListNode* subnode) {
		if (subnode) {
			plist_add_sub_node($self->node, subnode->node);
			//subnode is not root anymore. Do not delete tree
			subnode->should_keep_plist = 1;
		}
	}

	void add_sub_key(char* k) {
		plist_add_sub_key_el($self->node, k);
	}

	void add_sub_string(char* s) {
		plist_add_sub_string_el($self->node, s);
	}

	void add_sub_bool(char b) {
		plist_add_sub_bool_el($self->node, b);
	}

	void add_sub_uint(uint64_t i) {
		plist_add_sub_uint_el($self->node, i);
	}

	void add_sub_real(double d) {
		plist_add_sub_real_el($self->node, d);
	}

	%cstring_input_binary(char *data, uint64_t len);
	void add_sub_data(char *data, uint64_t len) {
		plist_add_sub_data_el($self->node, data, len);
	}

	void set_as_key(char* k) {
		plist_set_key_val($self->node, k);
	}

	void set_as_string(char* s) {
		plist_set_string_val($self->node, s);
	}

	void set_as_bool(char b) {
		plist_set_bool_val($self->node, b);
	}

	void set_as_uint(uint64_t i) {
		plist_set_uint_val($self->node, i);
	}

	void set_as_real(double d) {
		plist_set_real_val($self->node, d);
	}

	%cstring_input_binary(char *data, uint64_t len);
	void set_as_data(char *data, uint64_t len) {
		plist_set_data_val($self->node, data, len);
	}

	PListNode* get_first_child() {
		plist_t node = plist_get_first_child( $self->node );
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_next_sibling() {
		plist_t node = plist_get_next_sibling( $self->node );
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_prev_sibling() {
		plist_t node = plist_get_prev_sibling( $self->node );
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_parent() {
		plist_t node = plist_get_parent( $self->node );
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	%newobject as_key;
	char* as_key() {
		char* k = NULL;
		plist_get_key_val($self->node, &k);
		return k;
	}

	%newobject as_string;
	char* as_string() {
		char* s = NULL;
		plist_get_string_val($self->node, &s);
		return s;
	}

	char as_bool() {
		uint8_t b;
		plist_get_bool_val($self->node, &b);
		return (char)b;
	}

	uint64_t as_uint() {
		uint64_t i = 0;
		plist_get_uint_val($self->node, &i);
		return i;
	}

	double as_real() {
		double d = 0;
		plist_get_real_val($self->node, &d);
		return d;
	}

	%cstring_output_allocate_size(char **STRING, uint64_t *LENGTH, free(*$1));
	void as_data(char **STRING, uint64_t *LENGTH) {
		char* s = NULL;
		uint64_t l;
		plist_get_data_val($self->node, &s, &l);
		*STRING = s;
		*LENGTH = l;
		return;
	}

	plist_type get_type() {
		return plist_get_node_type($self->node);
	}

	PListNode* find_node_by_key(char *s) {
		plist_t node = plist_find_node_by_key($self->node, s);
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* find_node_by_string(char* s) {
		plist_t node = plist_find_node_by_string($self->node, s);
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_array_nth_el(unsigned int n) {
		plist_t node = plist_get_array_nth_el($self->node, n);
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_dict_el_from_key(char *key) {
		plist_t node = plist_get_dict_el_from_key($self->node, key);
		if (node) {
			return allocate_plist_wrapper(node, 1);
		}
		return NULL;
	}

	%newobject to_xml;
	char* to_xml () {
		char* s = NULL;
		uint32_t l;
		plist_to_xml($self->node, &s, &l);
		return s;
	}

	%cstring_output_allocate_size(char **STRING, uint64_t *LENGTH, free(*$1));
	void to_bin(char **STRING, uint64_t *LENGTH) {
		char* s = NULL;
		uint32_t l;
		plist_to_bin($self->node, &s, &l);
		*STRING = s;
		*LENGTH = l;
		return;
	}

	%cstring_input_binary(char *data, uint64_t len);
	void from_xml (char *data, uint64_t len) {
		if (!$self->should_keep_plist) {
			plist_free($self->node);
		}
		$self->node = NULL;
		$self->should_keep_plist = 0;
		plist_from_xml(data, len, &$self->node);
	}

	%cstring_input_binary(char *data, uint64_t len);
	void from_bin (char* data, uint64_t len) {
		if (!$self->should_keep_plist) {
			plist_free($self->node);
		}
		$self->node = NULL;
		$self->should_keep_plist = 0;
		plist_from_bin(data, len, &$self->node);
	}
};

