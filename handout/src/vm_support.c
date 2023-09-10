/* DO NOT EDIT THIS FILE
 * - vm_support.c (Trilby VM)
 * - Copyright: Prof. Kevin Andrea, George Mason University.  All Rights Reserved
 * - Date: Aug 2023
 *
 *   Collection of common supporting functions for the Trilby VM
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
/* Linux System API Includes */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
/* Trilby Function Includes */
#include "vm_cs.h"
#include "vm_settings.h"
#include "vm_shell.h"
#include "vm_printing.h"
#include "vm_support.h"

/* Quickly registers a new signal with the given signal number and handler
 * Exits the program on errors with signal registration.
 */
void register_signal(int sig, void (*handler)(int)) {
  if(handler == NULL) {
    ABORT_ERROR("Handler Function Needed for Registration");
  }

  // Register the handler with the OS
  struct sigaction sa = {0};
  sa.sa_handler = handler;
  sigaction(sig, &sa, NULL);
}

/* Print the Virtual System Prompt
 * PROMPT configurable in inc/vm_settings.h
 */
void print_prompt() {
  printf("%s(PID: %d)%s %s%s%s ", BLUE, getpid(), RST, GREEN, PROMPT, RST);
  fflush(stdout);
}

/* Special Error that also immediately exits the program. */
void abort_error(char *msg, char *src) {
  fprintf(stderr, "  %s[ERROR ] %s (File %s)%s\n", RED, msg, src, RST);
  fprintf(stderr, "  %s         Terminating Program%s\n", RED, RST);
  exit(EXIT_FAILURE);
}

/* Prints out the Opening Banner */
void print_trilby_banner() {
  printf(
" %sGreen Imperial HEx:%s %sTRILBY Task Manager v1.5a%s %s*TRIAL EXPIRED*%s\n%s"
"              [o]              \n"
"     ._________|__________.    \n"
"     |%s     .________.     %s|    \n"
"     |%s    /          \\    %s|    \n"
"     |%s   |____________|   %s|    \n"
"     |%s   |____________|   %s|    \n"
"     |%s \\================/ %s|    \n"
"     |                    |    \n"
"     | %s[TRILBY-VM] $%s _    |\n"
"     .____________________.    \n"
"              [  ]             \n"
"       ___________________  \n"
"    _-'.-.-.-.-. .-.-.-.-.`-_    \n" // Keyboard Derived from Dan Strychalski's Work
"  _'.-.-.-------------.-.-.-.`-_ \n" // Original Keyboard at https://www.asciiart.eu/computers/keyboards
" :------------------------------:%s\n",
  GREEN, RST, BLUE, RST, RED, RST, YELLOW,
  MAGENTA, YELLOW,
  MAGENTA, YELLOW,
  MAGENTA, YELLOW,
  MAGENTA, YELLOW,
  MAGENTA, YELLOW,
  GREEN, YELLOW,
  RST);
}

/* Prints the full Schedule of all processes being tracked. */
void print_hake_debug(Hake_schedule_s *schedule, Hake_process_s *on_cpu) {
  // If on_cpu is NULL, it won't be printed, but otherwise is legal
  
  // Only print if debug mode is enabled.
  if(g_debug_mode == 1) {
    print_schedule(schedule, on_cpu);
  }
}

/* Prints the full Schedule of all processes being tracked.
 * If on_cpu is NULL, simply won't print that anything is on CPU, but is legal */
void print_schedule(Hake_schedule_s *schedule, Hake_process_s *on_cpu) {
  // If no such schedule exists, print nothing.
  if(schedule == NULL) {
    return;
  }

  // Collect the number of processes in TRILBY
  int total_scheduled_processes = hake_get_count(schedule->ready_queue) + 
                                  hake_get_count(schedule->suspended_queue) + 
                                  hake_get_count(schedule->terminated_queue);
  PRINT_STATUS("Printing the current Status...");
  PRINT_STATUS("Running Process (Note: Processes run briefly, so this may almost always be empty.");

  // CPU - Add the On CPU Process to the count
  int count = 0;
  if(on_cpu != NULL) {
    count = 1;
  }

  // Now print all the information
  PRINT_STATUS("...[CPU Execution    - %2d Process%s]", count, count?"":"es");
  print_process_node(on_cpu);
  
  // Schedule
  PRINT_STATUS("Schedule - %d Processes across all Queues", total_scheduled_processes);
  // Ready Queue
  count = hake_get_count(schedule->ready_queue);
  PRINT_STATUS("...[Ready Queue      - %2d Process%s]", count, count==1?"":"es");
  print_hake_queue(schedule->ready_queue);
  // Suspended Queue
  count = hake_get_count(schedule->suspended_queue);
  PRINT_STATUS("...[Suspended Queue  - %2d Process%s]", count, count==1?"":"es");
  print_hake_queue(schedule->suspended_queue);
  // Terminated Queue
  count = hake_get_count(schedule->terminated_queue);
  PRINT_STATUS("...[Terminated Queue - %2d Process%s]", count, count==1?"":"es");
  print_hake_queue(schedule->terminated_queue);
}

/* Prints a single Scheduler Queue */
void print_hake_queue(Hake_queue_s *queue) {
  // If no such queue exists, print nothing.
  if(queue == NULL) {
    return;
  }

  // Iterate the queue and print each process
  Hake_process_s *walker = queue->head;
  while(walker != NULL) {
    print_process_node(walker);
    walker = walker->next;
  }
}

// Prints a schedule tracked process
void print_process_node(Hake_process_s *node) {
  // If no process exists, print nothing.
  if(node == NULL) {
    return;
  }

  // If Process has Terminated
  if((node->state >> 28)&1) {
    PRINT_STATUS("     [PID: %7d] %-15s ... Pri %3d, Flags: [%c%c%c%c%c], Age: %2d, Exit Code: %d",
        node->pid, node->cmd, node->priority, 
        ((node->state>>31)&1)?'R':' ',
        ((node->state>>30)&1)?'U':' ',
        ((node->state>>29)&1)?'S':' ',
        ((node->state>>28)&1)?'T':' ',
        ((node->state>>27)&1)?'C':' ',
        node->age,
        ((node->state)&((1<<27)-1)));
  }
  // If Process has not Terminated Yet
  else {
    PRINT_STATUS("     [PID: %7d] %-15s ...Pri %3d, Flags: [%c%c%c%c%c], Age: %2d",
        node->pid, node->cmd, node->priority, 
        ((node->state>>31)&1)?'R':' ',
        ((node->state>>30)&1)?'U':' ',
        ((node->state>>29)&1)?'S':' ',
        ((node->state>>28)&1)?'T':' ',
        ((node->state>>27)&1)?'C':' ',
        node->age);
  }
}
