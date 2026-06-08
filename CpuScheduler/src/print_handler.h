#pragma once

#include "data_structures.h"
#include "color_codes.h"
    
void print_gantt(Job* jobs, schedule_result* sch, int process_amount, ColorMode color_mode);

void print_eval_result(evaluation_result* result);
void print_eval_results(evaluation_result* results, int len);