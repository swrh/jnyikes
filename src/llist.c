/*
 * Copyright 2005, 2009 Fernando Silveira <fsilveira@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Fernando Silveira.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <malloc.h>
#include <assert.h>

#include "llist.h"

void
llappend(void **base_v, void *item_v)
{
	struct st_llist *p;
	struct st_llist **base = (struct st_llist **)base_v, *item = item_v;

	assert(base != NULL);
	assert(item != NULL);

	if (!*base) {
		*base = item;
		(*base)->prev = (*base)->next = NULL;
	} else {
		assert((*base)->prev == NULL);
		for (p = *base; p->next; p = p->next);

		p->next = item;
		item->prev = p;
		item->next = NULL;
	}
}

void
llprepend(void **base_v, void *item_v)
{
	struct st_llist **base = (struct st_llist **)base_v, *item = item_v;

	assert(base != NULL);
	assert(item != NULL);

	if (!*base) {
		*base = item;
		(*base)->prev = (*base)->next = NULL;
	} else {
		assert((*base)->prev == NULL);

		(*base)->prev = item;
		item->next = *base;
		item->prev = NULL;
	}
}

void
llcut(void **base_v, void *item_v)
{
	struct st_llist **base = (struct st_llist **)base_v, *item = item_v;

	assert(base != NULL);
	assert(item != NULL);
	assert(*base != NULL);
	assert((*base)->prev == NULL);

	if (*base == item) {
		*base = (*base)->next;
		if (*base)
			(*base)->prev = NULL;

		item->prev = item->next = NULL;
	} else {
		if (item->next)
			item->next->prev = item->prev;

		item->prev->next = item->next;

		item->prev = item->next = NULL;
	}
}

void
lldel(void **base_v, void *item_v)
{
	struct st_llist **base = (struct st_llist **)base_v, *item = item_v;

	assert(base != NULL);
	assert(item != NULL);
	assert(*base != NULL);
	assert((*base)->prev == NULL);

	llcut((void **) base, item);
	free(item);
}

void
lldestroy(void **base_v)
{
	struct st_llist **base = (struct st_llist **)base_v;

	assert(base != NULL);

	if (!*base)
		return;

	assert((*base)->prev == NULL);

	while (*base)
		lldel((void **)base, *base);
}
