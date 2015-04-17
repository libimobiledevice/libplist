/*
 * plist.c
 * XML plist implementation
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


#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <inttypes.h>
#include <locale.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <node.h>
#include <node_list.h>
#include <node_iterator.h>

#include "plist.h"
#include "base64.h"

#define XPLIST_TEXT	BAD_CAST("text")
#define XPLIST_KEY	BAD_CAST("key")
#define XPLIST_FALSE	BAD_CAST("false")
#define XPLIST_TRUE	BAD_CAST("true")
#define XPLIST_INT	BAD_CAST("integer")
#define XPLIST_REAL	BAD_CAST("real")
#define XPLIST_DATE	BAD_CAST("date")
#define XPLIST_DATA	BAD_CAST("data")
#define XPLIST_STRING	BAD_CAST("string")
#define XPLIST_ARRAY	BAD_CAST("array")
#define XPLIST_DICT	BAD_CAST("dict")

#define MAC_EPOCH 978307200

static const char *plist_base = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
</plist>\0";


/** Formats a block of text to be a given indentation and width.
 *
 * The total width of the return string will be depth + cols.
 *
 * @param buf The string to format.
 * @param cols The number of text columns for returned block of text.
 * @param depth The number of tabs to indent the returned block of text.
 *
 * @return The formatted string.
 */
static char *format_string(const char *buf, size_t len, int cols, int depth)
{
    if (!buf || !(len > 0)) return NULL;
    int colw = depth + cols + 1;
    int nlines = len / cols + 1;
    char *new_buf = NULL;
    int i = 0;
    int j = 0;

    assert(cols >= 0);
    assert(depth >= 0);

    new_buf = (char*) malloc(nlines * colw + depth + 1);
    assert(new_buf != 0);
    memset(new_buf, 0, nlines * colw + depth + 1);

    // Inserts new lines and tabs at appropriate locations
    for (i = 0; i < nlines; i++)
    {
        new_buf[i * colw] = '\n';
        for (j = 0; j < depth; j++)
            new_buf[i * colw + 1 + j] = '\t';
        memcpy(new_buf + i * colw + 1 + depth, buf + i * cols, (size_t)(i + 1) * cols <= len ? (size_t)cols : len - i * cols);
    }
    new_buf[len + (1 + depth) * nlines] = '\n';

    // Inserts final row of indentation and termination character
    for (j = 0; j < depth; j++)
        new_buf[len + (1 + depth) * nlines + 1 + j] = '\t';
    new_buf[len + (1 + depth) * nlines + depth + 1] = '\0';

    return new_buf;
}



struct xml_node
{
    xmlNodePtr xml;
    uint32_t depth;
};

/** Creates a new plist XML document.
 *
 * @return The plist XML document.
 */
static xmlDocPtr new_xml_plist(void)
{
    char *plist = strdup(plist_base);
    xmlDocPtr plist_xml = xmlParseMemory(plist, strlen(plist));

    free(plist);

    return plist_xml;
}

static struct node_t* new_key_node(const char* name)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_KEY;
    int size = strlen(name);
    data->strval = strdup(name);
    data->length = size;
    return node_create(NULL, data);
}

static struct node_t* new_uint_node(uint64_t value)
{
    plist_data_t data = plist_new_plist_data();
    data->type = PLIST_UINT;
    data->intval = value;
    data->length = sizeof(uint64_t);
    return node_create(NULL, data);
}

static void node_to_xml(node_t* node, void *xml_struct)
{
    struct xml_node *xstruct = NULL;
    plist_data_t node_data = NULL;

    xmlNodePtr child_node = NULL;
    char isStruct = FALSE;
    char isUIDNode = FALSE;

    const xmlChar *tag = NULL;
    char *val = NULL;

    //for base64
    char *valtmp = NULL;

    uint32_t i = 0;

    if (!node)
        return;

    xstruct = (struct xml_node *) xml_struct;
    node_data = plist_get_data(node);

    switch (node_data->type)
    {
    case PLIST_BOOLEAN:
    {
        if (node_data->boolval)
            tag = XPLIST_TRUE;
        else
            tag = XPLIST_FALSE;
    }
    break;

    case PLIST_UINT:
        tag = XPLIST_INT;
        val = (char*)malloc(64);
        if (node_data->length == 16) {
	        (void)snprintf(val, 64, "%"PRIu64, node_data->intval);
	} else {
	        (void)snprintf(val, 64, "%"PRIi64, node_data->intval);
	}
        break;

    case PLIST_REAL:
        tag = XPLIST_REAL;
        val = (char*)malloc(64);
        (void)snprintf(val, 64, "%f", node_data->realval);
        break;

    case PLIST_STRING:
        tag = XPLIST_STRING;
        val = strdup((char*) node_data->strval);
        break;

    case PLIST_KEY:
        tag = XPLIST_KEY;
        val = strdup((char*) node_data->strval);
        break;

    case PLIST_DATA:
        tag = XPLIST_DATA;
        if (node_data->length)
        {
            size_t len = node_data->length;
            valtmp = base64encode(node_data->buff, &len);
            val = format_string(valtmp, len, 68, xstruct->depth);
            free(valtmp);
        }
        break;
    case PLIST_ARRAY:
        tag = XPLIST_ARRAY;
        isStruct = TRUE;
        break;
    case PLIST_DICT:
        tag = XPLIST_DICT;
        isStruct = TRUE;
        break;
    case PLIST_DATE:
        tag = XPLIST_DATE;
        {
            time_t timev = (time_t)node_data->timeval.tv_sec + MAC_EPOCH;
            struct tm *btime = gmtime(&timev);
            if (btime) {
                val = (char*)malloc(24);
                memset(val, 0, 24);
                if (strftime(val, 24, "%Y-%m-%dT%H:%M:%SZ", btime) <= 0) {
                    free (val);
                    val = NULL;
                }
            }
        }
        break;
    case PLIST_UID:
        // special case for keyed encoding
        tag = XPLIST_DICT;
        isStruct = TRUE;
        isUIDNode = TRUE;
        node_data->type = PLIST_DICT;
        node_attach(node, new_key_node("CF$UID"));
        node_attach(node, new_uint_node(node_data->intval));
        break;
    default:
        break;
    }

    for (i = 0; i < xstruct->depth; i++)
    {
        xmlNodeAddContent(xstruct->xml, BAD_CAST("\t"));
    }
    if (node_data->type == PLIST_STRING || node_data->type == PLIST_KEY) {
        /* make sure we convert the following predefined xml entities */
        /* < = &lt; > = &gt; ' = &apos; " = &quot; & = &amp; */
        child_node = xmlNewTextChild(xstruct->xml, NULL, tag, BAD_CAST(val));
    } else
        child_node = xmlNewChild(xstruct->xml, NULL, tag, BAD_CAST(val));
    xmlNodeAddContent(xstruct->xml, BAD_CAST("\n"));
    if (val) {
        free(val);
    }

    //add return for structured types
    if (node_data->type == PLIST_ARRAY || node_data->type == PLIST_DICT)
        xmlNodeAddContent(child_node, BAD_CAST("\n"));

    //make sure we don't produce <data/> if it's empty
    if ((node_data->type == PLIST_DATA) && !val) {
        xmlNodeAddContent(child_node, BAD_CAST("\n"));
        for (i = 0; i < xstruct->depth; i++)
        {
            xmlNodeAddContent(child_node, BAD_CAST("\t"));
        }
    }

    if (isStruct)
    {
        struct xml_node child = { child_node, xstruct->depth + 1 };
        node_iterator_t *ni = node_iterator_create(node->children);
        node_t *ch;
        while ((ch = node_iterator_next(ni))) {
            node_to_xml(ch, &child);
        }
        node_iterator_destroy(ni);
    }
    //fix indent for structured types
    if (node_data->type == PLIST_ARRAY || node_data->type == PLIST_DICT)
    {

        for (i = 0; i < xstruct->depth; i++)
        {
            xmlNodeAddContent(child_node, BAD_CAST("\t"));
        }
    }
    if (isUIDNode)
    {
        unsigned int num = node_n_children(node);
        unsigned int j;
        for (j = num; j > 0; j--) {
            node_t* ch = node_nth_child(node, j-1);
            node_detach(node, ch);
            node_destroy(ch);
        }
        node_data->type = PLIST_UID;
    }

    return;
}

static void parse_date(const char *strval, struct tm *btime)
{
    if (!btime) return;
    memset(btime, 0, sizeof(struct tm));
    if (!strval) return;
#ifdef strptime
    strptime((char*)strval, "%Y-%m-%dT%H:%M:%SZ", btime);
#else
    sscanf(strval, "%d-%d-%dT%d:%d:%dZ", &btime->tm_year, &btime->tm_mon, &btime->tm_mday, &btime->tm_hour, &btime->tm_min, &btime->tm_sec);
    btime->tm_year-=1900;
    btime->tm_mon--;
#endif
    btime->tm_isdst=0;
}

static void xml_to_node(xmlNodePtr xml_node, plist_t * plist_node)
{
    xmlNodePtr node = NULL;
    plist_data_t data = NULL;
    plist_t subnode = NULL;

    //for string
    long len = 0;
    int type = 0;

    if (!xml_node)
        return;

    for (node = xml_node->children; node; node = node->next)
    {

        while (node && !xmlStrcmp(node->name, XPLIST_TEXT))
            node = node->next;
        if (!node)
            break;

        if (!xmlStrcmp(node->name, BAD_CAST("comment"))) {
            continue;
        }

        data = plist_new_plist_data();
        subnode = plist_new_node(data);
        if (*plist_node)
			node_attach((node_t*)*plist_node, (node_t*)subnode);
        else
            *plist_node = subnode;

        if (!xmlStrcmp(node->name, XPLIST_TRUE))
        {
            data->boolval = TRUE;
            data->type = PLIST_BOOLEAN;
            data->length = 1;
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_FALSE))
        {
            data->boolval = FALSE;
            data->type = PLIST_BOOLEAN;
            data->length = 1;
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_INT))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            int is_negative = 0;
            char *str = (char*)strval;
            if ((str[0] == '-') || (str[0] == '+')) {
                if (str[0] == '-') {
                    is_negative = 1;
                }
                str++;
            }
            char* endp = NULL;
            data->intval = strtoull((char*)str, &endp, 0);
            if ((endp != NULL) && (strlen(endp) > 0)) {
                fprintf(stderr, "%s: integer parse error: string contains invalid characters: '%s'\n", __func__, endp);
            }
            if (is_negative || (data->intval <= INT64_MAX)) {
                int64_t v = data->intval;
                if (is_negative) {
                    v = -v;
                }
                data->intval = (uint64_t)v;
                data->length = 8;
            } else {
                data->length = 16;
            }
            data->type = PLIST_UINT;
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_REAL))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            data->realval = atof((char *) strval);
            data->type = PLIST_REAL;
            data->length = 8;
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_DATE))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            time_t timev = 0;
            if (strlen((const char*)strval) >= 11) {
                struct tm btime;
                struct tm* tm_utc;
                parse_date((const char*)strval, &btime);
                timev = mktime(&btime);
                tm_utc = gmtime(&timev);
                timev -= (mktime(tm_utc) - timev);
            }
            data->timeval.tv_sec = (long)(timev - MAC_EPOCH);
            data->timeval.tv_usec = 0;
            data->type = PLIST_DATE;
            data->length = sizeof(struct timeval);
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_STRING))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            len = strlen((char *) strval);
            type = xmlDetectCharEncoding(strval, len);

            if (XML_CHAR_ENCODING_UTF8 == type || XML_CHAR_ENCODING_ASCII == type || XML_CHAR_ENCODING_NONE == type)
            {
                data->strval = strdup((char *) strval);
                data->type = PLIST_STRING;
                data->length = strlen(data->strval);
            }
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_KEY))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            len = strlen((char *) strval);
            type = xmlDetectCharEncoding(strval, len);

            if (XML_CHAR_ENCODING_UTF8 == type || XML_CHAR_ENCODING_ASCII == type || XML_CHAR_ENCODING_NONE == type)
            {
                data->strval = strdup((char *) strval);
                data->type = PLIST_KEY;
                data->length = strlen(data->strval);
            }
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_DATA))
        {
            xmlChar *strval = xmlNodeGetContent(node);
            size_t size = 0;
            unsigned char *dec = base64decode((char*)strval, &size);
            data->buff = (uint8_t *) malloc(size * sizeof(uint8_t));
            memcpy(data->buff, dec, size * sizeof(uint8_t));
            free(dec);
            data->length = size;
            data->type = PLIST_DATA;
            xmlFree(strval);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_ARRAY))
        {
            data->type = PLIST_ARRAY;
            xml_to_node(node, &subnode);
            continue;
        }

        if (!xmlStrcmp(node->name, XPLIST_DICT))
        {
            data->type = PLIST_DICT;
            xml_to_node(node, &subnode);
            if (plist_get_node_type(subnode) == PLIST_DICT) {
                if (plist_dict_get_size(subnode) == 1) {
                    plist_t uid = plist_dict_get_item(subnode, "CF$UID");
                    if (uid) {
                        uint64_t val = 0;
                        plist_get_uint_val(uid, &val);
                        plist_dict_remove_item(subnode, "CF$UID");
                        plist_data_t nodedata = plist_get_data((node_t*)subnode);
                        free(nodedata->buff);
                        nodedata->type = PLIST_UID;
                        nodedata->length = sizeof(uint64_t);
                        nodedata->intval = val;
                    } 
                }
            }
            continue;
        }
    }
}

PLIST_API void plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length)
{
    xmlDocPtr plist_doc = NULL;
    xmlNodePtr root_node = NULL;
    struct xml_node root = { NULL, 0 };
    int size = 0;

    if (!plist || !plist_xml || *plist_xml)
        return;
    plist_doc = new_xml_plist();
    root_node = xmlDocGetRootElement(plist_doc);
    root.xml = root_node;

    char *current_locale = setlocale(LC_NUMERIC, NULL);
    char *saved_locale = NULL;
    if (current_locale) {
        saved_locale = strdup(current_locale);
    }
    if (saved_locale) {
        setlocale(LC_NUMERIC, "POSIX");
    }
	node_to_xml((node_t*)plist, &root);

    xmlChar* tmp = NULL;
    xmlDocDumpMemory(plist_doc, &tmp, &size);
    if (size >= 0 && tmp)
    {
	/* make sure to copy the terminating 0-byte */
        *plist_xml = (char*)malloc((size+1) * sizeof(char));
	memcpy(*plist_xml, tmp, size+1);
        *length = size;
	xmlFree(tmp);
	tmp = NULL;
    }
    xmlFreeDoc(plist_doc);

    if (saved_locale) {
        setlocale(LC_NUMERIC, saved_locale);
        free(saved_locale);
    }
}

PLIST_API void plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist)
{
    xmlDocPtr plist_doc = xmlParseMemory(plist_xml, length);
    xmlNodePtr root_node = xmlDocGetRootElement(plist_doc);

    xml_to_node(root_node, plist);
    xmlFreeDoc(plist_doc);
}
