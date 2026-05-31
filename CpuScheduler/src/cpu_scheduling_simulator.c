#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "data_structures.h"
#include "job.h"
#include "input_handler.h"
#include "settings.h"
#include "scheduling_methods.h"

#define SCHEDULING_METHOD_AMOUNT (sizeof(scheduling_methods) / sizeof(scheduling_methods[0]))

Job jobs[JOB_QUEUE_SIZE];
data_structure new_queue = { .compare = job_compare };
data_structure ready_queue = { .compare = job_compare };
data_structure waiting_queue = { .compare = job_compare };
Job* cur_job = 0;

int process_amount = 0;

scheduling_method scheduling_methods[] = {
    {"FCFS", 0, &add_to_ready_queue_round_queue, &schedule_round_queue, &should_preempty_none},
    {"SJF-nonpreemptive", 0, &sjf_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"SJF-preemptive", 1, &sjf_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"Priority-nonpreemptive", 0, &priority_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"Priority-preemptive", 1, &priority_add_to_ready_queue, &schedule_tree, &should_preempty_none},
    {"RR", 0, &add_to_ready_queue_round_queue, &schedule_round_queue, &rr_should_preempty},
};

void reset()
{
    memset(jobs, 0, sizeof(jobs));
    memset(&new_queue, 0, sizeof(new_queue));
    memset(&ready_queue, 0, sizeof(ready_queue));
    memset(&waiting_queue, 0, sizeof(waiting_queue));
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

        jobs[i].first_priority = &(jobs[i].first_arrival_time);

        tree_insert(&new_queue, &(jobs[i]));
    }

    qsort(jobs, process_amount, sizeof(Job), job_pid_compare);

    for (int i = 0; i < process_amount; i++)
    {
        jobs[i].arr_index = i;
    }
}
schedule_result schedule(scheduling_method method)
{
    schedule_result sch;
    sch.min_run_time = ~0 >> 1; //max int

    int cur_time = 0;
    while (new_queue.len || ready_queue.len || waiting_queue.len)
    {
        Job* data = 0;
        //put processes that should arrive
        while (tree_peek(&new_queue, data)) {
            if (data->next_arrival_time > cur_time)
                break;
            method.add_to_ready_queue(&ready_queue, data);
            tree_pop_max(&new_queue, data);
            schedule_result_add(&sch, data->pid, cur_time, ARRIVE);
        }
        //put processes that should arrive
        while (tree_peek(&waiting_queue, data)) {
            if (data->next_arrival_time > cur_time)
                break;
            method.add_to_ready_queue(&ready_queue, data);
            tree_pop_max(&waiting_queue, data);
            schedule_result_add(&sch, data->pid, cur_time, ARRIVE);
        }

        //remove processes that has ended
        if (cur_job) {
            cur_job->left_burst_time--;
            if (cur_job->left_burst_time <= cur_time - cur_job->run_start_time)
            {
                sch.min_run_time = min(sch.min_run_time, cur_time - cur_job->run_start_time);
                schedule_result_add(&sch, cur_job->pid, cur_time, END);
                schedule_result_add(&sch, cur_job->pid, cur_time, LEAVE);
                cur_job = 0;
            }
            else if (cur_job->next_io_index < cur_job->io_amount
                && cur_job->left_burst_time <= cur_time - cur_job->io_start_times[cur_job->next_io_index])
            {
                cur_job->next_arrival_time = cur_time + cur_job->io_burst_times[cur_job->next_io_index];
                cur_job->first_priority = &(cur_job->next_arrival_time);
                tree_insert(&waiting_queue, cur_job);
                sch.min_run_time = min(sch.min_run_time, cur_time - cur_job->run_start_time);
                schedule_result_add(&sch, cur_job->pid, cur_time, LEAVE);
                cur_job->next_io_index++;
                cur_job = 0;
            }
        }

        if (method.is_preemptive || !cur_job || method.should_preempty(cur_time, cur_job))
        {
            Job* next_job = method.schedule(&waiting_queue, &sch, cur_job > 0);
            if (next_job)
            {
                if (cur_job)
                {
                    method.add_to_ready_queue(&ready_queue, cur_job);
                    schedule_result_add(&sch, cur_job->pid, cur_time, END);
                }
                cur_job = next_job;
                cur_job->run_start_time = cur_time;
                schedule_result_add(&sch, cur_job->pid, cur_time, START);
            }
        }

        cur_time++;
    }
}
evaluation_result evaluation(schedule_result sch_result)
{
    evaluation_result result = {0};

    int first_arrive_time[PROCESS_MAX] = { -1 };
    int last_wait_start_time[PROCESS_MAX] = { -1 };
    int process_amount = 0;
    int max_index = -1;
    
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
                process_amount++;
                first_arrive_time[index] = time;
                max_index = max(max_index, index);
            }
            //since both arrive and end have to set last_wait_start_time, no break; is intended, not a mistake
        case END:
            last_wait_start_time[index] = time;
            break;
        case LEAVE:
            //leave caused by io doesn't trigger end event. end+leave means the process has finished its work.
            if (last_wait_start_time[index] == time)
                job_result->turnaround_time = time - first_arrive_time[index];
            break;
        case START:
            job_result->waiting_time += time - last_wait_start_time[index];
            break;
        }

        part = part->next;
    }


    long long total_waiting_time = 0;
    long long total_turnaround_time = 0;

    for (int i = 0; i <= max_index; i++)
    {
        total_waiting_time += result.job_eval_result[i].waiting_time;
        total_turnaround_time += result.job_eval_result[i].turnaround_time;
    }

    result.avg.waiting_time = total_waiting_time / (double)process_amount;
    result.avg.turnaround_time = total_turnaround_time / (double)process_amount;

    return result;
}






int main()
{
    //config();
    while (1) {
        char menu = get_int_input(0, 3);
        char submenu;
        int seed;
        schedule_result sch_result;
        evaluation_result eval_result;
        evaluation_result eval_results[SCHEDULING_METHOD_AMOUNT];
        Job* processes;

        switch (menu) {
        case 1: //change seed and create processes
            seed = input_int_once(time(NULL));
            srand(seed);
            processes = setup_processes();
            break;
        case 2: //schedule
            submenu = get_int_input(1, SCHEDULING_METHOD_AMOUNT+1) - 1;
            sch_result = schedule(scheduling_methods[submenu]);
            eval_result = evaluation(sch_result);
            print_gantt(sch_result);
            print_eval_result(eval_result);

            break;
        case 3: //Perform all and print evaluation
            for (int i = 0; i < SCHEDULING_METHOD_AMOUNT; i++) {
                eval_results[i] = evaluation(schedule(scheduling_methods[i]));
            }
            print_eval_results(eval_results, SCHEDULING_METHOD_AMOUNT);
        case 0: //exit
            return 0;
        }
    }
    return 0;
}





/*
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
*/