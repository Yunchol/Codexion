#include "codexion.h"

static int	req_cmp(t_request *a, t_request *b, int sched)
{
	if (sched == CODEX_SCHED_EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline); /* earliest deadline */
		return (a->seq < b->seq); /* tie-break */
	}
	return (a->seq < b->seq); /* fifo order */
}

static void	swap_idx(t_heap *heap, int i, int j)
{
	t_request	*tmp;

	tmp = heap->items[i];
	heap->items[i] = heap->items[j];
	heap->items[j] = tmp;
	heap->items[i]->heap_index = i; /* update index */
	heap->items[j]->heap_index = j;
}

static void	heap_up(t_heap *heap, int idx, int sched)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!req_cmp(heap->items[idx], heap->items[parent], sched))
			break;
		swap_idx(heap, idx, parent); /* bubble up */
		idx = parent;
	}
}

static void	heap_down(t_heap *heap, int idx, int sched)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = idx * 2 + 1;
		right = idx * 2 + 2;
		smallest = idx;
		if (left < heap->count
			&& req_cmp(heap->items[left], heap->items[smallest], sched))
			smallest = left;
		if (right < heap->count
			&& req_cmp(heap->items[right], heap->items[smallest], sched))
			smallest = right;
		if (smallest == idx)
			break;
		swap_idx(heap, idx, smallest); /* bubble down */
		idx = smallest;
	}
}

int	heap_init(t_heap *heap, int cap)
{
	heap->items = (t_request **)malloc(sizeof(t_request *) * cap); /* alloc */
	if (!heap->items)
	{
		heap->count = 0;
		heap->cap = 0;
		return (0);
	}
	heap->count = 0;
	heap->cap = cap;
	return (1);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->items); /* free heap array */
	heap->items = NULL;
	heap->count = 0;
	heap->cap = 0;
}

void	heap_push(t_heap *heap, t_request *req, int sched)
{
	if (heap->count >= heap->cap)
		return; /* overflow guard */
	req->heap_index = heap->count;
	heap->items[heap->count] = req;
	heap->count++;
	heap_up(heap, req->heap_index, sched); /* restore order */
}

void	heap_remove(t_heap *heap, t_request *req, int sched)
{
	int	idx;

	idx = req->heap_index;
	if (idx < 0 || idx >= heap->count)
		return; /* not in heap */
	heap->count--;
	if (idx != heap->count)
	{
		heap->items[idx] = heap->items[heap->count];
		heap->items[idx]->heap_index = idx;
		heap_down(heap, idx, sched); /* fix down */
		heap_up(heap, idx, sched); /* fix up */
	}
	req->heap_index = -1; /* mark removed */
}

int	heap_is_top(t_heap *heap, t_request *req)
{
	if (heap->count == 0)
		return (0);
	return (heap->items[0] == req); /* top check */
}

t_request	*heap_peek(t_heap *heap)
{
	if (heap->count == 0)
		return (NULL);
	return (heap->items[0]); /* current top */
}
