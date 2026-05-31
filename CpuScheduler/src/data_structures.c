#include "job.h"
#include "data_structures.h"

int tree_insert(data_structure* das, void* data)
{
	if (das->end == das->len)
		return 0;

	das->arr[das->end] = das->arr[0];
	das->arr[0] = data;
	das->end++;

	_tree_update_root(das);

	return 1;
}

int tree_pop_min(data_structure* das, void* data)
{

}
int tree_pop_max(data_structure* das, void* data)
{
	if (das->end == 0)
		return 0;

	data = das->arr[0];
	das->arr[0] = das->arr[das->end - 1];
	das->end--;

	_tree_update_root(das);

	return 1;
}
int tree_peek(data_structure* das, void* data)
{
	if (das->end == 0)
		return 0;

	data = das->arr[0];

	return 1;
}
void* tree_search(data_structure* das, void* target)
{

}

void _tree_update_root(data_structure* das)
{
	int index = 0;

	while (1) {
		int bigger_index = index;
		if (index * 2 < das->end && das->compare(das->arr+(index * 2), das->arr+(bigger_index)) > 0)
			bigger_index = index * 2;
		if (index * 2 + 1 < das->end && das->compare(das->arr+(index * 2 + 1), das->arr+(bigger_index)) > 0)
			bigger_index = index * 2 + 1;

		if (bigger_index == index)
			return;

		void* tmp = das->arr[index];
		das->arr[index] = das->arr[bigger_index];
		das->arr[bigger_index] = tmp;
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

int round_queue_pop(data_structure* das, void* data)
{
	if (das->start == das->end)
		return 0;

	data = das->arr[das->start];
	das->start = (das->start + 1) % das->len;

	return 1;
}



void schedule_result_add(schedule_result* sch, int arr_index, int time, event_type type)
{
	schedule_result_part* new_part = (schedule_result_part*)malloc(sizeof(schedule_result_part));
	new_part->arr_index = arr_index;
	new_part->time = time;
	new_part->type = type;

	if (!sch->last)
		sch->last = new_part;
	else
		sch->last->next = new_part;

	if (!sch->start)
		sch->start = sch->last;
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