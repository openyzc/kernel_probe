# 1 "rbtree_augmented.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "rbtree_augmented.c"

static inline void
rb_insert_augmented(struct rb_node *node, struct rb_root *root,
      const struct rb_augment_callbacks *augment)
{
	__rb_insert_augmented(node, root, augment->rotate);
}

static long vma_compute_subtree_gap(struct vm_area_struct *vma)
{
	unsigned long max, prev_end, subtree_gap;

	/*
	 * Note: in the rare case of a VM_GROWSDOWN above a VM_GROWSUP, we
	 * allow two stack_guard_gaps between them here, and when choosing
	 * an unmapped area; whereas when expanding we only require one.
	 * That's a little inconsistent, but keeps the code here simpler.
	 */
	max = vm_start_gap(vma);
	if (vma->vm_prev) {
		prev_end = vm_end_gap(vma->vm_prev);
		if (max > prev_end)
			max -= prev_end;
		else
			max = 0;
	}
	if (vma->vm_rb.rb_left) {
		subtree_gap = rb_entry(vma->vm_rb.rb_left,
				struct vm_area_struct, vm_rb)->rb_subtree_gap;
		if (subtree_gap > max)
			max = subtree_gap;
	}
	if (vma->vm_rb.rb_right) {
		subtree_gap = rb_entry(vma->vm_rb.rb_right,
				struct vm_area_struct, vm_rb)->rb_subtree_gap;
		if (subtree_gap > max)
			max = subtree_gap;
	}
	return max;
}

# 45 "rbtree_augmented.c"
static inline void vma_gap_callbacks_propagate(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct vm_area_struct *node = rb_entry(rb, struct vm_area_struct, vm_rb);
		unsigned long augmented = vma_compute_subtree_gap(node);

		if (node->rb_subtree_gap == augmented)
			break;
		node->rb_subtree_gap = augmented;
		rb = rb_parent(&node->vm_rb);
	}
}

static inline void vma_gap_callbacks_copy(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old, struct vm_area_struct, vm_rb);
	struct vm_area_struct *new = rb_entry(rb_new, struct vm_area_struct, vm_rb);

	new->rb_subtree_gap = old->rb_subtree_gap;
}

static void vma_gap_callbacks_rotate(struct rb_node *rb_old, struct rb_node *rb_new)
{
	struct vm_area_struct *old = rb_entry(rb_old, struct vm_area_struct, vm_rb);
	struct vm_area_struct *new = rb_entry(rb_new, struct vm_area_struct, vm_rb);

	new->rb_subtree_gap = old->rb_subtree_gap;
	old->rb_subtree_gap = vma_compute_subtree_gap(old);
}

static const struct rb_augment_callbacks vma_gap_callbacks = {
	.propagate = vma_gap_callbacks_propagate,
	.copy = vma_gap_callbacks_copy,
	.rotate = vma_gap_callbacks_rotate
};


static void vma_gap_update(struct vm_area_struct *vma)
{
	vma_gap_callbacks_propagate(&vma->vm_rb, NULL);
}

static inline void vma_rb_insert(struct vm_area_struct *vma,
     struct rb_root *root)
{
	validate_mm_rb(root, NULL);
	rb_insert_augmented(&vma->vm_rb, root, &vma_gap_callbacks);
}
