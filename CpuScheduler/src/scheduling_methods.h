#pragma once
#include "data_structures.h"


void add_to_ready_queue_round_queue(data_structure* ready_queue, Job* job);
Job* schedule_round_queue(data_structure* ready_queue, schedule_result* sch_result);
Job* schedule_tree(data_structure* ready_queue, schedule_result* sch_result);
int should_preempty_none(int cur_time, Job* job);

void sjf_add_to_ready_queue(data_structure* ready_queue, Job* job);
void priority_add_to_ready_queue(data_structure* ready_queue, Job* job);
int rr_should_preempty(int cur_time, Job* job);
