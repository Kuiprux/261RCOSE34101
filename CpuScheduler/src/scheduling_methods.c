#include "data_structures.h"

void add_to_ready_queue_round_queue(data_structure* ready_queue, Job* job)
{
	round_queue_push(ready_queue, job);
}
Job* schedule_round_queue(data_structure* ready_queue, schedule_result* sch_result)
{
	Job* next_job = 0;
	round_queue_pop(ready_queue, (void**)&next_job);

	return next_job;
}
Job* schedule_tree(data_structure* ready_queue, schedule_result* sch_result)
{
	Job* next_job = 0;
	tree_pop_max(ready_queue, (void**)&next_job);

	return next_job;
}
Job* schedule_tree_min(data_structure* ready_queue, schedule_result* sch_result)
{
	Job* next_job = 0;
	tree_pop_min(ready_queue, (void**)&next_job);

	return next_job;
}
int should_preempty_none(int cur_time, Job* job)
{
	return 0;
}

void sjf_add_to_ready_queue(data_structure* ready_queue, Job* job)
{
	job->first_priority = &(job->left_burst_time);
	job->second_priority = &(job->next_arrival_time);
	tree_insert(ready_queue, job);
}
void priority_add_to_ready_queue(data_structure* ready_queue, Job* job)
{
	job->first_priority = &(job->priority);
	job->second_priority = &(job->next_arrival_time);
	tree_insert(ready_queue, job);
}
int rr_should_preempty(int cur_time, Job* job)
{
	return cur_time - job->run_start_time >= TIME_QUANTUM;
}
int wrr_should_preempty(int cur_time, Job* job)
{
	return cur_time - job->run_start_time >= TIME_QUANTUM * job->time_quantum_multiplier;
}
