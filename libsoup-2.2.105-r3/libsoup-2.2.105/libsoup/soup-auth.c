/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-auth.c: HTTP Authentication framework
 *
 * Copyright (C) 2001-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "soup-auth.h"
#include "soup-auth-basic.h"
#include "soup-auth-digest.h"
#include "soup-headers.h"

G_DEFINE_TYPE (SoupAuth, soup_auth, G_TYPE_OBJECT)

static void
finalize (GObject *object)
{
	SoupAuth *auth = SOUP_AUTH (object);

	g_free (auth->realm);

	G_OBJECT_CLASS (soup_auth_parent_class)->finalize (object);
}

static void
soup_auth_class_init (SoupAuthClass *auth_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (auth_class);

	object_class->finalize = finalize;
}

static void
soup_auth_init (SoupAuth *auth)
{
}

typedef GType (*GTypeFunc) (void);

typedef struct {
	const char *name;
	int         len;
	GTypeFunc   type_func;
	int         strength;
} AuthScheme; 

static AuthScheme known_auth_schemes [] = {
	{ "Basic",  sizeof ("Basic") - 1,  soup_auth_basic_get_type,  0 },
	{ "Digest", sizeof ("Digest") - 1, soup_auth_digest_get_type, 3 },
	{ NULL }
};

/* FIXME: it should be possible to register new auth schemes! */

/**
 * soup_auth_new_from_header_list:
 * @vals: a list of WWW-Authenticate headers from a server response
 *
 * Creates a #SoupAuth value based on the strongest available
 * supported auth type in @vals.
 *
 * Return value: the new #SoupAuth, or %NULL if none could be created.
 **/
SoupAuth *
soup_auth_new_from_header_list (const GSList *vals)
{
	char *header = NULL, *realm;
	AuthScheme *scheme = NULL, *iter;
	SoupAuth *auth = NULL;
	GHashTable *params;

	g_return_val_if_fail (vals != NULL, NULL);

	while (vals) {
		char *tryheader = vals->data;

		for (iter = known_auth_schemes; iter->name; iter++) {
			if (!g_ascii_strncasecmp (tryheader, iter->name, 
						  iter->len) &&
			    (!tryheader[iter->len] ||
			     g_ascii_isspace (tryheader[iter->len]))) {
				if (!scheme || 
				    scheme->strength < iter->strength) {
					header = tryheader;
					scheme = iter;
				}

				break;
			}
		}

		vals = vals->next;
	}

	if (!scheme)
		return NULL;

	params = soup_header_param_parse_list (header + scheme->len);
	if (!params)
		return NULL;
	realm = soup_header_param_copy_token (params, "realm");
	if (!realm) {
		soup_header_param_destroy_hash (params);
		return NULL;
	}

	auth = g_object_new (scheme->type_func (), NULL);
	auth->realm = realm;

	SOUP_AUTH_GET_CLASS (auth)->construct (auth, params);
	soup_header_param_destroy_hash (params);
	return auth;
}

/**
 * soup_auth_authenticate:
 * @auth: a #SoupAuth
 * @username: the username provided by the user or client
 * @password: the password provided by the user or client
 *
 * This is called by the session after requesting a username and
 * password from the application. @auth will take the information
 * and do whatever scheme-specific processing is needed.
 **/
void
soup_auth_authenticate (SoupAuth *auth, const char *username, const char *password)
{
	g_return_if_fail (SOUP_IS_AUTH (auth));
	g_return_if_fail (username != NULL);
	g_return_if_fail (password != NULL);

	SOUP_AUTH_GET_CLASS (auth)->authenticate (auth, username, password);
}

/**
 * soup_auth_get_scheme_name:
 * @auth: a #SoupAuth
 *
 * Returns @auth's scheme name. (Eg, "Basic")
 *
 * Return value: the scheme name
 **/
const char *
soup_auth_get_scheme_name (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return SOUP_AUTH_GET_CLASS (auth)->scheme_name;
}

/**
 * soup_auth_get_realm:
 * @auth: a #SoupAuth
 *
 * Returns @auth's realm.
 *
 * Return value: the realm name
 **/
const char *
soup_auth_get_realm (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return auth->realm;
}

/**
 * soup_auth_get_info:
 * @auth: a #SoupAuth
 *
 * Gets an identifier for @auth. #SoupAuth objects from the same
 * server with the same identifier refer to the same authentication
 * domain (eg, the URLs associated with them take the same usernames
 * and passwords).
 *
 * Return value: the identifier
 **/
char *
soup_auth_get_info (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);

	return g_strdup_printf ("%s:%s",
				SOUP_AUTH_GET_CLASS (auth)->scheme_name,
				auth->realm);
}

/**
 * soup_auth_is_authenticated:
 * @auth: a #SoupAuth
 *
 * Tests if @auth has been given a username and password
 *
 * Return value: %TRUE if @auth has been given a username and password
 **/
gboolean
soup_auth_is_authenticated (SoupAuth *auth)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), TRUE);

	return SOUP_AUTH_GET_CLASS (auth)->is_authenticated (auth);
}

/**
 * soup_auth_get_authorization:
 * @auth: a #SoupAuth
 * @msg: the #SoupMessage to be authorized
 *
 * Generates an appropriate "Authorization" header for @msg. (The
 * session will only call this if soup_auth_is_authenticated()
 * returned %TRUE.)
 *
 * Return value: the "Authorization" header, which must be freed.
 **/
char *
soup_auth_get_authorization (SoupAuth *auth, SoupMessage *msg)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);
	g_return_val_if_fail (msg != NULL, NULL);

	return SOUP_AUTH_GET_CLASS (auth)->get_authorization (auth, msg);
}

/**
 * soup_auth_get_protection_space:
 * @auth: a #SoupAuth
 * @source_uri: the URI of the request that @auth was generated in
 * response to.
 *
 * Returns a list of paths on the server which @auth extends over.
 * (All subdirectories of these paths are also assumed to be part
 * of @auth's protection space, unless otherwise discovered not to
 * be.)
 *
 * Return value: the list of paths, which must be freed with
 * soup_auth_free_protection_space().
 **/
GSList *
soup_auth_get_protection_space (SoupAuth *auth, const SoupUri *source_uri)
{
	g_return_val_if_fail (SOUP_IS_AUTH (auth), NULL);
	g_return_val_if_fail (source_uri != NULL, NULL);

	return SOUP_AUTH_GET_CLASS (auth)->get_protection_space (auth, source_uri);
}

/**
 * soup_auth_free_protection_space:
 * @auth: a #SoupAuth
 * @space: the return value from soup_auth_get_protection_space()
 *
 * Frees @space.
 **/
void
soup_auth_free_protection_space (SoupAuth *auth, GSList *space)
{
	GSList *s;

	for (s = space; s; s = s->next)
		g_free (s->data);
	g_slist_free (space);
}
