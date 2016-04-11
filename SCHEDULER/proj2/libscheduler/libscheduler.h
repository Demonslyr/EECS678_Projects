/** @file libscheduler.h
 */

#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_

typedef int bool;
#define true 1;
#define false 0;

/**
  Constants which represent the different scheduling algorithms
*/

typedef struct job_t
{
  int job_number;
  int time;
  int running_time;
  int priority;
  int time_placed_in_queue;
  int placed_on_core;
  int first_placed_on_core;
  bool previously_scheduled;
} job_t;

typedef struct core_t
{
  job_t *current_job;
} core_t;

typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

#endif /* LIBSCHEDULER_H_ */
