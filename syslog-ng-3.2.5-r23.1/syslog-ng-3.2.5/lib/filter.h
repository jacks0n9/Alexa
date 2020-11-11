/*
 * Copyright (c) 2002-2010 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2010 Balázs Scheidler
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
  
#ifndef FILTER_H_INCLUDED
#define FILTER_H_INCLUDED

#include "syslog-ng.h"
#include "logmsg.h"
#include "messages.h"
#include "logprocess.h"
#include "logmatcher.h"
#include "cfg-parser.h"

struct _GlobalConfig;

typedef struct _FilterExprNode
{
  gboolean comp;
  /* this filter changes the log message */
  gboolean modify;
  const gchar *type;
  gboolean (*eval)(struct _FilterExprNode *self, LogMessage *msg);
  void (*free_fn)(struct _FilterExprNode *self);
} FilterExprNode;

gboolean filter_expr_eval(FilterExprNode *self, LogMessage *msg);
void filter_expr_free(FilterExprNode *self);

typedef struct _FilterRE
{
  FilterExprNode super;
  NVHandle value_handle;
  LogMatcher *matcher;
} FilterRE;

typedef struct _FilterMatch FilterMatch;

void filter_re_set_matcher(FilterRE *self, LogMatcher *matcher);
gboolean filter_re_set_regexp(FilterRE *self, gchar *re);
void filter_re_set_flags(FilterRE *self, gint flags);
void filter_tags_add(FilterExprNode *s, GList *tags);

/* various constructors */

FilterExprNode *fop_or_new(FilterExprNode *e1, FilterExprNode *e2);
FilterExprNode *fop_and_new(FilterExprNode *e1, FilterExprNode *e2);
FilterExprNode *fop_cmp_new(LogTemplate *left, LogTemplate *right, gint op);

FilterExprNode *filter_facility_new(guint32 facilities);
FilterExprNode *filter_level_new(guint32 levels);
FilterExprNode *filter_call_new(gchar *rule, struct _GlobalConfig *cfg);
FilterExprNode *filter_netmask_new(gchar *cidr);
FilterExprNode *filter_re_new(NVHandle value_handle);
FilterExprNode *filter_match_new(void);
FilterExprNode *filter_tags_new(GList *tags);

LogProcessRule *log_filter_rule_new(const gchar *name, FilterExprNode *expr);

#endif
