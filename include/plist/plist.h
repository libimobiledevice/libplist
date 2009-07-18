/*
 * plist.h
 * Main include of libplist
 *
 * Copyright (c) 2008 Jonathan Beck All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 */

#ifndef LIBPLIST_H
#define LIBPLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;

    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;

    #ifdef plist_EXPORTS
        #define PLIST_API  __declspec( dllexport )
    #else
        #define PLIST_API  __declspec( dllimport )
    #endif
#else
    #include <stdint.h>
    #define PLIST_API
#endif

#include <sys/types.h>

/**
 * \mainpage libplist : A library to handle Apple Property Lists
 * \defgroup PublicAPI Public libplist API
 */
/*@{*/


/**
 * The basic plist abstract data type.
 */
	typedef void *plist_t;

/**
 * The enumeration of plist node types.
 */
	typedef enum {
		PLIST_BOOLEAN,
					/**< Boolean, scalar type */
		PLIST_UINT,	/**< Unsigned integer, scalar type */
		PLIST_REAL,	/**< Real, scalar type */
		PLIST_STRING,
					/**< ASCII string, scalar type */
		PLIST_ARRAY,/**< Ordered array, structured type */
		PLIST_DICT,	/**< Unordered dictionary (key/value pair), structured type */
		PLIST_DATE,	/**< Date, scalar type */
		PLIST_DATA,	/**< Binary data, scalar type */
		PLIST_KEY,	/**< Key in dictionaries (ASCII String), scalar type */
		PLIST_NONE	/**< No type */
	} plist_type;


/********************************************
 *                                          *
 *          Creation & Destruction          *
 *                                          *
 ********************************************/

/**
 * Create a new root plist_t type #PLIST_DICT
 *
 * @return the created plist
 * @sa #plist_type
 */
	PLIST_API plist_t plist_new_dict(void);

/**
 * Create a new root plist_t type #PLIST_ARRAY
 *
 * @return the created plist
 * @sa #plist_type
 */
	PLIST_API plist_t plist_new_array(void);

/**
 * Destruct a plist_t node and all its children recursively
 *
 * @param plist the plist to free
 */
	PLIST_API void plist_free(plist_t plist);

/**
 * Return a copy of passed node and it's children
 *
 * @param plist the plist to copy
 * @return copied plist
 */
	PLIST_API plist_t plist_copy(plist_t node);


/********************************************
 *                                          *
 *            Tree navigation               *
 *                                          *
 ********************************************/

/**
 * Get the first child of a node
 *
 * @param node the first child
 */
	PLIST_API plist_t plist_get_first_child(plist_t node);


/**
 * Get the next sibling of a node
 *
 * @param node the next sibling
 */
	PLIST_API plist_t plist_get_next_sibling(plist_t node);


/**
 * Get the previous sibling of a node
 *
 * @param node the previous sibling
 */
	PLIST_API plist_t plist_get_prev_sibling(plist_t node);

/**
 * Get the parent of a node
 *
 * @param node the parent (NULL if node is root)
 */
	PLIST_API plist_t plist_get_parent(plist_t node);

/**
 * Get the nth child of a #PLIST_ARRAY node.
 *
 * @param node the node of type #PLIST_ARRAY
 * @param n the index of the child to get. Range is [0, array_size[
 * @return the nth children or NULL if node is not of type #PLIST_ARRAY
 */
	PLIST_API plist_t plist_get_array_nth_el(plist_t node, uint32_t n);

/**
 * Get the child of a #PLIST_DICT node from the associated key value.
 *
 * @param node the node of type #PLIST_DICT
 * @param key the key associated to the requested value
 * @return the key associated value or NULL if node is not of type #PLIST_DICT
 */
	PLIST_API plist_t plist_get_dict_el_from_key(plist_t node, const char *key);


/********************************************
 *                                          *
 *                Setters                   *
 *                                          *
 ********************************************/

/**
 * Add a subnode to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY). This function fails silently
 * if subnode already has a father.
 *
 * @param node the node to add a children to
 * @param subnode the children node
 */
	PLIST_API void plist_add_sub_node(plist_t node, plist_t subnode);

/**
 * Add a subnode of type #PLIST_KEY to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the key value encoded as an ASCII string (must be null terminated)
 */
	PLIST_API void plist_add_sub_key_el(plist_t node, const char *val);

/**
 * Add a subnode of type #PLIST_STRING to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the string value encoded as an ASCII or UTF-8 string (must be null terminated)
 */
	PLIST_API void plist_add_sub_string_el(plist_t node, const char *val);

/**
 * Add a subnode of type #PLIST_BOOLEAN to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the boolean value (TRUE or FALSE)
 */
	PLIST_API void plist_add_sub_bool_el(plist_t node, uint8_t val);

/**
 * Add a subnode of type #PLIST_UINT to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the unsigned integer value
 */
	PLIST_API void plist_add_sub_uint_el(plist_t node, uint64_t val);

/**
 * Add a subnode of type #PLIST_REAL to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the real value
 */
	PLIST_API void plist_add_sub_real_el(plist_t node, double val);

/**
 * Add a subnode of type #PLIST_DATA to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param val the binary buffer
 * @param length the length of the buffer
 */
	PLIST_API void plist_add_sub_data_el(plist_t node, const char *val, uint64_t length);

/**
 * Add a subnode of type #PLIST_DATE to a node. The node must be of a structured type
 * (ie #PLIST_DICT or #PLIST_ARRAY).
 *
 * @param node the node to add a children to
 * @param sec the number of seconds since 01/01/2001
 * @param usec the number of microseconds
 */
	PLIST_API void plist_add_sub_date_el(plist_t node, int32_t sec, int32_t usec);


/********************************************
 *                                          *
 *                Getters                   *
 *                                          *
 ********************************************/

/**
 * Get the #plist_type of a node.
 *
 * @param node the node
 * @return the type of the node
 */
	PLIST_API plist_type plist_get_node_type(plist_t node);

/**
 * Get the value of a #PLIST_KEY node.
 * This function does nothing if node is not of type #PLIST_KEY
 *
 * @param node the node
 * @param val a pointer to a C-string. This function allocates the memory,
 *            caller is responsible for freeing it.
 */
	PLIST_API void plist_get_key_val(plist_t node, char **val);

/**
 * Get the value of a #PLIST_STRING node.
 * This function does nothing if node is not of type #PLIST_STRING
 *
 * @param node the node
 * @param val a pointer to a C-string. This function allocates the memory,
 *            caller is responsible for freeing it. Data is UTF-8 encoded.
 */
	PLIST_API void plist_get_string_val(plist_t node, char **val);

/**
 * Get the value of a #PLIST_BOOLEAN node.
 * This function does nothing if node is not of type #PLIST_BOOLEAN
 *
 * @param node the node
 * @param val a pointer to a uint8_t variable.
 */
	PLIST_API void plist_get_bool_val(plist_t node, uint8_t * val);

/**
 * Get the value of a #PLIST_UINT node.
 * This function does nothing if node is not of type #PLIST_UINT
 *
 * @param node the node
 * @param val a pointer to a uint64_t variable.
 */
	PLIST_API void plist_get_uint_val(plist_t node, uint64_t * val);

/**
 * Get the value of a #PLIST_REAL node.
 * This function does nothing if node is not of type #PLIST_REAL
 *
 * @param node the node
 * @param val a pointer to a double variable.
 */
	PLIST_API void plist_get_real_val(plist_t node, double *val);

/**
 * Get the value of a #PLIST_DATA node.
 * This function does nothing if node is not of type #PLIST_DATA
 *
 * @param node the node
 * @param val a pointer to an unallocated char buffer. This function allocates the memory,
 *            caller is responsible for freeing it.
 */
	PLIST_API void plist_get_data_val(plist_t node, char **val, uint64_t * length);

/**
 * Get the value of a #PLIST_DATE node.
 * This function does nothing if node is not of type #PLIST_DATE
 *
 * @param node the node
 * @param sec a pointer to an int32_t variable. Represents the number of seconds since 01/01/2001.
 * @param usec a pointer to an int32_t variable. Represents the number of microseconds
 */
	PLIST_API void plist_get_date_val(plist_t node, int32_t * sec, int32_t * usec);


/********************************************
 *                                          *
 *                Setters                   *
 *                                          *
 ********************************************/

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_KEY
 *
 * @param node the node
 * @param val the key value
 */
	PLIST_API void plist_set_key_val(plist_t node, const char *val);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_STRING
 *
 * @param node the node
 * @param val the string value
 */
	PLIST_API void plist_set_string_val(plist_t node, const char *val);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_BOOLEAN
 *
 * @param node the node
 * @param val the boolean value
 */
	PLIST_API void plist_set_bool_val(plist_t node, uint8_t val);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_UINT
 *
 * @param node the node
 * @param val the unsigned integer value
 */
	PLIST_API void plist_set_uint_val(plist_t node, uint64_t val);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_REAL
 *
 * @param node the node
 * @param val the real value
 */
	PLIST_API void plist_set_real_val(plist_t node, double val);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_DATA
 *
 * @param node the node
 * @param val the binary buffer
 * @param length the length of the buffer
 */
	PLIST_API void plist_set_data_val(plist_t node, const char *val, uint64_t length);

/**
 * Set the value of a node.
 * Forces type of node to #PLIST_DATE
 *
 * @param node the node
 * @param sec the number of seconds since 01/01/2001
 * @param usec the number of microseconds
 */
	PLIST_API void plist_set_date_val(plist_t node, int32_t sec, int32_t usec);


/********************************************
 *                                          *
 *            Import & Export               *
 *                                          *
 ********************************************/

/**
 * Export the #plist_t structure to XML format.
 *
 * @param plist the root node to export
 * @param plist_xml a pointer to a C-string. This function allocates the memory,
 *            caller is responsible for freeing it. Data is UTF-8 encoded.
 * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
 */
	PLIST_API void plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length);

/**
 * Export the #plist_t structure to binary format.
 *
 * @param plist the root node to export
 * @param plist_bin a pointer to a char* buffer. This function allocates the memory,
 *            caller is responsible for freeing it.
 * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
 */
	PLIST_API void plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length);

/**
 * Import the #plist_t structure from XML format.
 *
 * @param plist_xml a pointer to the xml buffer.
 * @param length length of the buffer to read.
 * @param plist a pointer to the imported plist.
 */
	PLIST_API void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist);

/**
 * Import the #plist_t structure from binary format.
 *
 * @param plist_bin a pointer to the xml buffer.
 * @param length length of the buffer to read.
 * @param plist a pointer to the imported plist.
 */
	PLIST_API void plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist);



/********************************************
 *                                          *
 *                 Utils                    *
 *                                          *
 ********************************************/

/**
 * Find the first encountered #PLIST_KEY node mathing that key.
 * Search is breath first order.
 *
 * @param plist the root node of the plist structure.
 * @param value the ASCII Key to match.
 */
	PLIST_API plist_t plist_find_node_by_key(plist_t plist, const char *value);

/**
 * Find the first encountered #PLIST_STRING node mathing that string.
 * Search is breath first order.
 *
 * @param plist the root node of the plist structure.
 * @param value the ASCII String to match.
 */
	PLIST_API plist_t plist_find_node_by_string(plist_t plist, const char *value);

/**
 * Compare two node values
 *
 * @param node_l left node to compare
 * @param node_r rigth node to compare
 * @return TRUE is type and value match, FALSE otherwise.
 */
	PLIST_API char plist_compare_node_value(plist_t node_l, plist_t node_r);

/*@}*/


#ifdef __cplusplus
}
#endif
#endif
