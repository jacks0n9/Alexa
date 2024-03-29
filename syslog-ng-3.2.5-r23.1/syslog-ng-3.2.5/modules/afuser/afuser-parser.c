/*
 * Copyright (c) 2002-2010 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2010 Balázs Scheidler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
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

#include "afuser.h"
#include "cfg-parser.h"
#include "afuser-grammar.h"

extern int afuser_debug;

int afuser_parse(CfgLexer *lexer, LogDriver **instance);

static CfgLexerKeyword afuser_keywords[] = {
  { "usertty",               KW_USERTTY },

  { NULL }
};

CfgParser afuser_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &afuser_debug,
#endif
  .name = "afuser",
  .keywords = afuser_keywords,
  .parse = (gint (*)(CfgLexer *, gpointer *)) afuser_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(afuser_, LogDriver **)
