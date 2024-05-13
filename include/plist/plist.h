/**
 * @file plist/plist.h
 * @brief Main include of libplist
 * \internal
 *
 * Copyright (c) 2012-2023 Nikias Bassen, All Rights Reserved.
 * Copyright (c) 2008-2009 Jonathan Beck, All Rights Reserved.
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
extern "C"
{
#endif

#if _MSC_VER && _MSC_VER < 1700
    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;

    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;

#else
#include <stdint.h>
#endif

/*{{{ deprecation macros */
#ifdef __llvm__
  #if defined(__has_extension)
    #if (__has_extension(attribute_deprecated_with_message))
      #ifndef PLIST_WARN_DEPRECATED
        #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated(x)))
      #endif
    #else
      #ifndef PLIST_WARN_DEPRECATED
        #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated))
      #endif
    #endif
  #else
    #ifndef PLIST_WARN_DEPRECATED
      #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated))
    #endif
  #endif
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 5)))
  #ifndef PLIST_WARN_DEPRECATED
    #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated(x)))
  #endif
#elif defined(_MSC_VER)
  #ifndef PLIST_WARN_DEPRECATED
    #define PLIST_WARN_DEPRECATED(x) __declspec(deprecated(x))
  #endif
#else
  #define PLIST_WARN_DEPRECATED(x)
  #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#endif
/*}}}*/

#ifndef PLIST_API
  #ifdef LIBPLIST_STATIC
    #define PLIST_API
  #elif defined(_WIN32)
    #define PLIST_API __declspec(dllimport)
  #else
    #define PLIST_API
  #endif
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>

    /**
     * libplist : A library to handle Apple Property Lists
     * \defgroup PublicAPI Public libplist API
     */
    /*@{*/


    /**
     * The basic plist abstract data type.
     */
    typedef void *plist_t;

    /**
     * The plist dictionary iterator.
     */
    typedef void* plist_dict_iter;

    /**
     * The plist array iterator.
     */
    typedef void* plist_array_iter;

    /**
     * The enumeration of plist node types.
     */
    typedef enum
    {
        PLIST_NONE =-1, /**< No type */
        PLIST_BOOLEAN,  /**< Boolean, scalar type */
        PLIST_INT,      /**< Integer, scalar type */
        PLIST_REAL,     /**< Real, scalar type */
        PLIST_STRING,   /**< ASCII string, scalar type */
        PLIST_ARRAY,    /**< Ordered array, structured type */
        PLIST_DICT,     /**< Unordered dictionary (key/value pair), structured type */
        PLIST_DATE,     /**< Date, scalar type */
        PLIST_DATA,     /**< Binary data, scalar type */
        PLIST_KEY,      /**< Key in dictionaries (ASCII String), scalar type */
        PLIST_UID,      /**< Special type used for 'keyed encoding' */
        PLIST_NULL,     /**< NULL type */
    } plist_type;

    /* for backwards compatibility */
    #define PLIST_UINT PLIST_INT

    /**
     * libplist error values
     */
    typedef enum
    {
        PLIST_ERR_SUCCESS      =  0,  /**< operation successful */
        PLIST_ERR_INVALID_ARG  = -1,  /**< one or more of the parameters are invalid */
        PLIST_ERR_FORMAT       = -2,  /**< the plist contains nodes not compatible with the output format */
        PLIST_ERR_PARSE        = -3,  /**< parsing of the input format failed */
        PLIST_ERR_NO_MEM       = -4,  /**< not enough memory to handle the operation */
        PLIST_ERR_IO           = -5,  /**< I/O error */
        PLIST_ERR_UNKNOWN      = -255 /**< an unspecified error occurred */
    } plist_err_t;

    /**
     * libplist format types
     */
    typedef enum
    {
        PLIST_FORMAT_NONE    = 0,  /**< No format */
        PLIST_FORMAT_XML     = 1,  /**< XML format */
        PLIST_FORMAT_BINARY  = 2,  /**< bplist00 format */
        PLIST_FORMAT_JSON    = 3,  /**< JSON format */
        PLIST_FORMAT_OSTEP   = 4,  /**< OpenStep "old-style" plist format */
        /* 5-9 are reserved for possible future use */
        PLIST_FORMAT_PRINT   = 10, /**< human-readable output-only format */
        PLIST_FORMAT_LIMD    = 11, /**< "libimobiledevice" output-only format (ideviceinfo) */
        PLIST_FORMAT_PLUTIL  = 12, /**< plutil-style output-only format */
    } plist_format_t;

    /**
     * libplist write options
     */
    typedef enum
    {
        PLIST_OPT_NONE      = 0, /**< Default value to use when none of the options is needed. */
        PLIST_OPT_COMPACT   = 1 << 0, /**< Use a compact representation (non-prettified). Only valid for #PLIST_FORMAT_JSON and #PLIST_FORMAT_OSTEP. */
        PLIST_OPT_PARTIAL_DATA = 1 << 1, /**< Print 24 bytes maximum of #PLIST_DATA values. If the data is longer than 24 bytes,  the first 16 and last 8 bytes will be written. Only valid for #PLIST_FORMAT_PRINT. */
        PLIST_OPT_NO_NEWLINE = 1 << 2, /**< Do not print a final newline character. Only valid for #PLIST_FORMAT_PRINT, #PLIST_FORMAT_LIMD, and #PLIST_FORMAT_PLUTIL. */
        PLIST_OPT_INDENT = 1 << 3, /**< Indent each line of output. Currently only #PLIST_FORMAT_PRINT and #PLIST_FORMAT_LIMD are supported. Use #PLIST_OPT_INDENT_BY() macro to specify the level of indentation. */
    } plist_write_options_t;

    /** To be used with #PLIST_OPT_INDENT - encodes the level of indentation for OR'ing it into the #plist_write_options_t bitfield. */
    #define PLIST_OPT_INDENT_BY(x) ((x & 0xFF) << 24)


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
     * Create a new plist_t type #PLIST_STRING
     *
     * @param val the sting value, encoded in UTF8.
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_string(const char *val);

    /**
     * Create a new plist_t type #PLIST_BOOLEAN
     *
     * @param val the boolean value, 0 is false, other values are true.
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_bool(uint8_t val);

    /**
     * Create a new plist_t type #PLIST_INT with an unsigned integer value
     *
     * @param val the unsigned integer value
     * @return the created item
     * @sa #plist_type
     * @note The value is always stored as uint64_t internally.
     *    Use #plist_get_uint_val or #plist_get_int_val to get the unsigned or signed value.
     */
    PLIST_API plist_t plist_new_uint(uint64_t val);

    /**
     * Create a new plist_t type #PLIST_INT with a signed integer value
     *
     * @param val the signed integer value
     * @return the created item
     * @sa #plist_type
     * @note The value is always stored as uint64_t internally.
     *    Use #plist_get_uint_val or #plist_get_int_val to get the unsigned or signed value.
     */
    PLIST_API plist_t plist_new_int(int64_t val);

    /**
     * Create a new plist_t type #PLIST_REAL
     *
     * @param val the real value
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_real(double val);

    /**
     * Create a new plist_t type #PLIST_DATA
     *
     * @param val the binary buffer
     * @param length the length of the buffer
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_data(const char *val, uint64_t length);

    /**
     * Create a new plist_t type #PLIST_DATE
     *
     * @param sec the number of seconds since 01/01/2001
     * @param usec the number of microseconds
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_date(int32_t sec, int32_t usec);

    /**
     * Create a new plist_t type #PLIST_UID
     *
     * @param val the unsigned integer value
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_uid(uint64_t val);

    /**
     * Create a new plist_t type #PLIST_NULL
     * @return the created item
     * @sa #plist_type
     * @note This type is not valid for all formats, e.g. the XML format
     *     does not support it.
     */
    PLIST_API plist_t plist_new_null(void);

    /**
     * Destruct a plist_t node and all its children recursively
     *
     * @param plist the plist to free
     */
    PLIST_API void plist_free(plist_t plist);

    /**
     * Return a copy of passed node and it's children
     *
     * @param node the plist to copy
     * @return copied plist
     */
    PLIST_API plist_t plist_copy(plist_t node);


    /********************************************
     *                                          *
     *            Array functions               *
     *                                          *
     ********************************************/

    /**
     * Get size of a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @return size of the #PLIST_ARRAY node
     */
    PLIST_API uint32_t plist_array_get_size(plist_t node);

    /**
     * Get the nth item in a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param n the index of the item to get. Range is [0, array_size[
     * @return the nth item or NULL if node is not of type #PLIST_ARRAY
     */
    PLIST_API plist_t plist_array_get_item(plist_t node, uint32_t n);

    /**
     * Get the index of an item. item must be a member of a #PLIST_ARRAY node.
     *
     * @param node the node
     * @return the node index or UINT_MAX if node index can't be determined
     */
    PLIST_API uint32_t plist_array_get_item_index(plist_t node);

    /**
     * Set the nth item in a #PLIST_ARRAY node.
     * The previous item at index n will be freed using #plist_free
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item at index n. The array is responsible for freeing item when it is no longer needed.
     * @param n the index of the item to get. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_set_item(plist_t node, plist_t item, uint32_t n);

    /**
     * Append a new item at the end of a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item. The array is responsible for freeing item when it is no longer needed.
     */
    PLIST_API void plist_array_append_item(plist_t node, plist_t item);

    /**
     * Insert a new item at position n in a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item to insert. The array is responsible for freeing item when it is no longer needed.
     * @param n The position at which the node will be stored. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_insert_item(plist_t node, plist_t item, uint32_t n);

    /**
     * Remove an existing position in a #PLIST_ARRAY node.
     * Removed position will be freed using #plist_free.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param n The position to remove. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_remove_item(plist_t node, uint32_t n);

    /**
     * Remove a node that is a child node of a #PLIST_ARRAY node.
     * node will be freed using #plist_free.
     *
     * @param node The node to be removed from its #PLIST_ARRAY parent.
     */
    PLIST_API void plist_array_item_remove(plist_t node);

    /**
     * Create an iterator of a #PLIST_ARRAY node.
     * The allocated iterator should be freed with the standard free function.
     *
     * @param node The node of type #PLIST_ARRAY
     * @param iter Location to store the iterator for the array.
     */
    PLIST_API void plist_array_new_iter(plist_t node, plist_array_iter *iter);

    /**
     * Increment iterator of a #PLIST_ARRAY node.
     *
     * @param node The node of type #PLIST_ARRAY.
     * @param iter Iterator of the array
     * @param item Location to store the item. The caller must *not* free the
     *          returned item. Will be set to NULL when no more items are left
     *          to iterate.
     */
    PLIST_API void plist_array_next_item(plist_t node, plist_array_iter iter, plist_t *item);


    /********************************************
     *                                          *
     *         Dictionary functions             *
     *                                          *
     ********************************************/

    /**
     * Get size of a #PLIST_DICT node.
     *
     * @param node the node of type #PLIST_DICT
     * @return size of the #PLIST_DICT node
     */
    PLIST_API uint32_t plist_dict_get_size(plist_t node);

    /**
     * Create an iterator of a #PLIST_DICT node.
     * The allocated iterator should be freed with the standard free function.
     *
     * @param node The node of type #PLIST_DICT.
     * @param iter Location to store the iterator for the dictionary.
     */
    PLIST_API void plist_dict_new_iter(plist_t node, plist_dict_iter *iter);

    /**
     * Increment iterator of a #PLIST_DICT node.
     *
     * @param node The node of type #PLIST_DICT
     * @param iter Iterator of the dictionary
     * @param key Location to store the key, or NULL. The caller is responsible
     *		for freeing the the returned string.
     * @param val Location to store the value, or NULL. The caller must *not*
     *		free the returned value. Will be set to NULL when no more
     *		key/value pairs are left to iterate.
     */
    PLIST_API void plist_dict_next_item(plist_t node, plist_dict_iter iter, char **key, plist_t *val);

    /**
     * Get key associated key to an item. Item must be member of a dictionary.
     *
     * @param node the item
     * @param key a location to store the key. The caller is responsible for freeing the returned string.
     */
    PLIST_API void plist_dict_get_item_key(plist_t node, char **key);

    /**
     * Get the nth item in a #PLIST_DICT node.
     *
     * @param node the node of type #PLIST_DICT
     * @param key the identifier of the item to get.
     * @return the item or NULL if node is not of type #PLIST_DICT. The caller should not free
     *		the returned node.
     */
    PLIST_API plist_t plist_dict_get_item(plist_t node, const char* key);

    /**
     * Get key node associated to an item. Item must be member of a dictionary.
     *
     * @param node the item
     * @return the key node of the given item, or NULL.
     */
    PLIST_API plist_t plist_dict_item_get_key(plist_t node);

    /**
     * Set item identified by key in a #PLIST_DICT node.
     * The previous item identified by key will be freed using #plist_free.
     * If there is no item for the given key a new item will be inserted.
     *
     * @param node the node of type #PLIST_DICT
     * @param item the new item associated to key
     * @param key the identifier of the item to set.
     */
    PLIST_API void plist_dict_set_item(plist_t node, const char* key, plist_t item);

    /**
     * Remove an existing position in a #PLIST_DICT node.
     * Removed position will be freed using #plist_free
     *
     * @param node the node of type #PLIST_DICT
     * @param key The identifier of the item to remove. Assert if identifier is not present.
     */
    PLIST_API void plist_dict_remove_item(plist_t node, const char* key);

    /**
     * Merge a dictionary into another. This will add all key/value pairs
     * from the source dictionary to the target dictionary, overwriting
     * any existing key/value pairs that are already present in target.
     *
     * @param target pointer to an existing node of type #PLIST_DICT
     * @param source node of type #PLIST_DICT that should be merged into target
     */
    PLIST_API void plist_dict_merge(plist_t *target, plist_t source);

    /**
     * Get a boolean value from a given #PLIST_DICT entry.
     *
     * The value node can be of type #PLIST_BOOLEAN, but also
     * #PLIST_STRING (either 'true' or 'false'),
     * #PLIST_INT with a numerical value of 0 or >= 1,
     * or #PLIST_DATA with a single byte with a value of 0 or >= 1.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return 0 or 1 depending on the value of the node.
     */
    PLIST_API uint8_t plist_dict_get_bool(plist_t dict, const char *key);

    /**
     * Get a signed integer value from a given #PLIST_DICT entry.
     * The value node can be of type #PLIST_INT, but also
     * #PLIST_STRING with a numerical value as string (decimal or hexadecimal),
     * or #PLIST_DATA with a size of 1, 2, 4, or 8 bytes in little endian byte order.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return Signed integer value depending on the value of the node.
     */
    PLIST_API int64_t plist_dict_get_int(plist_t dict, const char *key);

    /**
     * Get an unsigned integer value from a given #PLIST_DICT entry.
     * The value node can be of type #PLIST_INT, but also
     * #PLIST_STRING with a numerical value as string (decimal or hexadecimal),
     * or #PLIST_DATA with a size of 1, 2, 4, or 8 bytes in little endian byte order.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return Signed integer value depending on the value of the node.
     */
    PLIST_API uint64_t plist_dict_get_uint(plist_t dict, const char *key);

    /**
     * Copy a node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_item(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a boolean value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The boolean value from *source_dict* is retrieved with #plist_dict_get_bool,
     *     but is **always** created as #PLIST_BOOLEAN in *target_dict*.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_bool(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a signed integer value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The signed integer value from *source_dict* is retrieved with #plist_dict_get_int,
     *     but is **always** created as #PLIST_INT.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_int(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy an unsigned integer value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The unsigned integer value from *source_dict* is retrieved with #plist_dict_get_uint,
     *     but is **always** created as #PLIST_INT.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_uint(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a #PLIST_DATA node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note This function is like #plist_dict_copy_item, except that it fails
     *     if the source node is not of type #PLIST_DATA.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key, or if it is not of type #PLIST_DATA.
     */
    PLIST_API plist_err_t plist_dict_copy_data(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a #PLIST_STRING node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note This function is like #plist_dict_copy_item, except that it fails
     *     if the source node is not of type #PLIST_STRING.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key, or if it is not of type #PLIST_STRING.
     */
    PLIST_API plist_err_t plist_dict_copy_string(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /********************************************
     *                                          *
     *                Getters                   *
     *                                          *
     ********************************************/

    /**
     * Get the parent of a node
     *
     * @param node the parent (NULL if node is root)
     */
    PLIST_API plist_t plist_get_parent(plist_t node);

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
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_key_val(plist_t node, char **val);

    /**
     * Get the value of a #PLIST_STRING node.
     * This function does nothing if node is not of type #PLIST_STRING
     *
     * @param node the node
     * @param val a pointer to a C-string. This function allocates the memory,
     *            caller is responsible for freeing it. Data is UTF-8 encoded.
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_string_val(plist_t node, char **val);

    /**
     * Get a pointer to the buffer of a #PLIST_STRING node.
     *
     * @note DO NOT MODIFY the buffer. Mind that the buffer is only available
     *   until the plist node gets freed. Make a copy if needed.
     *
     * @param node The node
     * @param length If non-NULL, will be set to the length of the string
     *
     * @return Pointer to the NULL-terminated buffer.
     */
    PLIST_API const char* plist_get_string_ptr(plist_t node, uint64_t* length);

    /**
     * Get the value of a #PLIST_BOOLEAN node.
     * This function does nothing if node is not of type #PLIST_BOOLEAN
     *
     * @param node the node
     * @param val a pointer to a uint8_t variable.
     */
    PLIST_API void plist_get_bool_val(plist_t node, uint8_t * val);

    /**
     * Get the unsigned integer value of a #PLIST_INT node.
     * This function does nothing if node is not of type #PLIST_INT
     *
     * @param node the node
     * @param val a pointer to a uint64_t variable.
     */
    PLIST_API void plist_get_uint_val(plist_t node, uint64_t * val);

    /**
     * Get the signed integer value of a #PLIST_INT node.
     * This function does nothing if node is not of type #PLIST_INT
     *
     * @param node the node
     * @param val a pointer to a int64_t variable.
     */
    PLIST_API void plist_get_int_val(plist_t node, int64_t * val);

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
     * @param length the length of the buffer
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_data_val(plist_t node, char **val, uint64_t * length);

    /**
     * Get a pointer to the data buffer of a #PLIST_DATA node.
     *
     * @note DO NOT MODIFY the buffer. Mind that the buffer is only available
     *   until the plist node gets freed. Make a copy if needed.
     *
     * @param node The node
     * @param length Pointer to a uint64_t that will be set to the length of the buffer
     *
     * @return Pointer to the buffer
     */
    PLIST_API const char* plist_get_data_ptr(plist_t node, uint64_t* length);

    /**
     * Get the value of a #PLIST_DATE node.
     * This function does nothing if node is not of type #PLIST_DATE
     *
     * @param node the node
     * @param sec a pointer to an int32_t variable. Represents the number of seconds since 01/01/2001.
     * @param usec a pointer to an int32_t variable. Represents the number of microseconds
     */
    PLIST_API void plist_get_date_val(plist_t node, int32_t * sec, int32_t * usec);

    /**
     * Get the value of a #PLIST_UID node.
     * This function does nothing if node is not of type #PLIST_UID
     *
     * @param node the node
     * @param val a pointer to a uint64_t variable.
     */
    PLIST_API void plist_get_uid_val(plist_t node, uint64_t * val);


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
     * @param val the string value. The string is copied when set and will be
     *		freed by the node.
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
     * Forces type of node to #PLIST_INT
     *
     * @param node the node
     * @param val the unsigned integer value
     */
    PLIST_API void plist_set_uint_val(plist_t node, uint64_t val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_INT
     *
     * @param node the node
     * @param val the signed integer value
     */
    PLIST_API void plist_set_int_val(plist_t node, int64_t val);

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
     * @param val the binary buffer. The buffer is copied when set and will
     *		be freed by the node.
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

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_UID
     *
     * @param node the node
     * @param val the unsigned integer value
     */
    PLIST_API void plist_set_uid_val(plist_t node, uint64_t val);


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
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length);

    /**
     * Export the #plist_t structure to binary format.
     *
     * @param plist the root node to export
     * @param plist_bin a pointer to a char* buffer. This function allocates the memory,
     *            caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length);

    /**
     * Export the #plist_t structure to JSON format.
     *
     * @param plist the root node to export
     * @param plist_json a pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @param prettify pretty print the output if != 0
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_json(plist_t plist, char **plist_json, uint32_t* length, int prettify);

    /**
     * Export the #plist_t structure to OpenStep format.
     *
     * @param plist the root node to export
     * @param plist_openstep a pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @param prettify pretty print the output if != 0
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_openstep(plist_t plist, char **plist_openstep, uint32_t* length, int prettify);


    /**
     * Import the #plist_t structure from XML format.
     *
     * @param plist_xml a pointer to the xml buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from binary format.
     *
     * @param plist_bin a pointer to the xml buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from JSON format.
     *
     * @param json a pointer to the JSON buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_json(const char *json, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from OpenStep plist format.
     *
     * @param openstep a pointer to the OpenStep plist buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_openstep(const char *openstep, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from memory data.
     *
     * This function will look at the first bytes of plist_data
     * to determine if plist_data contains a binary, JSON, OpenStep, or XML plist
     * and tries to parse the data in the appropriate format.
     * @note This is just a convenience function and the format detection is
     *     very basic. It checks with plist_is_binary() if the data supposedly
     *     contains binary plist data, if not it checks if the first bytes have
     *     either '{' or '[' and assumes JSON format, and XML tags will result
     *     in parsing as XML, otherwise it will try to parse as OpenStep.
     *
     * @param plist_data A pointer to the memory buffer containing plist data.
     * @param length Length of the buffer to read.
     * @param plist A pointer to the imported plist.
     * @param format If non-NULL, the #plist_format_t value pointed to will be set to the parsed format.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_memory(const char *plist_data, uint32_t length, plist_t *plist, plist_format_t *format);

    /**
     * Import the #plist_t structure directly from file.
     *
     * This function will look at the first bytes of the file data
     * to determine if it contains a binary, JSON, OpenStep, or XML plist
     * and tries to parse the data in the appropriate format.
     * Uses plist_from_memory() internally.
     *
     * @param filename The name of the file to parse.
     * @param plist A pointer to the imported plist.
     * @param format If non-NULL, the #plist_format_t value pointed to will be set to the parsed format.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_read_from_file(const char *filename, plist_t *plist, plist_format_t *format);

    /**
     * Write the #plist_t structure to a NULL-terminated string using the given format and options.
     *
     * @param plist The input plist structure
     * @param output Pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length A pointer to a uint32_t value that will receive the lenght of the allocated buffer.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note Use plist_mem_free() to free the allocated memory.
     * @note #PLIST_FORMAT_BINARY is not supported by this function.
     */
    PLIST_API plist_err_t plist_write_to_string(plist_t plist, char **output, uint32_t* length, plist_format_t format, plist_write_options_t options);

    /**
     * Write the #plist_t structure to a FILE* stream using the given format and options.
     *
     * @param plist The input plist structure
     * @param stream A writeable FILE* stream that the data will be written to.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note While this function allows all formats to be written to the given stream,
     *     only the formats #PLIST_FORMAT_PRINT, #PLIST_FORMAT_LIMD, and #PLIST_FORMAT_PLUTIL
     *     (basically all output-only formats) are directly and efficiently written to the stream;
     *     the other formats are written to a memory buffer first.
     */
    PLIST_API plist_err_t plist_write_to_stream(plist_t plist, FILE* stream, plist_format_t format, plist_write_options_t options);

    /**
     * Write the #plist_t structure to a file at given path using the given format and options.
     *
     * @param plist The input plist structure
     * @param filename The file name of the file to write to. Existing files will be overwritten.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_write_to_file(plist_t plist, const char *filename, plist_format_t format, plist_write_options_t options);

    /**
     * Print the given plist in human-readable format to standard output.
     * This is equivalent to
     * <code>plist_write_to_stream(plist, stdout, PLIST_FORMAT_PRINT, PLIST_OPT_PARTIAL_DATA);</code>
     * @param plist The #plist_t structure to print
     * @note For #PLIST_DATA nodes, only a maximum of 24 bytes (first 16 and last 8) are written.
     */
    PLIST_API void plist_print(plist_t plist);

    /**
     * Test if in-memory plist data is in binary format.
     * This function will look at the first bytes of plist_data to determine
     * if it supposedly contains a binary plist.
     * @note The function is not validating the whole memory buffer to check
     * if the content is truly a plist, it is only using some heuristic on
     * the first few bytes of plist_data.
     *
     * @param plist_data a pointer to the memory buffer containing plist data.
     * @param length length of the buffer to read.
     * @return 1 if the buffer is a binary plist, 0 otherwise.
     */
    PLIST_API int plist_is_binary(const char *plist_data, uint32_t length);

    /********************************************
     *                                          *
     *                 Utils                    *
     *                                          *
     ********************************************/

    /**
     * Get a node from its path. Each path element depends on the associated father node type.
     * For Dictionaries, var args are casted to const char*, for arrays, var args are caster to uint32_t
     * Search is breath first order.
     *
     * @param plist the node to access result from.
     * @param length length of the path to access
     * @return the value to access.
     */
    PLIST_API plist_t plist_access_path(plist_t plist, uint32_t length, ...);

    /**
     * Variadic version of #plist_access_path.
     *
     * @param plist the node to access result from.
     * @param length length of the path to access
     * @param v list of array's index and dic'st key
     * @return the value to access.
     */
    PLIST_API plist_t plist_access_pathv(plist_t plist, uint32_t length, va_list v);

    /**
     * Compare two node values
     *
     * @param node_l left node to compare
     * @param node_r rigth node to compare
     * @return TRUE is type and value match, FALSE otherwise.
     */
    PLIST_API char plist_compare_node_value(plist_t node_l, plist_t node_r);

    /** Helper macro used by PLIST_IS_* macros that will evaluate the type of a plist node. */
    #define _PLIST_IS_TYPE(__plist, __plist_type) (__plist && (plist_get_node_type(__plist) == PLIST_##__plist_type))

    /* Helper macros for the different plist types */
    /** Evaluates to true if the given plist node is of type PLIST_BOOLEAN */
    #define PLIST_IS_BOOLEAN(__plist) _PLIST_IS_TYPE(__plist, BOOLEAN)
    /** Evaluates to true if the given plist node is of type PLIST_INT */
    #define PLIST_IS_INT(__plist)     _PLIST_IS_TYPE(__plist, INT)
    /** Evaluates to true if the given plist node is of type PLIST_REAL */
    #define PLIST_IS_REAL(__plist)    _PLIST_IS_TYPE(__plist, REAL)
    /** Evaluates to true if the given plist node is of type PLIST_STRING */
    #define PLIST_IS_STRING(__plist)  _PLIST_IS_TYPE(__plist, STRING)
    /** Evaluates to true if the given plist node is of type PLIST_ARRAY */
    #define PLIST_IS_ARRAY(__plist)   _PLIST_IS_TYPE(__plist, ARRAY)
    /** Evaluates to true if the given plist node is of type PLIST_DICT */
    #define PLIST_IS_DICT(__plist)    _PLIST_IS_TYPE(__plist, DICT)
    /** Evaluates to true if the given plist node is of type PLIST_DATE */
    #define PLIST_IS_DATE(__plist)    _PLIST_IS_TYPE(__plist, DATE)
    /** Evaluates to true if the given plist node is of type PLIST_DATA */
    #define PLIST_IS_DATA(__plist)    _PLIST_IS_TYPE(__plist, DATA)
    /** Evaluates to true if the given plist node is of type PLIST_KEY */
    #define PLIST_IS_KEY(__plist)     _PLIST_IS_TYPE(__plist, KEY)
    /** Evaluates to true if the given plist node is of type PLIST_UID */
    #define PLIST_IS_UID(__plist)     _PLIST_IS_TYPE(__plist, UID)
    /* for backwards compatibility */
    #define PLIST_IS_UINT             PLIST_IS_INT

    /**
     * Helper function to check the value of a PLIST_BOOL node.
     *
     * @param boolnode node of type PLIST_BOOL
     * @return 1 if the boolean node has a value of TRUE or 0 if FALSE.
     */
    PLIST_API int plist_bool_val_is_true(plist_t boolnode);

    /**
     * Helper function to test if a given #PLIST_INT node's value is negative
     *
     * @param intnode node of type PLIST_INT
     * @return 1 if the node's value is negative, or 0 if positive.
     */
    PLIST_API int plist_int_val_is_negative(plist_t intnode);

    /**
     * Helper function to compare the value of a PLIST_INT node against
     * a given signed integer value.
     *
     * @param uintnode node of type PLIST_INT
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_int_val_compare(plist_t uintnode, int64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_INT node against
     * a given unsigned integer value.
     *
     * @param uintnode node of type PLIST_INT
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_uint_val_compare(plist_t uintnode, uint64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_UID node against
     * a given value.
     *
     * @param uidnode node of type PLIST_UID
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_uid_val_compare(plist_t uidnode, uint64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_REAL node against
     * a given value.
     *
     * @note WARNING: Comparing floating point values can give inaccurate
     *     results because of the nature of floating point values on computer
     *     systems. While this function is designed to be as accurate as
     *     possible, please don't rely on it too much.
     *
     * @param realnode node of type PLIST_REAL
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are (almost) equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_real_val_compare(plist_t realnode, double cmpval);

    /**
     * Helper function to compare the value of a PLIST_DATE node against
     * a given set of seconds and fraction of a second since epoch.
     *
     * @param datenode node of type PLIST_DATE
     * @param cmpsec number of seconds since epoch to compare against
     * @param cmpusec fraction of a second in microseconds to compare against
     * @return 0 if the node's date is equal to the supplied values,
     *         1 if the node's date is greater than the supplied values,
     *         or -1 if the node's date is less than the supplied values.
     */
    PLIST_API int plist_date_val_compare(plist_t datenode, int32_t cmpsec, int32_t cmpusec);

    /**
     * Helper function to compare the value of a PLIST_STRING node against
     * a given value.
     * This function basically behaves like strcmp.
     *
     * @param strnode node of type PLIST_STRING
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_string_val_compare(plist_t strnode, const char* cmpval);

    /**
     * Helper function to compare the value of a PLIST_STRING node against
     * a given value, while not comparing more than n characters.
     * This function basically behaves like strncmp.
     *
     * @param strnode node of type PLIST_STRING
     * @param cmpval value to compare against
     * @param n maximum number of characters to compare
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_string_val_compare_with_size(plist_t strnode, const char* cmpval, size_t n);

    /**
     * Helper function to match a given substring in the value of a
     * PLIST_STRING node.
     *
     * @param strnode node of type PLIST_STRING
     * @param substr value to match
     * @return 1 if the node's value contains the given substring,
     *     or 0 if not.
     */
    PLIST_API int plist_string_val_contains(plist_t strnode, const char* substr);

    /**
     * Helper function to compare the value of a PLIST_KEY node against
     * a given value.
     * This function basically behaves like strcmp.
     *
     * @param keynode node of type PLIST_KEY
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_key_val_compare(plist_t keynode, const char* cmpval);

    /**
     * Helper function to compare the value of a PLIST_KEY node against
     * a given value, while not comparing more than n characters.
     * This function basically behaves like strncmp.
     *
     * @param keynode node of type PLIST_KEY
     * @param cmpval value to compare against
     * @param n maximum number of characters to compare
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_key_val_compare_with_size(plist_t keynode, const char* cmpval, size_t n);

    /**
     * Helper function to match a given substring in the value of a
     * PLIST_KEY node.
     *
     * @param keynode node of type PLIST_KEY
     * @param substr value to match
     * @return 1 if the node's value contains the given substring,
     *     or 0 if not.
     */
    PLIST_API int plist_key_val_contains(plist_t keynode, const char* substr);

    /**
     * Helper function to compare the data of a PLIST_DATA node against
     * a given blob and size.
     * This function basically behaves like memcmp after making sure the
     * size of the node's data value is equal to the size of cmpval (n),
     * making this a "full match" comparison.
     *
     * @param datanode node of type PLIST_DATA
     * @param cmpval data blob to compare against
     * @param n size of data blob passed in cmpval
     * @return 0 if the node's data blob and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_data_val_compare(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Helper function to compare the data of a PLIST_DATA node against
     * a given blob and size, while no more than n bytes are compared.
     * This function basically behaves like memcmp after making sure the
     * size of the node's data value is at least n, making this a
     * "starts with" comparison.
     *
     * @param datanode node of type PLIST_DATA
     * @param cmpval data blob to compare against
     * @param n size of data blob passed in cmpval
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_data_val_compare_with_size(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Helper function to match a given data blob within the value of a
     * PLIST_DATA node.
     *
     * @param datanode node of type PLIST_KEY
     * @param cmpval data blob to match
     * @param n size of data blob passed in cmpval
     * @return 1 if the node's value contains the given data blob
     *     or 0 if not.
     */
    PLIST_API int plist_data_val_contains(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Sort all PLIST_DICT key/value pairs in a property list lexicographically
     * by key. Recurses into the child nodes if necessary.
     *
     * @param plist The property list to perform the sorting operation on.
     */
    PLIST_API void plist_sort(plist_t plist);

    /**
     * Free memory allocated by relevant libplist API calls:
     * - plist_to_xml()
     * - plist_to_bin()
     * - plist_get_key_val()
     * - plist_get_string_val()
     * - plist_get_data_val()
     *
     * @param ptr pointer to the memory to free
     *
     * @note Do not use this function to free plist_t nodes, use plist_free()
     *     instead.
     */
    PLIST_API void plist_mem_free(void* ptr);

    /**
     * Set debug level for the format parsers.
     * @note This function does nothing if libplist was not configured with --enable-debug .
     *
     * @param debug Debug level. Currently, only 0 (off) and 1 (enabled) are supported.
     */
    PLIST_API void plist_set_debug(int debug);

    /**
     * Returns a static string of the libplist version.
     *
     * @return The libplist version as static ascii string
     */
    PLIST_API const char* libplist_version();

    /*@}*/

#ifdef __cplusplus
}
#endif
#endif
