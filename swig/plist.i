 /* swig.i */
 %module(package="libplist") PList
 %{
 /* Includes the header in the wrapper code */
 #include <plist/plist.h>
typedef struct {
	plist_t node;
} PListNode;
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
} PListNode;

%extend PListNode {             // Attach these functions to struct Vector
	PListNode(plist_type t) {
		PListNode* node = NULL;
		switch (t) {
			case PLIST_ARRAY :
				node = (PListNode*) malloc(sizeof(PListNode));
				node->node = plist_new_array();
				break;
			case PLIST_DICT :
				node = (PListNode*) malloc(sizeof(PListNode));
				node->node = plist_new_dict();
				break;
			default :
				node = NULL;
				break;
		}
		return node;
	}

	PListNode(char* xml) {
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist_from_xml(xml, strlen(xml), &plist->node);
		return plist;
	}

	PListNode(char* bin, uint64_t len) {
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist_from_bin(bin, len, &plist->node);
		return plist;
	}

	~PListNode() {
		plist_free($self->node);
		free($self);
	}

	void AddSubNode(PListNode* subnode) {
		plist_add_sub_node($self->node, subnode);
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
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist_get_first_child(&$self->node);
		return plist;
	}

	PListNode* get_next_sibling() {
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist_get_next_sibling(&$self->node);
		return plist;
	}

	PListNode* get_prev_sibling() {
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist_get_prev_sibling(&$self->node);
		return plist;
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

	PListNode* find_sub_node_by_string(char* s) {
		PListNode* plist = (PListNode*) malloc(sizeof(PListNode));
		plist = plist_find_node_by_string($self->node, s);
		return plist;
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
};

