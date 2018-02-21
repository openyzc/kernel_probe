
static inline unsigned long vma_interval_tree_compute_subtree_last(struct vm_area_struct *node)
{
	unsigned long max = vma_last_pgoff(node), subtree_last;

	if (node->shared.rb.rb_left) {
		subtree_last = rb_entry(node->shared.rb.rb_left, struct vm_area_struct, shared.rb)->shared.rb_subtree_last;

		if (max < subtree_last) max = subtree_last;
	}

	if (node->shared.rb.rb_right) {
		subtree_last = rb_entry(node->shared.rb.rb_right, struct vm_area_struct, shared.rb)->shared.rb_subtree_last;

		if (max < subtree_last) max = subtree_last;
	}
	return max;
}

static inline void vma_interval_tree_augment_propagate(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct vm_area_struct *node = rb_entry(rb, struct vm_area_struct, shared.rb);
		unsigned long augmented = vma_interval_tree_compute_subtree_last(node);

		if (node->shared.rb_subtree_last == augmented)
			break;

		node->shared.rb_subtree_last = augmented;
		rb = rb_parent(&node->shared.rb);
	}
}

static inline void vma_interval_tree_augment_copy(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old, struct vm_area_struct, shared.rb);
	struct vm_area_struct *new = rb_entry(rb_new, struct vm_area_struct, shared.rb);

	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
}

static void vma_interval_tree_augment_rotate(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old, struct vm_area_struct, shared.rb);
	struct vm_area_struct *new = rb_entry(rb_new, struct vm_area_struct, shared.rb);

	new->shared.rb_subtree_last = old->shared.rb_subtree_last;
	old->shared.rb_subtree_last = vma_interval_tree_compute_subtree_last(old);
}

static const struct rb_augment_callbacks vma_interval_tree_augment = {
	.propagate = vma_interval_tree_augment_propagate,
	.copy = vma_interval_tree_augment_copy,
	.rotate = vma_interval_tree_augment_rotate
};

void vma_interval_tree_insert(struct vm_area_struct *node, struct rb_root *root) {
	struct rb_node **link = &root->rb_node, *rb_parent = NULL;
	unsigned long start = vma_start_pgoff(node), last = vma_last_pgoff(node);
	struct vm_area_struct *parent;

	while (*link) {
		rb_parent = *link;
		parent = rb_entry(rb_parent, struct vm_area_struct, shared.rb);
		if (parent->shared.rb_subtree_last < last)
			parent->shared.rb_subtree_last = last;
		if (start < vma_start_pgoff(parent))
			link = &parent->shared.rb.rb_left;
		else
			link = &parent->shared.rb.rb_right;
	}
	node->shared.rb_subtree_last = last;
	rb_link_node(&node->shared.rb, rb_parent, link);
	rb_insert_augmented(&node->shared.rb, root, &vma_interval_tree_augment);
}

void vma_interval_tree_remove(struct vm_area_struct *node, struct rb_root *root)
{
	rb_erase_augmented(&node->shared.rb, root, &vma_interval_tree_augment);
}

static struct vm_area_struct * vma_interval_tree_subtree_search(struct vm_area_struct *node, unsigned long start, unsigned long last)
{
	while (true) {
		if (node->shared.rb.rb_left) {
			struct vm_area_struct *left = rb_entry(node->shared.rb.rb_left, struct vm_area_struct, shared.rb);

			if (start <= left->shared.rb_subtree_last) {
				node = left;
				continue;
			}
		}

		if (vma_start_pgoff(node) <= last) {
			if (start <= vma_last_pgoff(node)) return node;
			if (node->shared.rb.rb_right) {
				node = rb_entry(node->shared.rb.rb_right, struct vm_area_struct, shared.rb);
				if (start <= node->shared.rb_subtree_last)
					continue;
			}
		}
		return NULL;
	}
}

struct vm_area_struct * vma_interval_tree_iter_first(struct rb_root *root, unsigned long start, unsigned long last) {
	struct vm_area_struct *node;

	if (!root->rb_node)
		return NULL;

	node = rb_entry(root->rb_node, struct vm_area_struct, shared.rb);
	if (node->shared.rb_subtree_last < start)
		return NULL;

	return vma_interval_tree_subtree_search(node, start, last);
}

struct vm_area_struct * vma_interval_tree_iter_next(struct vm_area_struct *node, unsigned long start, unsigned long last)
{
	struct rb_node *rb = node->shared.rb.rb_right, *prev;

	while (true) {
		if (rb) {
			struct vm_area_struct *right = rb_entry(rb, struct vm_area_struct, shared.rb);

			if (start <= right->shared.rb_subtree_last)
				return vma_interval_tree_subtree_search(right, start, last);
		}
		do {
			rb = rb_parent(&node->shared.rb);
			if (!rb)
				return NULL;
			prev = &node->shared.rb;
			node = rb_entry(rb, struct vm_area_struct, shared.rb);
			rb = node->shared.rb.rb_right;
		} while (prev == rb);

		if (last < vma_start_pgoff(node))
			return NULL;
		else if (start <= vma_last_pgoff(node))
			return node;
	}
}
