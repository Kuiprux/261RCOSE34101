#pragma once
#include "job.h"
#include "settings.h"

typedef struct {
	void* arr[PROCESS_MAX];
	int start;
	int end;
	int len;
	int is_max_heap;
	int (*compare)(void*, void*);
}data_structure;

int tree_insert(data_structure* das, void* data);
int tree_pop_max(data_structure* das, void** data);
int tree_pop_min(data_structure* das, void** data);
int tree_peek(data_structure* das, void** data);
void _tree_update_root(data_structure* das);

int round_queue_push(data_structure* das, void* data);
int round_queue_pop(data_structure* das, void** data);


typedef enum {
	ARRIVE, LEAVE, START, END
} event_type;

typedef struct schedule_result_part {
	int arr_index;
	int time;
	event_type type;
	struct schedule_result_part* next;
}schedule_result_part;
typedef struct {
	schedule_result_part* start;
	schedule_result_part* last;

	int min_run_time;
}schedule_result;

void schedule_result_add(schedule_result* sch, int arr_index, int time, event_type type);
void schedule_result_free(schedule_result* sch);


typedef struct {
	int pid;
	double waiting_time;
	double turnaround_time;
}job_evaluation_result;
typedef struct {
	int arrived_count;
	job_evaluation_result job_eval_result[PROCESS_MAX];
	job_evaluation_result avg;
}evaluation_result;

typedef struct {
	char name[40];
	int is_preemptive;
	int ready_queue_is_max_heap;
	void (*add_to_ready_queue)(data_structure*, Job*);
	Job* (*schedule)(data_structure*, schedule_result*);
	int (*should_preempty)(int, Job*);
}scheduling_method;

extern scheduling_method scheduling_methods[];