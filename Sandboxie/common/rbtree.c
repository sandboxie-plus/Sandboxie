/*
 * rbtree.c -- generic red black tree
 *
 * Copyright (c) 2001-2007, NLnet Labs. All rights reserved.
 * 
 * This software is open source under the BSD license.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 * Implementation of a redblack tree.
 */

#include <stdlib.h>
#include "rbtree.h"

#define log_assert(x)

/** Node colour black */
#define	BLACK	0
/** Node colour red */
#define	RED	1

/** the NULL node, global alloc */
rbnode_t	rbtree_null_node = {
	RBTREE_NULL,		/* Parent.  */
	RBTREE_NULL,		/* Left.  */
	RBTREE_NULL,		/* Right.  */
	NULL,			/* Key.  */
	BLACK			/* Color.  */
};

/** rotate subtree left (to preserve redblack property) */
static void rbtree_rotate_left(rbtree_t *rbtree, rbnode_t *node);
/** rotate subtree right (to preserve redblack property) */
static void rbtree_rotate_right(rbtree_t *rbtree, rbnode_t *node);
/** Fixup node colours when insert happened */
static void rbtree_insert_fixup(rbtree_t *rbtree, rbnode_t *node);
/** Fixup node colours when delete happened */
static void rbtree_delete_fixup(rbtree_t* rbtree, rbnode_t* child, rbnode_t* child_parent);

/*
 * Initializes a new red black tree.
 *
 */
void 
rbtree_init(rbtree_t *rbtree, int (*cmpf)(const void *, const void *))
{
	rbtree->root = RBTREE_NULL;
	rbtree->count = 0;
	rbtree->cmp = cmpf;
}

/*
 * Rotates the node to the left.
 *
 */
static void
rbtree_rotate_left(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t *right = node->right;
	node->right = right->left;
	if (right->left != RBTREE_NULL)
		right->left->parent = node;

	right->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->left) {
			node->parent->left = right;
		} else  {
			node->parent->right = right;
		}
	} else {
		rbtree->root = right;
	}
	right->left = node;
	node->parent = right;
}

/*
 * Rotates the node to the right.
 *
 */
static void
rbtree_rotate_right(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t *left = node->left;
	node->left = left->right;
	if (left->right != RBTREE_NULL)
		left->right->parent = node;

	left->parent = node->parent;

	if (node->parent != RBTREE_NULL) {
		if (node == node->parent->right) {
			node->parent->right = left;
		} else  {
			node->parent->left = left;
		}
	} else {
		rbtree->root = left;
	}
	left->right = node;
	node->parent = left;
}

static void
rbtree_insert_fixup(rbtree_t *rbtree, rbnode_t *node)
{
	rbnode_t	*uncle;

	/* While not at the root and need fixing... */
	while (node != rbtree->root && node->parent->color == RED) {
		/* If our parent is left child of our grandparent... */
		if (node->parent == node->parent->parent->left) {
			uncle = node->parent->parent->right;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->right) {
					node = node->parent;
					rbtree_rotate_left(rbtree, node);
				}
				/* Now we're the left child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_right(rbtree, node->parent->parent);
			}
		} else {
			uncle = node->parent->parent->left;

			/* If our uncle is red... */
			if (uncle->color == RED) {
				/* Paint the parent and the uncle black... */
				node->parent->color = BLACK;
				uncle->color = BLACK;

				/* And the grandparent red... */
				node->parent->parent->color = RED;

				/* And continue fixing the grandparent */
				node = node->parent->parent;
			} else {				/* Our uncle is black... */
				/* Are we the right child? */
				if (node == node->parent->left) {
					node = node->parent;
					rbtree_rotate_right(rbtree, node);
				}
				/* Now we're the right child, repaint and rotate... */
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				rbtree_rotate_left(rbtree, node->parent->parent);
			}
		}
	}
	rbtree->root->color = BLACK;
}


/*
 * Inserts a node into a red black tree.
 *
 * Returns NULL on failure or the pointer to the newly added node
 * otherwise.
 */
rbnode_t *
rbtree_insert (rbtree_t *rbtree, rbnode_t *data)
{
	/* XXX Not necessary, but keeps compiler quiet... */
	int r = 0;

	/* We start at the root of the tree */
	rbnode_t	*node = rbtree->root;
	rbnode_t	*parent = RBTREE_NULL;

	/* Lets find the new parent... */
	while (node != RBTREE_NULL) {
		/* Compare two keys, do we have a duplicate? */
		if ((r = rbtree->cmp(data->key, node->key)) == 0) {
			return NULL;
		}
		parent = node;

		if (r < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	/* Initialize the new node */
	data->parent = parent;
	data->left = data->right = RBTREE_NULL;
	data->color = RED;
	rbtree->count++;

	/* Insert it into the tree... */
	if (parent != RBTREE_NULL) {
		if (r < 0) {
			parent->left = data;
		} else {
			parent->right = data;
		}
	} else {
		rbtree->root = data;
	}

	/* Fix up the red-black properties... */
	rbtree_insert_fixup(rbtree, data);

	return data;
}

/*
 * Searches the red black tree, returns the data if key is found or NULL otherwise.
 *
 */
rbnode_t *
rbtree_search (rbtree_t *rbtree, const void *key)
{
	rbnode_t *node;

	if (rbtree_find_less_equal(rbtree, key, &node)) {
		return node;
	} else {
		return NULL;
	}
}

/** helpers for delete: swap node colours */
static void swap_int8(unsigned char* x, unsigned char* y) 
{ 
	unsigned char t = *x; *x = *y; *y = t; 
}

/** helpers for delete: swap node pointers */
static void swap_np(rbnode_t** x, rbnode_t** y) 
{
	rbnode_t* t = *x; *x = *y; *y = t; 
}

/** Update parent pointers of child trees of 'parent' */
static void change_parent_ptr(rbtree_t* rbtree, rbnode_t* parent, rbnode_t* old, rbnode_t* new)
{
	if(parent == RBTREE_NULL)
	{
		log_assert(rbtree->root == old);
		if(rbtree->root == old) rbtree->root = new;
		return;
	}
	log_assert(parent->left == old || parent->right == old
		|| parent->left == new || parent->right == new);
	if(parent->left == old) parent->left = new;
	if(parent->right == old) parent->right = new;
}
/** Update parent pointer of a node 'child' */
static void change_child_ptr(rbnode_t* child, rbnode_t* old, rbnode_t* new)
{
	if(child == RBTREE_NULL) return;
	log_assert(child->parent == old || child->parent == new);
	if(child->parent == old) child->parent = new;
}

rbnode_t* 
rbtree_delete(rbtree_t *rbtree, const void *key)
{
	rbnode_t *to_delete;
	rbnode_t *child;
	if((to_delete = rbtree_search(rbtree, key)) == 0) return 0;
	rbtree->count--;

	/* make sure we have at most one non-leaf child */
	if(to_delete->left != RBTREE_NULL && to_delete->right != RBTREE_NULL)
	{
		/* swap with smallest from right subtree (or largest from left) */
		rbnode_t *smright = to_delete->right;
		while(smright->left != RBTREE_NULL)
			smright = smright->left;
		/* swap the smright and to_delete elements in the tree,
		 * but the rbnode_t is first part of user data struct
		 * so cannot just swap the keys and data pointers. Instead
		 * readjust the pointers left,right,parent */

		/* swap colors - colors are tied to the position in the tree */
		swap_int8(&to_delete->color, &smright->color);

		/* swap child pointers in parents of smright/to_delete */
		change_parent_ptr(rbtree, to_delete->parent, to_delete, smright);
		if(to_delete->right != smright)
			change_parent_ptr(rbtree, smright->parent, smright, to_delete);

		/* swap parent pointers in children of smright/to_delete */
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->left, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(smright->right, smright, to_delete);
		change_child_ptr(to_delete->left, to_delete, smright);
		if(to_delete->right != smright)
			change_child_ptr(to_delete->right, to_delete, smright);
		if(to_delete->right == smright)
		{
			/* set up so after swap they work */
			to_delete->right = to_delete;
			smright->parent = smright;
		}

		/* swap pointers in to_delete/smright nodes */
		swap_np(&to_delete->parent, &smright->parent);
		swap_np(&to_delete->left, &smright->left);
		swap_np(&to_delete->right, &smright->right);

		/* now delete to_delete (which is at the location where the smright previously was) */
	}
	log_assert(to_delete->left == RBTREE_NULL || to_delete->right == RBTREE_NULL);

	if(to_delete->left != RBTREE_NULL) child = to_delete->left;
	else child = to_delete->right;

	/* unlink to_delete from the tree, replace to_delete with child */
	change_parent_ptr(rbtree, to_delete->parent, to_delete, child);
	change_child_ptr(child, to_delete, to_delete->parent);

	if(to_delete->color == RED)
	{
		/* if node is red then the child (black) can be swapped in */
	}
	else if(child->color == RED)
	{
		/* change child to BLACK, removing a RED node is no problem */
		if(child!=RBTREE_NULL) child->color = BLACK;
	}
	else rbtree_delete_fixup(rbtree, child, to_delete->parent);

	/* unlink completely */
	to_delete->parent = RBTREE_NULL;
	to_delete->left = RBTREE_NULL;
	to_delete->right = RBTREE_NULL;
	to_delete->color = BLACK;
	return to_delete;
}

static void rbtree_delete_fixup(rbtree_t* rbtree, rbnode_t* child, rbnode_t* child_parent)
{
	rbnode_t* sibling;
	int go_up = 1;

	/* determine sibling to the node that is one-black short */
	if(child_parent->right == child) sibling = child_parent->left;
	else sibling = child_parent->right;

	while(go_up)
	{
		if(child_parent == RBTREE_NULL)
		{
			/* removed parent==black from root, every path, so ok */
			return;
		}

		if(sibling->color == RED)
		{	/* rotate to get a black sibling */
			child_parent->color = RED;
			sibling->color = BLACK;
			if(child_parent->right == child)
				rbtree_rotate_right(rbtree, child_parent);
			else	rbtree_rotate_left(rbtree, child_parent);
			/* new sibling after rotation */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}

		if(child_parent->color == BLACK 
			&& sibling->color == BLACK
			&& sibling->left->color == BLACK
			&& sibling->right->color == BLACK)
		{	/* fixup local with recolor of sibling */
			if(sibling != RBTREE_NULL)
				sibling->color = RED;

			child = child_parent;
			child_parent = child_parent->parent;
			/* prepare to go up, new sibling */
			if(child_parent->right == child) sibling = child_parent->left;
			else sibling = child_parent->right;
		}
		else go_up = 0;
	}

	if(child_parent->color == RED
		&& sibling->color == BLACK
		&& sibling->left->color == BLACK
		&& sibling->right->color == BLACK) 
	{
		/* move red to sibling to rebalance */
		if(sibling != RBTREE_NULL)
			sibling->color = RED;
		child_parent->color = BLACK;
		return;
	}
	log_assert(sibling != RBTREE_NULL);

	/* get a new sibling, by rotating at sibling. See which child
	   of sibling is red */
	if(child_parent->right == child
		&& sibling->color == BLACK
		&& sibling->right->color == RED
		&& sibling->left->color == BLACK)
	{
		sibling->color = RED;
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}
	else if(child_parent->left == child
		&& sibling->color == BLACK
		&& sibling->left->color == RED
		&& sibling->right->color == BLACK)
	{
		sibling->color = RED;
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, sibling);
		/* new sibling after rotation */
		if(child_parent->right == child) sibling = child_parent->left;
		else sibling = child_parent->right;
	}

	/* now we have a black sibling with a red child. rotate and exchange colors. */
	sibling->color = child_parent->color;
	child_parent->color = BLACK;
	if(child_parent->right == child)
	{
		log_assert(sibling->left->color == RED);
		sibling->left->color = BLACK;
		rbtree_rotate_right(rbtree, child_parent);
	}
	else
	{
		log_assert(sibling->right->color == RED);
		sibling->right->color = BLACK;
		rbtree_rotate_left(rbtree, child_parent);
	}
}

int
rbtree_find_less_equal(rbtree_t *rbtree, const void *key, rbnode_t **result)
{
	int r;
	rbnode_t *node;

	log_assert(result);
	
	/* We start at root... */
	node = rbtree->root;

	*result = NULL;

	/* While there are children... */
	while (node != RBTREE_NULL) {
		r = rbtree->cmp(key, node->key);
		if (r == 0) {
			/* Exact match */
			*result = node;
			return 1;
		} 
		if (r < 0) {
			node = node->left;
		} else {
			/* Temporary match */
			*result = node;
			node = node->right;
		}
	}
	return 0;
}

/*
 * Finds the first element in the red black tree
 *
 */
rbnode_t *
rbtree_first (rbtree_t *rbtree)
{
	rbnode_t *node;

	for (node = rbtree->root; node->left != RBTREE_NULL; node = node->left);
	return node;
}

rbnode_t *
rbtree_last (rbtree_t *rbtree)
{
	rbnode_t *node;

	for (node = rbtree->root; node->right != RBTREE_NULL; node = node->right);
	return node;
}

/*
 * Returns the next node...
 *
 */
rbnode_t *
rbtree_next (rbnode_t *node)
{
	rbnode_t *parent;

	if (node->right != RBTREE_NULL) {
		/* One right, then keep on going left... */
		for (node = node->right; node->left != RBTREE_NULL; node = node->left);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->right) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

rbnode_t *
rbtree_previous(rbnode_t *node)
{
	rbnode_t *parent;

	if (node->left != RBTREE_NULL) {
		/* One left, then keep on going right... */
		for (node = node->left; node->right != RBTREE_NULL; node = node->right);
	} else {
		parent = node->parent;
		while (parent != RBTREE_NULL && node == parent->left) {
			node = parent;
			parent = parent->parent;
		}
		node = parent;
	}
	return node;
}

/** recursive descent traverse */
static void 
traverse_post(void (*func)(rbnode_t*, void*), void* arg, rbnode_t* node)
{
	if(!node || node == RBTREE_NULL)
		return;
	/* recurse */
	traverse_post(func, arg, node->left);
	traverse_post(func, arg, node->right);
	/* call user func */
	(*func)(node, arg);
}

void 
traverse_postorder(rbtree_t* tree, void (*func)(rbnode_t*, void*), void* arg)
{
	traverse_post(func, arg, tree->root);
}