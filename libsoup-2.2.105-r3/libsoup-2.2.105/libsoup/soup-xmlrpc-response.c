/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-xmlrpc-response.c: XMLRPC response message
 *
 * Copyright (C) 2003, Novell, Inc.
 * Copyright (C) 2004, Mariano Suarez-Alvarez <mariano@gnome.org>
 * Copyright (C) 2004, Fernando Herrera  <fherrera@onirica.com>
 * Copyright (C) 2005, Jeff Bailey  <jbailey@ubuntu.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib.h>
#include <libxml/tree.h>

#include "soup-date.h"
#include "soup-misc.h"
#include "soup-xmlrpc-response.h"


G_DEFINE_TYPE (SoupXmlrpcResponse, soup_xmlrpc_response, G_TYPE_OBJECT)

typedef struct {
	xmlDocPtr doc;
	gboolean fault;
	xmlNodePtr value;
} SoupXmlrpcResponsePrivate;
#define SOUP_XMLRPC_RESPONSE_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_XMLRPC_RESPONSE, SoupXmlrpcResponsePrivate))

static void
finalize (GObject *object)
{
	SoupXmlrpcResponsePrivate *priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (object);

	if (priv->doc)
		xmlFreeDoc (priv->doc);

	G_OBJECT_CLASS (soup_xmlrpc_response_parent_class)->finalize (object);
}

static void
soup_xmlrpc_response_class_init (SoupXmlrpcResponseClass *soup_xmlrpc_response_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (soup_xmlrpc_response_class);

	g_type_class_add_private (soup_xmlrpc_response_class, sizeof (SoupXmlrpcResponsePrivate));

	object_class->finalize = finalize;
}

static void
soup_xmlrpc_response_init (SoupXmlrpcResponse *response)
{
	SoupXmlrpcResponsePrivate *priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (response);

	priv->doc = xmlNewDoc ((const xmlChar *)"1.0");
	priv->fault = FALSE;
}


SoupXmlrpcResponse *
soup_xmlrpc_response_new (void)
{
	SoupXmlrpcResponse *response;

	response = g_object_new (SOUP_TYPE_XMLRPC_RESPONSE, NULL);
	return response;
}

SoupXmlrpcResponse *
soup_xmlrpc_response_new_from_string (const char *xmlstr)
{
	SoupXmlrpcResponse *response;

	g_return_val_if_fail (xmlstr != NULL, NULL);

	response = g_object_new (SOUP_TYPE_XMLRPC_RESPONSE, NULL);

	if (!soup_xmlrpc_response_from_string (response, xmlstr)) {
		g_object_unref (response);
		return NULL;
	}

	return response;
}

static xmlNode *
exactly_one_child (xmlNode *node)
{
	xmlNode *child;

	child = soup_xml_real_node (node->children);
	if (!child || soup_xml_real_node (child->next))
		return NULL;

	return child;
}

gboolean
soup_xmlrpc_response_from_string (SoupXmlrpcResponse *response, const char *xmlstr)
{
	SoupXmlrpcResponsePrivate *priv;
	xmlDocPtr newdoc;
	xmlNodePtr body;
	gboolean fault = TRUE;

	g_return_val_if_fail (SOUP_IS_XMLRPC_RESPONSE (response), FALSE);
	priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (response);
	g_return_val_if_fail (xmlstr != NULL, FALSE);

	newdoc = xmlParseMemory (xmlstr, strlen (xmlstr));
	if (!newdoc)
		goto very_bad;

	body = xmlDocGetRootElement (newdoc);
	if (!body || strcmp ((const char *)body->name, "methodResponse"))
		goto bad;

	body = exactly_one_child (body);
	if (!body)
		goto bad;

	if (strcmp ((const char *)body->name, "params") == 0) {
		fault = FALSE;
		body = exactly_one_child (body);
		if (!body || strcmp ((const char *)body->name, "param"))
			goto bad;
	} else if (strcmp ((const char *)body->name, "fault") != 0)
		goto bad;

	body = exactly_one_child (body);
	if (!body || strcmp ((const char *)body->name, "value"))
		goto bad;

	/* body should be pointing by now to the struct of a fault, or the value of a
	 * normal response
	 */

	xmlFreeDoc (priv->doc);
	priv->doc = newdoc;
	priv->value = body;
	priv->fault = fault;

	return TRUE;

bad:
	xmlFreeDoc (newdoc);
very_bad:
	return FALSE;
}

xmlChar *
soup_xmlrpc_response_to_string (SoupXmlrpcResponse *response)
{
	SoupXmlrpcResponsePrivate *priv;
	xmlChar *str;
	int size;

	g_return_val_if_fail (SOUP_IS_XMLRPC_RESPONSE (response), FALSE);
	priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (response);

	xmlDocDumpMemoryEnc (priv->doc, &str, &size, "UTF-8");

	return str;
}

gboolean
soup_xmlrpc_response_is_fault (SoupXmlrpcResponse *response)
{
	SoupXmlrpcResponsePrivate *priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (response);

	return priv->fault;
}

SoupXmlrpcValue *
soup_xmlrpc_response_get_value (SoupXmlrpcResponse *response)
{
	SoupXmlrpcResponsePrivate *priv;
	g_return_val_if_fail (SOUP_IS_XMLRPC_RESPONSE (response), FALSE);
	priv = SOUP_XMLRPC_RESPONSE_GET_PRIVATE (response);

	return (SoupXmlrpcValue*) priv->value;
}

SoupXmlrpcValueType
soup_xmlrpc_value_get_type (SoupXmlrpcValue *value)
{
	xmlNode *xml;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return SOUP_XMLRPC_VALUE_TYPE_BAD;

	xml = exactly_one_child (xml);
	if (!xml)
		return SOUP_XMLRPC_VALUE_TYPE_BAD;

	if (xml->type == XML_TEXT_NODE)
		return SOUP_XMLRPC_VALUE_TYPE_STRING;
	else if (xml->type != XML_ELEMENT_NODE)
		return SOUP_XMLRPC_VALUE_TYPE_BAD;
	
	if (strcmp ((const char *)xml->name, "i4") == 0 || strcmp ((const char *)xml->name, "int") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_INT;
	else if (strcmp ((const char *)xml->name, "boolean") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_BOOLEAN;
	else if (strcmp ((const char *)xml->name, "string") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_STRING;
	else if (strcmp ((const char *)xml->name, "double") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_DOUBLE;
	else if (strcmp ((const char *)xml->name, "dateTime.iso8601") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_DATETIME;
	else if (strcmp ((const char *)xml->name, "base64") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_BASE64;
	else if (strcmp ((const char *)xml->name, "struct") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_STRUCT;
	else if (strcmp ((const char *)xml->name, "array") == 0)
		return SOUP_XMLRPC_VALUE_TYPE_ARRAY;
	else
		return SOUP_XMLRPC_VALUE_TYPE_BAD;
}

gboolean
soup_xmlrpc_value_get_int (SoupXmlrpcValue *value, long *i)
{
	xmlNode *xml;
	xmlChar *content;
	char *tail;
	gboolean ok;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml || (strcmp ((const char *)xml->name, "int") && strcmp ((const char *)xml->name, "i4")))
		return FALSE;

	/* FIXME this should be exactly one text node */
	content = xmlNodeGetContent (xml);
	*i = strtol ((char *)content, &tail, 10);
	ok = (*tail == '\0');
	xmlFree (content);

	return ok;
}

gboolean
soup_xmlrpc_value_get_double (SoupXmlrpcValue *value, double *d)
{
	xmlNode *xml;
	xmlChar *content;
	char *tail;
	gboolean ok;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml || (strcmp ((const char *)xml->name, "double")))
		return FALSE;

	/* FIXME this should be exactly one text node */
	content = xmlNodeGetContent (xml);
	*d = g_ascii_strtod ((char *)content, &tail);
	ok = (*tail == '\0');
	xmlFree (content);

	return ok;
}

gboolean
soup_xmlrpc_value_get_boolean (SoupXmlrpcValue *value, gboolean *b)
{
	xmlNode *xml;
	xmlChar *content;
	char *tail;
	gboolean ok;
	int i;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml || strcmp ((const char *)xml->name, "boolean"))
		return FALSE;

	content = xmlNodeGetContent (xml);
	i = strtol ((char *)content, &tail, 10);
	*b = (i == 1);
	ok = (*tail == '\0');
	xmlFree (content);

	return ok;
}

gboolean
soup_xmlrpc_value_get_string (SoupXmlrpcValue *value, char **str)
{
	xmlNode *xml;
	xmlChar *content;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml)
		return FALSE;
	if (xml->type == XML_ELEMENT_NODE) {
		if (strcmp ((char *)xml->name, "string"))
			return FALSE;
	} else if (xml->type != XML_TEXT_NODE)
		return FALSE;
	
	content = xmlNodeGetContent (xml);
	*str = content ? g_strdup ((char *)content) : g_strdup ("");
	xmlFree (content);

	return TRUE;
}

gboolean
soup_xmlrpc_value_get_datetime (SoupXmlrpcValue *value, time_t *timeval)
{
	xmlNode *xml;
	xmlChar *content;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml || (strcmp ((const char *)xml->name, "dateTime.iso8601")))
		return FALSE;

	/* FIXME this should be exactly one text node */
	content = xmlNodeGetContent (xml);
	if (xmlStrlen (content) != 17) {
		xmlFree (content);
		return FALSE;
	}

	*timeval = soup_date_iso8601_parse ((char *)content);
	xmlFree (content);
	return TRUE;
}

gboolean
soup_xmlrpc_value_get_base64 (SoupXmlrpcValue *value, GByteArray **data)
{
	xmlNode *xml;
	xmlChar *content;
	guchar *decoded;
	gsize len;

	xml = (xmlNode *) value;
	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);
	if (!xml || strcmp ((const char *)xml->name, "base64"))
		return FALSE;

	content = xmlNodeGetContent (xml);
	decoded = g_base64_decode ((const char *)content, &len);
	xmlFree (content);

	*data = g_byte_array_new ();
	g_byte_array_append (*data, decoded, len);
	g_free (decoded);

	return TRUE;
}


gboolean
soup_xmlrpc_value_get_struct (SoupXmlrpcValue *value, GHashTable **table)
{
	xmlNode *xml;
	GHashTable *t;

	xml = (xmlNode *) value;

	if (strcmp ((const char *)xml->name, "value"))
		return FALSE;
	xml = exactly_one_child (xml);

	if (!xml || strcmp ((const char *)xml->name, "struct"))
		return FALSE;

	t = g_hash_table_new_full (g_str_hash, g_str_equal, xmlFree, NULL);

	for (xml = soup_xml_real_node (xml->children);
	     xml;
	     xml = soup_xml_real_node (xml->next)) {
		xmlChar *name;
		xmlNode *val, *cur;

		if (strcmp ((const char *)xml->name, "member") ||
		    !xml->children)
			goto bad;

		name = NULL;
		val = NULL;

		for (cur = soup_xml_real_node (xml->children);
		     cur;
		     cur = soup_xml_real_node (cur->next)) {
			if (strcmp((const char *)cur->name, "name") == 0) {
				if (name)
					goto local_bad;
				name = xmlNodeGetContent (cur);
			} else if (strcmp ((const char *)cur->name, "value") == 0)
				val = cur;
			else
				goto local_bad;

			continue;
local_bad:
			if (name)
				xmlFree (name);
			goto bad;
		}

		if (!name || !val) {
			if (name)
				xmlFree (name);
			goto bad;
		}
		g_hash_table_insert (t, name, val);
	}

	*table = t;
	return TRUE;

bad:
	g_hash_table_destroy (t);
	return FALSE;
}

gboolean
soup_xmlrpc_value_array_get_iterator (SoupXmlrpcValue *value, SoupXmlrpcValueArrayIterator **iter)
{
	xmlNode *xml, *array, *data;

	xml = (xmlNode *) value;

	array = soup_xml_real_node (xml->children);
	if (!array || strcmp((const char *)array->name, "array") != 0 ||
	    soup_xml_real_node (array->next))
		return FALSE;

	data = soup_xml_real_node (array->children);
	if (!data || strcmp((const char *)data->name, "data") != 0 ||
	    soup_xml_real_node (data->next))
		return FALSE;

	*iter = (SoupXmlrpcValueArrayIterator *) soup_xml_real_node (data->children);
	return TRUE;
}


SoupXmlrpcValueArrayIterator *
soup_xmlrpc_value_array_iterator_prev (SoupXmlrpcValueArrayIterator *iter)
{
	xmlNode *xml, *prev;

	xml = (xmlNode *) iter;

	prev = xml->prev;
	while (prev != soup_xml_real_node (prev))
		prev = prev->prev;

	return (SoupXmlrpcValueArrayIterator *) prev;
}

SoupXmlrpcValueArrayIterator *
soup_xmlrpc_value_array_iterator_next (SoupXmlrpcValueArrayIterator *iter)
{
	xmlNode *xml;

	xml = (xmlNode *) iter;

	return (SoupXmlrpcValueArrayIterator *) soup_xml_real_node (xml->next);
}

gboolean
soup_xmlrpc_value_array_iterator_get_value (SoupXmlrpcValueArrayIterator *iter,
					    SoupXmlrpcValue **value)
{
	*value = (SoupXmlrpcValue *) iter;

	return TRUE;
}

static void
indent (int d)
{
	while (d--)
		g_printerr (" ");
}

static void
soup_xmlrpc_value_dump_internal (SoupXmlrpcValue *value, int d);

static void
soup_xmlrpc_value_dump_struct_member (const char *name, SoupXmlrpcValue *value, gpointer d)
{
	indent (GPOINTER_TO_INT (d));
	g_printerr ("MEMBER: %s\n", name);
	soup_xmlrpc_value_dump_internal (value, GPOINTER_TO_INT (d));
}

static void
soup_xmlrpc_value_dump_array_element (const int i, SoupXmlrpcValue *value, gpointer d)
{
	indent (GPOINTER_TO_INT (d));
	g_printerr ("ELEMENT: %d\n", i);
	soup_xmlrpc_value_dump_internal (value, GPOINTER_TO_INT (d));
}

static void
soup_xmlrpc_value_dump_internal (SoupXmlrpcValue *value, int d)
{
	long i;
	gboolean b;
	char *str;
	double f;
	time_t timeval;
	GByteArray *base64;
	GHashTable *hash;
	SoupXmlrpcValueArrayIterator *iter;

	g_printerr ("\n\n[%s]\n", ((xmlNode*)value)->name);
	switch (soup_xmlrpc_value_get_type (value)) {

		case SOUP_XMLRPC_VALUE_TYPE_BAD:
			indent (d);
			g_printerr ("BAD\n");
			break;

		case SOUP_XMLRPC_VALUE_TYPE_INT:
			indent (d);
			if (!soup_xmlrpc_value_get_int (value, &i))
				g_printerr ("BAD INT\n");
			else
				g_printerr ("INT: %ld\n", i);
			break;

		case SOUP_XMLRPC_VALUE_TYPE_BOOLEAN:
			indent (d);
			if (!soup_xmlrpc_value_get_boolean (value, &b))
				g_printerr ("BAD BOOLEAN\n");
			else
				g_printerr ("BOOLEAN: %s\n", b ? "true" : "false");
			break;

		case SOUP_XMLRPC_VALUE_TYPE_STRING:
			indent (d);
			if (!soup_xmlrpc_value_get_string (value, &str))
				g_printerr ("BAD STRING\n");
			else {
				g_printerr ("STRING: \"%s\"\n", str);
				g_free (str);
			}
			break;

		case SOUP_XMLRPC_VALUE_TYPE_DOUBLE:
			indent (d);
			if (!soup_xmlrpc_value_get_double (value, &f))
				g_printerr ("BAD DOUBLE\n");
			else
				g_printerr ("DOUBLE: %f\n", f);
			break;

		case SOUP_XMLRPC_VALUE_TYPE_DATETIME:
			indent (d);
			if (!soup_xmlrpc_value_get_datetime (value, &timeval))
				g_printerr ("BAD DATETIME\n");
			else
				g_printerr ("DATETIME: %s\n", asctime (gmtime (&timeval)));
			break;

		case SOUP_XMLRPC_VALUE_TYPE_BASE64:
			indent (d);
			if (!soup_xmlrpc_value_get_base64 (value, &base64))
				g_printerr ("BAD BASE64\n");
			else {
				GString *hex = g_string_new (NULL);
				int i;

				for (i = 0; i < base64->len; i++)
					g_string_append_printf (hex, "%02x", base64->data[i]);

				g_printerr ("BASE64: %s\n", hex->str);
				g_string_free (hex, TRUE);
				g_byte_array_free (base64, TRUE);
			}

			break;

		case SOUP_XMLRPC_VALUE_TYPE_STRUCT:
			indent (d);
			if (!soup_xmlrpc_value_get_struct (value, &hash))
				g_printerr ("BAD STRUCT\n");
			else {
				g_printerr ("STRUCT\n");
				g_hash_table_foreach (hash, (GHFunc) soup_xmlrpc_value_dump_struct_member,
						      GINT_TO_POINTER (d+1));
				g_hash_table_destroy (hash);
			}
			break;

		case SOUP_XMLRPC_VALUE_TYPE_ARRAY:
			indent (d);
			if (!soup_xmlrpc_value_array_get_iterator (value, &iter))
				g_printerr ("BAD ARRAY\n");
			else {
				SoupXmlrpcValue *evalue;
				int i = 0;
				g_printerr ("ARRAY\n");
				while (iter != NULL) {
					soup_xmlrpc_value_array_iterator_get_value (iter, &evalue);
					soup_xmlrpc_value_dump_array_element (i, evalue, GINT_TO_POINTER (d+1));
					iter = soup_xmlrpc_value_array_iterator_next (iter);
					i++;
				}
			}
			break;
	}

}

void
soup_xmlrpc_value_dump (SoupXmlrpcValue *value)
{
	soup_xmlrpc_value_dump_internal (value, 0);
}

