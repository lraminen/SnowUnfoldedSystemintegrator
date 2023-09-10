/* This is the only file you will be editing.
 * - hake_sched.c (Hake Scheduler Library Code)
 * - Copyright of Starter Code: Prof. Kevin Andrea, George Mason University.  All Rights Reserved
 * - Copyright of Student Code: You!  
 * - Copyright of ASCII Art: [Modified from an Unknown Artist - https://www.asciiart.eu/animals/fish]
 * - Restrictions on Student Code: Do not post your code on any public site (eg. Github).
 * -- Feel free to post your code on a PRIVATE Github and give interviewers access to it.
 * -- You are liable for the protection of your code from others.
 * - Date: Aug 2023
 */

/* CS367 Project 1, Fall Semester, 2023
 * Fill in your Name, GNumber, and Section Number in the following comment fields
 * Name: Priya Ramineni
 * GNumber: G01090744
 * Section Number: CS367-005             (Replace the _ with your section number)
 */

/* hake CPU Scheduling Library
      /`·   ¸...¸
     /¸..\.´....¸`:.
 ¸.·´  ¸            `·.¸.·´)
: © ):´;   hake        ¸  {
 `·.¸ `·           ¸.·´\`·¸)
     `\\´´´´´´´´´´´\¸.·´
*/
 
/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
/* Unix System Includes */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
/* Local Includes */
#include "hake_sched.h"
#include "vm_support.h"
#include "vm_process.h"

/* Feel free to create any helper functions you like! */

/*** Hake Library API Functions to Complete ***/

/* Initializes the Hake_schedule_s Struct and all of the Hake_queue_s Structs
 * Follow the project documentation for this function.
 * Returns a pointer to the new Hake_schedule_s or NULL on any error.
 */
Hake_schedule_s *hake_create() {
  return NULL; // Replace This Line with Your Code!
}

/* Allocate and Initialize a new Hake_process_s with the given information.
 * - Malloc and copy the command string, don't just assign it!
 * Follow the project documentation for this function.
 * - You may assume all arguments are Legal and Correct for this Function Only
 * Returns a pointer to the Hake_process_s on success or a NULL on any error.
 */
Hake_process_s *hake_new_process(char *command, pid_t pid, int priority, int is_critical) {
  return NULL; // Replace This Line with Your Code!
}

/* Inserts a process into the Ready Queue (singly linked list).
 * Follow the project documentation for this function.
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Returns a 0 on success or a -1 on any error.
 */
int hake_insert(Hake_schedule_s *schedule, Hake_process_s *process) {
  return -1; // Replace This Line with Your Code!
}

/* Returns the number of items in a given Hake Queue (singly linked list).
 * Follow the project documentation for this function.
 * Returns the number of processes in the list or -1 on any errors.
 */
int hake_get_count(Hake_queue_s *queue) {
  return -1; // Replace This Line with Your Code!
}

/* Selects the best process to run from the Ready Queue (singly linked list).
 * Follow the project documentation for this function.
 * Returns a pointer to the process selected or NULL if none available or on any errors.
 * - Do not create a new process to return, return a pointer to the SAME process removed.
 */
Hake_process_s *hake_select(Hake_schedule_s *schedule) {
  return NULL; // Replace This Line with Your Code!
}

/* Move the process with matching pid from Ready to Suspended Queue.
 * Follow the specification for this function.
 * - Do not create a copy of the process in the Suspended Queue.  
 * - Insert the SAME process removed from the Ready Queue to the Suspended Queue
 * Returns a 0 on success or a -1 on any error (such as process not found).
 */
int hake_suspend(Hake_schedule_s *schedule, pid_t pid) {
  return -1; // Replace Me with Your Code!
}

/* Move the process with matching pid from Suspended to Ready Queue.
 * Follow the specification for this function.
 * - Do not create a copy of the process in the Ready Queue.  
 * - Insert the SAME process removed from the Suspended Queue to the Ready Queue
 * Returns a 0 on success or a -1 on any error (such as process not found).
 */
int hake_resume(Hake_schedule_s *schedule, pid_t pid) {
  return -1; // Replace Me with Your Code!
}

/* This is called when a process exits normally that was just Running.
 * Put the given node into the Terminated Queue and set the Exit Code 
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Follow the project documentation for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int hake_exited(Hake_schedule_s *schedule, Hake_process_s *process, int exit_code) {
  return -1; // Replace This Line with Your Code!
}

/* This is called when the OS terminates a process early. 
 * - This will either be in your Ready or Suspended Queue.
 * - The difference with hake_exited is that this process is in one of your Queues already.
 * Remove the process with matching pid from the Ready or Suspended Queue and add the Exit Code to it.
 * - You have to check both since it could be in either queue.
 * Follow the project documentation for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int hake_terminated(Hake_schedule_s *schedule, pid_t pid, int exit_code) {
  return -1; // Replace This Line with Your Code!
}

/* Frees all allocated memory in the Hake_schedule_s, all of the Queues, and all of their Nodes.
 * Follow the project documentation for this function.
 * Returns void.
 */
void hake_deallocate(Hake_schedule_s *schedule) {
  return; // Replace This Line with Your Code!
}
