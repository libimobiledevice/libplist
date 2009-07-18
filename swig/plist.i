 /* swig.i */
 %module(package="libplist") PList
 %feature("autodoc", "1");
 %{
 /* Includes the header in the wrapper code */
 #include <plist/plist.h>
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
		char b;
		plist_get_bool_val($self->node, &b);
		return b;
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

