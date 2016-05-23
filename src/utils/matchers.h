/* vifm
 * Copyright (C) 2016 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef VIFM__UTILS__MATCHERS_H__
#define VIFM__UTILS__MATCHERS_H__

#include "test_helpers.h"

/* Opaque matchers type. */
typedef struct matchers_t matchers_t;

/* Arguments and return value match matcher_alloc() except for first argument,
 * which is a list here. */
matchers_t * matchers_alloc(const char list[], int cs_by_def, int glob_by_def,
		const char on_empty_re[], char **error);

/* Frees all resources allocated by the matchers.  matchers can be NULL. */
void matchers_free(matchers_t *matchers);

/* Checks whether given path/name matches.  Returns non-zero if so, otherwise
 * zero is returned. */
int matchers_match(const matchers_t *matchers, const char path[]);

/* Retrieves original matcher expression.  Returns the expression. */
const char * matchers_get_expr(const matchers_t *matchers);

TSTATIC_DEFS(
	char ** break_into_matchers(const char concat[], int *count);
)

#endif /* VIFM__UTILS__MATCHERS_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
