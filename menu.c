/*
 *  kexecboot - A kexec based bootloader
 *
 *  Copyright (c) 2009-2011 Yuri Bushmelev <jay4mail@gmail.com>
 *  Copyright (c) 2009 Omegamoon <omegamoon@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

#include "config.h"
#include "menu.h"
#include "util.h"

kx_menu *
menu_create(void) {
	kx_menu *menu;
	
	menu = malloc(sizeof *menu);
	if (menu == NULL) {
		DPRINTF("Can\'t allocate menu");
		return NULL;
	}

	
	menu->head_lvls = TAILQ_HEAD_INITIALIZER(menu->head_lvls);
	TAILQ_INIT(menu->head_lvls);
	
	menu->top = NULL;
	menu->current = NULL;
	return menu;
}

/* Get next available menu id */
kx_menu_id
menu_get_next_id(kx_menu *menu) {
	return menu->next_id++;
}

kx_menu_level *
menu_level_create(kx_menu *menu, kx_menu_level *parent) {
	
	kx_menu_level *level;
	struct kx_head head;
	
	if (!menu) return NULL;

	level = malloc(sizeof *level);
	if (level == NULL) {
		DPRINTF("Can't allocate menu items array");
		return NULL;
	}
	
	head = menu->head_lvls;
	level->parent = parent;
	if (!menu->current)
		menu->current = level;
	
	if (TAILQ_EMPTY(&head)) {
		TAILQ_INSERT_HEAD(&head, level, links);
		return level;
	}
	
	TAILQ_INSERT_TAIL(&head, level, links);	
	return level;
}


/* Add menu item to menu level */
kx_menu_item *
menu_item_add( kx_menu_level *level,
			   kx_menu_id     id,
			   char          *label,
			   char          *description,
			   kx_menu_level *submenu) {
	
	kx_menu_item *item;
	struct kx_head head;
	
	if (!level)
		return NULL;
	
	item = malloc(sizeof *item);
	if (NULL == item) {
		DPRINTF("Can't allocate menu level");
		return NULL;
	}

	item->label = strdup(label);
	item->description = ( description ? strdup(description) : NULL );
	item->id = id;
	item->submenu = submenu;

	head = level->head_itms;
	TAILQ_INSERT_TAIL(&head, item, links);
	if (!level->current) {
		level->current = item;
	}
	
	/* level->list[level->count] = item; */

	/* If there is no current item yet then make this item current */
	/* if (!level->current) { */
	/* 	level->current = item; */
	/* 	level->current_no = level->count; */
	/* } */

	/* ++level->count; */

	return item;
}

void
menu_destroy(kx_menu *menu, int destroy_data)
{
	kx_menu_level *ml;
	kx_menu_item *mi;
	kx_list hl, hi;

	hl = menu->head_lvls;
	while ( !TAILQ_EMPTY(&hl) ) {
		ml = TAILQ_FIRST(&hl);
		hi = ml->head_itms;
		while ( !TAILQ_EMPTY(&hi) ) {
			mi = TAILQ_FIRST(&hi);
			dispose(mi->label);
			dispose(mi->description);
			if (destroy_data && mi->data)
				free(mi->data);
			TAILQ_REMOVE(mi, links);
			free(mi);
		}
		TAILQ_REMOVE(ml, links);
		free(ml);
	}
}

/* Select next/prev/first item in current level */
void
menu_item_select(kx_menu *menu, int direction) {
	kx_menu_level *ml;
	struct kx_head head;

	ml = menu->current;
	head = menu->current->head_itms;
	
	if (direction == 0) 
		ml->current = TAILQ_FIRST(&head);
	else if (direction > 0) 
		ml->current = TAILQ_NEXT(ml->current, links);
	else 
		ml->current = TAILQ_PREV(ml->current, links);
}

/* Select no'th item in current level */
kx_menu_dim
menu_item_select_by_no(kx_menu *menu, int no) {
	kx_menu_level *ml;
	kx_menu_item *tmp;
	struct kx_head head;
	int i;
	
	ml = menu->current;
	head = ml->head_itms;

	i = 0;
	TAILQ_FOREACH(tmp, &head, links) {
		if (i == no) {
			i = -1;
			ml->current = tmp;
			break;
		}
		i++;
	}
	
	if (i != -1) return (-1);
	
	return no;
}


inline void
menu_item_set_data(kx_menu_item *item, void *data) {
	item->data = data;
}
