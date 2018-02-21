# 1 "anon_vma_interval_macro.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "anon_vma_interval_macro.c"
# 191 "anon_vma_interval_macro.c"
static inline unsigned long __anon_vma_interval_tree_compute_subtree_last(struct anon_vma_chain *node)
{
	unsigned long max = avc_last_pgoff(node), subtree_last;

	if (node->rb.rb_left) {
		subtree_last = rb_entry(node->rb.rb_left, struct anon_vma_chain, rb)->rb_subtree_last;

		if (max < subtree_last) max = subtree_last;
	}
	if (node->rb.rb_right) {
		subtree_last = rb_entry(node->rb.rb_right, struct anon_vma_chain, rb)->rb_subtree_last;
		if (max < subtree_last) max = subtree_last;
	}
	return max;
}

static inline void __anon_vma_interval_tree_augment_propagate(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct anon_vma_chain *node = rb_entry(rb, struct anon_vma_chain, rb);
		unsigned long augmented = __anon_vma_interval_tree_compute_subtree_last(node);

		if (node->rb_subtree_last == augmented)
			break;
		node->rb_subtree_last = augmented;
		rb = rb_parent(&node->rb);
	}
}

static inline void __anon_vma_interval_tree_augment_copy(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct anon_vma_chain *old = rb_entry(rb_old, struct anon_vma_chain, rb);
	struct anon_vma_chain *new = rb_entry(rb_new, struct anon_vma_chain, rb);

	new->rb_subtree_last = old->rb_subtree_last;
}

static void __anon_vma_interval_tree_augment_rotate(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct anon_vma_chain *old = rb_entry(rb_old, struct anon_vma_chain, rb);
	struct anon_vma_chain *new = rb_entry(rb_new, struct anon_vma_chain, rb);

	new->rb_subtree_last = old->rb_subtree_last;
	old->rb_subtree_last = __anon_vma_interval_tree_compute_subtree_last(old);
}

static const struct rb_augment_callbacks __anon_vma_interval_tree_augment = {
	.propagate = __anon_vma_interval_tree_augment_propagate,
	.copy = __anon_vma_interval_tree_augment_copy,
	.rotate = __anon_vma_interval_tree_augment_rotate
};

static inline void __anon_vma_interval_tree_insert(struct anon_vma_chain *node, struct rb_root *root)
{
	struct rb_node **link = &root->rb_node, *rb_parent = NULL;
	unsigned long start = avc_start_pgoff(node), last = avc_last_pgoff(node);
	struct anon_vma_chain *parent;

	while (*link) {
		rb_parent = *link;
		parent = rb_entry(rb_parent, struct anon_vma_chain, rb);
		if (parent->rb_subtree_last < last)
			parent->rb_subtree_last = last;
		if (start < avc_start_pgoff(parent))
			link = &parent->rb.rb_left;
		else
			link = &parent->rb.rb_right;
	}

	node->rb_subtree_last = last;
	rb_link_node(&node->rb, rb_parent, link);
	rb_insert_augmented(&node->rb, root, &__anon_vma_interval_tree_augment);
}

static inline void __anon_vma_interval_tree_remove(struct anon_vma_chain *node, struct rb_root *root)
{
	rb_erase_augmented(&node->rb, root, &__anon_vma_interval_tree_augment);
}

static struct anon_vma_chain * __anon_vma_interval_tree_subtree_search(struct anon_vma_chain *node, unsigned long start, unsigned long last)
{
	while (true) {
		if (node->rb.rb_left) {
			struct anon_vma_chain *left = rb_entry(node->rb.rb_left, struct anon_vma_chain, rb);

			if (start <= left->rb_subtree_last) {
				node = left;
				continue;
			}
		}

		if (avc_start_pgoff(node) <= last) {
			if (start <= avc_last_pgoff(node))
				return node;
			if (node->rb.rb_right) {
				node = rb_entry(node->rb.rb_right, struct anon_vma_chain, rb);
				if (start <= node->rb_subtree_last)
					continue;
			}
		}

		return NULL;
	}
}

static inline struct anon_vma_chain * __anon_vma_interval_tree_iter_first(struct rb_root *root, unsigned long start, unsigned long last)
{
	struct anon_vma_chain *node;

	if (!root->rb_node)
		return NULL;
	node = rb_entry(root->rb_node, struct anon_vma_chain, rb);
	if (node->rb_subtree_last < start)
		return NULL;
	return __anon_vma_interval_tree_subtree_search(node, start, last);
}

static inline struct anon_vma_chain * __anon_vma_interval_tree_iter_next(struct anon_vma_chain *node, unsigned long start, unsigned long last)
{
	struct rb_node *rb = node->rb.rb_right, *prev;

	while (true) {
		if (rb) {
			struct anon_vma_chain *right = rb_entry(rb, struct anon_vma_chain, rb);

			if (start <= right->rb_subtree_last)
				return __anon_vma_interval_tree_subtree_search(right, start, last);
		}
		do {
			rb = rb_parent(&node->rb);
			if (!rb)
				return NULL;
			prev = &node->rb;
			node = rb_entry(rb, struct anon_vma_chain, rb);
			rb = node->rb.rb_right;
		} while (prev == rb);

		if (last < avc_start_pgoff(node))
			return NULL;
		else if (start <= avc_last_pgoff(node))
			return node;
	}
}
