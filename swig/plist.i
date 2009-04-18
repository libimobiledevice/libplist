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

PListNode *allocate_wrapper(plist_t plist, char should_keep_plist) {
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

/* Parse the header file to generate wrappers */
typedef enum {
	PLIST_BOOLEAN,
	PLIST_UINT,
	PLIST_REAL,
	PLIST_STRING,
	PLIST_UNICODE,
	PLIST_ARRAY,
	PLIST_DICT,
	PLIST_DATE,
	PLIST_DATA,
	PLIST_KEY,
	PLIST_NONE
} plist_type;

typedef struct {
	plist_t node;
	char should_keep_plist;
} PListNode;

%extend PListNode {             // Attach these functions to struct Vector
	PListNode(plist_type t) {
		PListNode* node = NULL;
		switch (t) {
			case PLIST_ARRAY :
				node = allocate_wrapper( plist_new_array(), 0 );
				break;
			case PLIST_DICT :
				node = allocate_wrapper( plist_new_dict(), 0 );
				break;
			default :
				node = NULL;
				break;
		}
		return node;
	}

	PListNode(char* xml) {
		plist_t plist = NULL;
		plist_from_xml(xml, strlen(xml), &plist);
		if (plist)
			return allocate_wrapper( plist, 0 );
		return NULL;
	}

	PListNode(char* bin, uint64_t len) {
		plist_t plist = NULL;
		plist_from_bin(bin, len, &plist);
		if (plist)
			return allocate_wrapper( plist, 0 );
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

	void add_sub_data(char* v, uint64_t l) {
		plist_add_sub_data_el($self->node, v, l);
	}

	PListNode* get_first_child() {
		plist_t node = plist_get_first_child( $self->node );
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_next_sibling() {
		plist_t node = plist_get_next_sibling( $self->node );
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_prev_sibling() {
		plist_t node = plist_get_prev_sibling( $self->node );
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	char* as_key() {
		char* k = NULL;
		plist_get_key_val($self->node, &k);
		return k;
	}

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

	char* as_data() {
		char* v;
		uint64_t l;
		plist_get_data_val($self->node, &v, &l);
		return v;
	}

	plist_type get_type() {
		return plist_get_node_type($self->node);
	}

	PListNode* find_node_by_key(char *s) {
		plist_t node = plist_find_node_by_key($self->node, s);
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* find_node_by_string(char* s) {
		plist_t node = plist_find_node_by_string($self->node, s);
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_array_nth_el(unsigned int n) {
		plist_t node = plist_get_array_nth_el($self->node, n);
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	PListNode* get_dict_el_from_key(char *key) {
		plist_t node = plist_get_dict_el_from_key($self->node, key);
		if (node) {
			return allocate_wrapper(node, 1);
		}
		return NULL;
	}

	char* to_xml () {
		char* s = NULL;
		uint32_t l;
		plist_to_xml($self->node, &s, &l);
		return s;
	}

	char* to_bin () {
		char* s = NULL;
		uint32_t l;
		plist_to_bin($self->node, &s, &l);
		return s;
	}

	void from_xml (char* xml) {
		if (!$self->should_keep_plist) {
			plist_free($self->node);
		}
		$self->node = NULL;
		$self->should_keep_plist = 0;
		plist_from_xml(xml, strlen(xml), &$self->node);
	}

	void from_bin (char* data, uint64_t len) {
		if (!$self->should_keep_plist) {
			plist_free($self->node);
		}
		$self->node = NULL;
		$self->should_keep_plist = 0;
		plist_from_bin(data, len, &$self->node);
	}
};

