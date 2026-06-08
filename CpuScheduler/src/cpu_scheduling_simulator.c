#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "data_structures.h"
#include "job.h"
#include "input_handler.h"
#include "settings.h"
#include "scheduling_methods.h"
#include "print_handler.h"
#include "color_codes.h"

#define SCHEDULING_METHOD_AMOUNT (sizeof(scheduling_methods) / sizeof(scheduling_methods[0]))

Job jobs[JOB_QUEUE_SIZE];
data_structure new_queue = { .compare = (int (*)(void*, void*))job_compare, .len = JOB_QUEUE_SIZE, .is_max_heap = 0 };
data_structure ready_queue = { .compare = (int (*)(void*, void*))job_compare, .len = JOB_QUEUE_SIZE, .is_max_heap = 1 };
data_structure waiting_queue = { .compare = (int (*)(void*, void*))job_compare, .len = JOB_QUEUE_SIZE, .is_max_heap = 0 };
Job* cur_job = 0;

int process_amount = 0;

scheduling_method scheduling_methods[] = {
    {"FCFS", 0, 0, &add_to_ready_queue_round_queue, &schedule_round_queue, &should_preempty_none},
    {"SJF-nonpreemptive", 0, 0, &sjf_add_to_ready_queue, &schedule_tree_min, &should_preempty_none},
    {"SJF-preemptive", 1, 0, &sjf_add_to_ready_queue, &schedule_tree_min, &should_preempty_none},
    {"Priority-nonpreemptive", 0, 1, &priority_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"Priority-preemptive", 1, 1, &priority_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"RR", 0, 0, &add_to_ready_queue_round_queue, &schedule_round_queue, &rr_should_preempty},
    {"Weighted RR", 0, 0, &add_to_ready_queue_round_queue, &schedule_round_queue, &wrr_should_preempty},
};

void reset()
{
    memset(jobs, 0, sizeof(jobs));
    new_queue.start = new_queue.end = 0;
    new_queue.len = JOB_QUEUE_SIZE;
    new_queue.is_max_heap = 0;
    new_queue.compare = (int (*)(void*, void*))job_compare;
    ready_queue.start = ready_queue.end = 0;
    ready_queue.len = JOB_QUEUE_SIZE;
    ready_queue.is_max_heap = 1;
    ready_queue.compare = (int (*)(void*, void*))job_compare;
    waiting_queue.start = waiting_queue.end = 0;
    waiting_queue.len = JOB_QUEUE_SIZE;
    waiting_queue.is_max_heap = 0;
    waiting_queue.compare = (int (*)(void*, void*))job_compare;
    cur_job = 0;
}
Job* setup_processes()
{
    reset();

    process_amount = rand() % (PROCESS_MAX - PROCESS_MIN + 1) + PROCESS_MIN;

    bool is_picked[PID_AVAILABLE_AMOUNT] = {false};
    for (int i = 0; i < process_amount; i++)
    {
        int pid_index = rand() % PID_AVAILABLE_AMOUNT;
        while (is_picked[pid_index])
            pid_index = (pid_index + 1) % PID_AVAILABLE_AMOUNT;
        is_picked[pid_index] = true;
        jobs[i].pid = pid_index + PID_MIN;

        jobs[i].first_arrival_time = rand() % (ARRIVAL_TIME_MAX - ARRIVAL_TIME_MIN + 1) + ARRIVAL_TIME_MIN;

        int io_amount = rand() % (IO_BURST_AMOUNT_MAX - IO_BURST_AMOUNT_MIN + 1) + IO_BURST_AMOUNT_MIN;
        jobs[i].io_amount = io_amount;
        for (int j = 0; j < io_amount; j++)
        {
            if (j > 0)
                jobs[i].io_start_times[j] = jobs[i].io_start_times[j - 1];
            jobs[i].io_start_times[j] += rand() % (IO_GAP_MAX - IO_GAP_MIN + 1) + IO_GAP_MIN;
            jobs[i].io_burst_times[j] += rand() % (IO_BURST_TIME_MAX - IO_BURST_TIME_MIN + 1) + IO_BURST_TIME_MIN;
        }

        if (io_amount > 0)
            jobs[i].burst_time = jobs[i].io_start_times[io_amount - 1];
        jobs[i].burst_time += rand() % (EXTRA_CPU_BURST_TIME_MAX - EXTRA_CPU_BURST_TIME_MIN + 1) + EXTRA_CPU_BURST_TIME_MIN;
        jobs[i].priority = rand() % (PRIORITY_MAX - PRIORITY_MIN + 1) + PRIORITY_MIN;
        jobs[i].time_quantum_multiplier = rand() % (TIME_QUANTUM_MULTIPLIER_MAX - TIME_QUANTUM_MULTIPLIER_MIN + 1) + TIME_QUANTUM_MULTIPLIER_MIN;

        jobs[i].next_arrival_time = jobs[i].first_arrival_time;
        jobs[i].left_burst_time = jobs[i].burst_time;
        jobs[i].first_priority = &(jobs[i].first_arrival_time);
        jobs[i].second_priority = &(jobs[i].pid);
    }

    qsort(jobs, process_amount, sizeof(Job), job_pid_compare);

    for (int i = 0; i < process_amount; i++)
    {
        jobs[i].arr_index = i;
    }

    return jobs;
}
schedule_result schedule(scheduling_method method)
{
    schedule_result sch = {0};
    sch.min_run_time = INT_MAX;

    cur_job = 0;
    new_queue.start = new_queue.end = 0;
    ready_queue.start = ready_queue.end = 0;
    waiting_queue.start = waiting_queue.end = 0;
    ready_queue.is_max_heap = method.ready_queue_is_max_heap;

    for (int i = 0; i < process_amount; i++)
    {
        jobs[i].next_arrival_time = jobs[i].first_arrival_time;
        jobs[i].left_burst_time = jobs[i].burst_time;
        jobs[i].next_io_index = 0;
        jobs[i].first_priority = &(jobs[i].first_arrival_time);
        jobs[i].second_priority = &(jobs[i].pid);
        tree_insert(&new_queue, &(jobs[i]));
    }

    int cur_time = 0;
    while (new_queue.end || ready_queue.start != ready_queue.end || waiting_queue.end || cur_job)
    {
        Job* data = 0;
        //put processes that should arrive
        while (tree_peek(&new_queue, (void**)&data)) {
            if (data->next_arrival_time > cur_time)
                break;
            method.add_to_ready_queue(&ready_queue, data);
            tree_pop_min(&new_queue, (void**)&data);
            schedule_result_add(&sch, data->arr_index, cur_time, ARRIVE);
        }
        //put processes that should arrive
        while (tree_peek(&waiting_queue, (void**)&data)) {
            if (data->next_arrival_time > cur_time)
                break;
            method.add_to_ready_queue(&ready_queue, data);
            tree_pop_min(&waiting_queue, (void**)&data);
            schedule_result_add(&sch, data->arr_index, cur_time, ARRIVE);
        }

        //remove processes that has ended
        if (cur_job) {
            cur_job->left_burst_time--;
            if (cur_job->left_burst_time <= 0)
            {
                sch.min_run_time = min(sch.min_run_time, cur_time - cur_job->run_start_time);
                schedule_result_add(&sch, cur_job->arr_index, cur_time, END);
                schedule_result_add(&sch, cur_job->arr_index, cur_time, LEAVE);
                cur_job = 0;
            }
            else if (cur_job->next_io_index < cur_job->io_amount
                && cur_job->left_burst_time <= cur_job->burst_time - cur_job->io_start_times[cur_job->next_io_index])
            {
                cur_job->next_arrival_time = cur_time + cur_job->io_burst_times[cur_job->next_io_index];
                cur_job->first_priority = &(cur_job->next_arrival_time);
                cur_job->second_priority = &(cur_job->pid);
                tree_insert(&waiting_queue, cur_job);
                sch.min_run_time = min(sch.min_run_time, cur_time - cur_job->run_start_time);
                schedule_result_add(&sch, cur_job->arr_index, cur_time, LEAVE);
                cur_job->next_io_index++;
                cur_job = 0;
            }
        }

        if (method.is_preemptive || !cur_job || method.should_preempty(cur_time, cur_job))
        {
            Job* next_job = 0;
            if (!cur_job || method.should_preempty(cur_time, cur_job))
                next_job = method.schedule(&ready_queue, &sch);
            else if (method.is_preemptive)
            {
                Job* ready_head = 0;
                if (tree_peek(&ready_queue, (void**)&ready_head))
                {
                    int should_switch = method.ready_queue_is_max_heap
                        ? job_compare(ready_head, cur_job) > 0
                        : job_compare(cur_job, ready_head) > 0;
                    if (should_switch)
                        next_job = method.schedule(&ready_queue, &sch);
                }
            }
            if (next_job)
            {
                if (cur_job)
                {
                    sch.min_run_time = min(sch.min_run_time, cur_time - cur_job->run_start_time);
                    method.add_to_ready_queue(&ready_queue, cur_job);
                    schedule_result_add(&sch, cur_job->arr_index, cur_time, END);
                }
                cur_job = next_job;
                cur_job->run_start_time = cur_time;
                schedule_result_add(&sch, cur_job->arr_index, cur_time, START);
            }
        }

        cur_time++;
    }

    return sch;
}
evaluation_result evaluation(schedule_result sch_result)
{
    evaluation_result result = {0};

    int first_arrive_time[PROCESS_MAX];
    int last_wait_start_time[PROCESS_MAX];
    int arrived_count = 0;
    int max_index = -1;

    memset(first_arrive_time, -1, sizeof(first_arrive_time));
    memset(last_wait_start_time, -1, sizeof(last_wait_start_time));

    for (int i = 0; i < PROCESS_MAX; i++) {
        result.job_eval_result[i].pid = -1;
    }
    
    schedule_result_part* part = sch_result.start;
    while (part)
    {
        int index = part->arr_index;
        int time = part->time;

        job_evaluation_result* job_result = &(result.job_eval_result[index]);

        switch (part->type)
        {
        case ARRIVE:
            if (first_arrive_time[index] < 0)
            {
                arrived_count++;
                first_arrive_time[index] = time;
                max_index = max(max_index, index);
                job_result->pid = jobs[index].pid;
            }
            //since both arrive and end have to set last_wait_start_time, no break; is intended, not a mistake
        case END:
            last_wait_start_time[index] = time;
            break;
        case LEAVE:
            //leave caused by io doesn't trigger end event. end+leave means the process has finished its work.
            if (last_wait_start_time[index] == time)
                job_result->turnaround_time = time - jobs[index].first_arrival_time;
            break;
        case START:
            job_result->waiting_time += time - last_wait_start_time[index];
            break;
        }

        part = part->next;
    }

    result.arrived_count = arrived_count;

    long long total_waiting_time = 0;
    long long total_turnaround_time = 0;

    for (int i = 0; i <= max_index; i++)
    {
        total_waiting_time += result.job_eval_result[i].waiting_time;
        total_turnaround_time += result.job_eval_result[i].turnaround_time;
    }

    int divisor = arrived_count > 0 ? arrived_count : 1;
    result.avg.waiting_time = total_waiting_time / (double)divisor;
    result.avg.turnaround_time = total_turnaround_time / (double)divisor;

    return result;
}






#ifndef EVAL_TEST
int main()
{
    ColorMode color_mode = COLOR_256;

    while (1) {
        printf("\nCPU Scheduling Simulator\n");
        printf("1. Change color mode\n");
        printf("2. Generate processes\n");
        printf("3. Print processes\n");
        printf("4. Schedule and Evaluate\n");
        printf("0. Exit\n");

        char menu = input_int_range("Select menu: ", 0, 4);
        int submenu;
        int seed;
        schedule_result sch_result;
        evaluation_result eval_result;
        evaluation_result eval_results[SCHEDULING_METHOD_AMOUNT];
        Job* processes;

        switch (menu) {
        case 1: //change color mode
            printf("\nColor Mode\n");
            printf("1. COLOR_8\n");
            printf("2. COLOR_16\n");
            printf("3. COLOR_256\n");
            printf("4. COLOR_TRUE\n");
            printf("0. Back\n");

            int new_color_mode = input_int_range("Select color mode: ", 0, 4) - 1;
            if(new_color_mode < 0)
                break;
            color_mode = new_color_mode;
            printf("New color mode: %d\n", color_mode);
            break;
        case 2: //generate processes
            printf("\nGenerate Processes\n");
            printf("Enter seed(default value: time(NULL)): ");
            seed = input_int_once(time(NULL));
            printf("New seed: %d\n", seed);
            srand(seed);
            setup_processes();
            printf("%d Processes generated\n", process_amount);
            break;
        case 3: //print processes
            printf("\nProcesses\n");
            //pid, first_arrival_time, burst_time, io(each start_time, burst_time), priority, time_quantum_multiplier
            printf("PID: Arrival Time, Burst Time, IO Amount, Priority, Time Quantum Multiplier\n");
            for (int i = 0; i < process_amount; i++) {
                printf("P%d: %d, %d, %d, %d, %d\n", jobs[i].pid, jobs[i].first_arrival_time, jobs[i].burst_time, jobs[i].io_amount, jobs[i].priority, jobs[i].time_quantum_multiplier);
                for (int j = 0; j < jobs[i].io_amount; j++) {
                    printf("  IO %d: Start Time: %d, Burst Time: %d\n", j, jobs[i].io_start_times[j], jobs[i].io_burst_times[j]);
                }
            }
            break;
        case 4: //schedule
            printf("\nScheduling Method\n");
            for(int method_index = 0; method_index < SCHEDULING_METHOD_AMOUNT; method_index++) {
                printf("%d. %s\n", method_index + 1, scheduling_methods[method_index].name);
            }
            printf("%d. All\n", SCHEDULING_METHOD_AMOUNT+1);
            printf("0. Back\n");

            submenu = input_int_range("Select scheduling method: ", 0, SCHEDULING_METHOD_AMOUNT+2) - 1;
            if(submenu < 0)
                break;
            if(submenu < SCHEDULING_METHOD_AMOUNT) {
                sch_result = schedule(scheduling_methods[submenu]);
                eval_result = evaluation(sch_result);
                printf("\n%s\n", scheduling_methods[submenu].name);
                print_gantt(jobs, &sch_result, process_amount, color_mode);
                print_eval_result(&eval_result);
            } else {
                for (int i = 0; i < SCHEDULING_METHOD_AMOUNT; i++) {
                    printf("\n%s\n", scheduling_methods[i].name);
                    sch_result = schedule(scheduling_methods[i]);
                    eval_results[i] = evaluation(sch_result);
                    print_gantt(jobs, &sch_result, process_amount, color_mode);
                }
                printf("\n");
                print_eval_results(eval_results, SCHEDULING_METHOD_AMOUNT);
            }

            break;
        case 0: //exit
            return 0;
        }
    }
    return 0;
}
#endif /* EVAL_TEST */




/*
05/12
구조체 사용: https://dream-and-develop.tistory.com/10
함수 선언: https://dojang.io/mod/page/view.php?id=1936

랜덤: https://yeolco.tistory.com/64

05/12
동적 메모리 할당: https://medium.com/@patdhlk/c-dynamic-data-structures-1-7e518b2868a0
printf %*s: https://fmyson.tistory.com/164
함수 포인터: https://dojang.io/mod/page/view.php?id=592

05/19
reset struct: https://stackoverflow.com/questions/6891720/initialize-reset-struct-to-zero-null

05/26
enum: https://dojang.io/mod/page/view.php?id=480
enum+switch: https://stackoverflow.com/questions/15237656/using-enum-type-in-a-switch-statement
terminal size: https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c
memset: https://goodbyefin.tistory.com/58#google_vignette
sprintf: https://velog.io/@hjqueeen/C%EC%96%B8%EC%96%B4sprintf-%ED%95%A8%EC%88%98-%EC%84%9C%EC%8B%9D%EC%9D%84-%EC%A7%80%EC%A0%95%ED%95%98%EC%97%AC-%EB%B0%B0%EC%97%B4-%ED%98%95%ED%83%9C%EB%A1%9C-%EB%AC%B8%EC%9E%90%EC%97%B4-%EB%A7%8C%EB%93%A4%EA%B8%B0
struct designated initializer(C99): https://stackoverflow.com/questions/330793/how-to-initialize-a-struct-in-accordance-with-c-programming-language-standards

05/30
qsort: https://learn.microsoft.com/ko-kr/cpp/c-runtime-library/reference/qsort?view=msvc-170
ansi color: https://en.wikipedia.org/wiki/ANSI_escape_code
viridis color scheme: https://github.com/sjmgarnier/viridisLite/blob/master/data-raw/optionD.csv
*/
