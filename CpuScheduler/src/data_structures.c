#include <stdlib.h>
#include "job.h"
#include "data_structures.h"

static void _tree_heapify_up(data_structure* das, int index)
{
	while (index > 0) {
		int parent = (index - 1) / 2;
		int cmp = das->compare(das->arr[index], das->arr[parent]);
		if (das->is_max_heap ? cmp <= 0 : cmp >= 0)
			return;

		void* tmp = das->arr[index];
		das->arr[index] = das->arr[parent];
		das->arr[parent] = tmp;
		index = parent;
	}
}

int tree_insert(data_structure* das, void* data)
{
	if (das->end == das->len)
		return 0;

	das->arr[das->end] = data;
	das->end++;

	_tree_heapify_up(das, das->end - 1);

	return 1;
}

int tree_pop_min(data_structure* das, void** data)
{
	int saved = das->is_max_heap;
	das->is_max_heap = 0;
	int result = tree_pop_max(das, data);
	das->is_max_heap = saved;
	return result;
}
int tree_pop_max(data_structure* das, void** data)
{
	if (das->end == 0)
		return 0;

	*data = das->arr[0];
	das->arr[0] = das->arr[das->end - 1];
	das->end--;

	_tree_update_root(das);

	return 1;
}
int tree_peek(data_structure* das, void** data)
{
	if (das->end == 0)
		return 0;

	*data = das->arr[0];

	return 1;
}
void* tree_search(data_structure* das, void* target)
{

}

void _tree_update_root(data_structure* das)
{
	int index = 0;

	while (1) {
		int better_index = index;
		int left = index * 2 + 1;
		int right = index * 2 + 2;
		if (left < das->end) {
			int cmp = das->compare(das->arr[left], das->arr[better_index]);
			if (das->is_max_heap ? cmp > 0 : cmp < 0)
				better_index = left;
		}
		if (right < das->end) {
			int cmp = das->compare(das->arr[right], das->arr[better_index]);
			if (das->is_max_heap ? cmp > 0 : cmp < 0)
				better_index = right;
		}

		if (better_index == index)
			return;

		void* tmp = das->arr[index];
		das->arr[index] = das->arr[better_index];
		das->arr[better_index] = tmp;
		index = better_index;
	}
}



int round_queue_push(data_structure* das, void* data)
{
	if (das->start == (das->end + 1) % das->len)
		return 0;

	das->arr[das->end] = data;
	das->end = (das->end + 1) % das->len;
	
	return 1;
}

int round_queue_pop(data_structure* das, void** data)
{
	if (das->start == das->end)
		return 0;

	*data = das->arr[das->start];
	das->start = (das->start + 1) % das->len;

	return 1;
}



void schedule_result_add(schedule_result* sch, int arr_index, int time, event_type type)
{
	schedule_result_part* new_part = (schedule_result_part*)malloc(sizeof(schedule_result_part));
	new_part->arr_index = arr_index;
	new_part->time = time;
	new_part->type = type;
	new_part->next = 0;

	if (!sch->last)
		sch->start = sch->last = new_part;
	else {
		sch->last->next = new_part;
		sch->last = new_part;
	}
}
void schedule_result_free(schedule_result* sch)
{
	schedule_result_part* cur_part = sch->start;
	schedule_result_part* next_part;
	while (cur_part)
	{
		next_part = cur_part->next;
		free(cur_part);
		cur_part = next_part;
	}

	sch->start = 0;
	sch->last = 0;
}
