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
  Hake_schedule_s *schedule = (Hake_schedule_s *)malloc(sizeof(Hake_schedule_s));
  // Check if memory allocation was successful
    if (schedule == NULL) {
        perror("Failed to allocate memory for Hake_schedule_s");
        return NULL;
    }
   // Initialize ready_queue
    schedule->ready_queue = (Hake_queue_s *)malloc(sizeof(Hake_queue_s));
    if (schedule->ready_queue == NULL) {
        perror("Failed to allocate memory for ready_queue");
        free(schedule);
        return NULL;
    }
  schedule->suspended_queue = (Hake_queue_s *)malloc(sizeof(Hake_queue_s));
    if (schedule->suspended_queue == NULL) {
        perror("Failed to allocate memory for terminated_queue");
        free(schedule);
        return NULL;
    }
   // Initialize terminated_queue
    schedule->terminated_queue = (Hake_queue_s *)malloc(sizeof(Hake_queue_s));
    if (schedule->terminated_queue == NULL) {
        perror("Failed to allocate memory for terminated_queue");
        free(schedule);
        return NULL;
    }
  // Initialize the counts to 0 for each queue
    schedule->ready_queue->count = 0;
    schedule->suspended_queue->count = 0;
    schedule->terminated_queue->count = 0;

    // Initialize the head pointers to NULL for each queue
    schedule->ready_queue->head = NULL;
    schedule->suspended_queue->head = NULL;
    schedule->terminated_queue->head = NULL;

    return schedule;
}

/* Allocate and Initialize a new Hake_process_s with the given information.
 * - Malloc and copy the command string, don't just assign it!
 * Follow the project documentation for this function.
 * - You may assume all arguments are Legal and Correct for this Function Only
 * Returns a pointer to the Hake_process_s on success or a NULL on any error.
 */
Hake_process_s *hake_new_process(char *command, pid_t pid, int priority, int is_critical) {
   Hake_process_s *new_process = (Hake_process_s *)malloc(sizeof(Hake_process_s));
  // Check if memory allocation was successful
    if (new_process == NULL) {
        perror("Failed to allocate memory for Hake_process_s");
        return NULL; // Return NULL on error
    }
      // Initialize the state with the Ready State bit set to 1 and the Critical bit set accordingly
    new_process->state = (is_critical ? 0x88000000 : 0x80000000);
   // Initialize priority, age, and pid
    new_process->priority = priority;
    new_process->age = 0;
    new_process->pid = pid;

    // Allocate memory for the cmd member and copy the command
    new_process->cmd = (char *)malloc(strlen(command) + 1);
    if (new_process->cmd == NULL) {
        perror("Failed to allocate memory for cmd");
        free(new_process);
        return NULL; // Return NULL on error
    }
    strncpy(new_process->cmd, command, strlen(command) + 1);
   // Initialize the next member to NULL
    new_process->next = NULL;

    return new_process;
}

/* Inserts a process into the Ready Queue (singly linked list).
 * Follow the project documentation for this function.
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Returns a 0 on success or a -1 on any error.
 */
int hake_insert(Hake_schedule_s *schedule, Hake_process_s *process) {
 // Set the Ready State bit of the state member to 1
    // (Keep other state bits unchanged)
    process->state &= 0xFFFFFFFF
    process->state |= 0x80000000;
    // Insert the Process Node in Ascending PID Order
    Hake_queue_s *ready_queue = schedule->ready_queue;
    Hake_process_s *current = ready_queue->head;
    Hake_process_s *prev = NULL;

    while (current != NULL && current->pid < process->pid) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        // Insert at the beginning of the Ready Queue
        process->next = ready_queue->head;
        ready_queue->head = process;
    } else {
        // Insert after 'prev'
        prev->next = process;
        process->next = current;
    }

    // Update the Ready Queue's count member
    ready_queue->count++;

    return 0; // Return 0 on success
}

/* Returns the number of items in a given Hake Queue (singly linked list).
 * Follow the project documentation for this function.
 * Returns the number of processes in the list or -1 on any errors.
 */
int hake_get_count(Hake_queue_s *queue) {
  if (queue == NULL) {
        // Check for NULL pointer
        return -1; // Return -1 on error
    }

    // Return the count from the queue's count member
    return queue->count;
}

/* Selects the best process to run from the Ready Queue (singly linked list).
 * Follow the project documentation for this function.
 * Returns a pointer to the process selected or NULL if none available or on any errors.
 * - Do not create a new process to return, return a pointer to the SAME process removed.
 */
Hake_process_s *hake_select(Hake_schedule_s *schedule) {
  if (schedule == NULL || schedule->ready_queue == NULL || schedule->ready_queue->head == NULL) {
        // Check for NULL pointers or an empty Ready Queue
        return NULL; // Return NULL on error or if the Ready Queue is empty
    }
  // Initialize variables to find the best process
    Hake_process_s *best_process = NULL;
    Hake_process_s *current = schedule->ready_queue->head;
    Hake_process_s *prev = NULL;
    Hake_process_s *best_prev = NULL;

    Hake_process_s  critical = NULL;
    Hake_process_s  starving = NULL;
  while (current != NULL) {
        // Check if the process is critical or starving
        if (current->state & 0x40000000) {
            critical = current;
            if (best_process == NULL || critical == NULL || current->pid < critical->pid) {
                best_process = current;
                best_prev = prev;
            }
        } else if (current->age >= STARVING_AGE) {
            starving = current;
          if (critical == NULL && (best_process == NULL || starving == NULL || current->pid < starving->pid)) {
                best_process = current;
                best_prev = prev;
            }
        }
        else {
            if (critical == NULL && starving == NULL && (best_process == NULL || current->priority < best_process->priority ||
                (current->priority == best_process->priority && current->pid < best_process->pid))) {
                best_process = current;
                best_prev = prev;
            }
        }
        // Move to the next process
        prev = current;
        current = current->next;
  }
    if (best_process != NULL) {
        // Remove the best process from the Ready Queue
        if (best_prev != NULL) {
            best_prev->next = best_process->next;
        } else {
            schedule->ready_queue->head = best_process->next;
        }

        // Set the chosen process' age to 0, state to Running, and next to NULL
        best_process->age = 0;
        best_process->state = (best_process->state & 0xFFFFFFF) | 0x40000000;
        best_process->next = NULL;
  // Increment the age member for all remaining processes in the Ready Queue
        current = schedule->ready_queue->head;
        while (current != NULL) {
            current->age++;
            current = current->next;
        }

        // Return a pointer to the chosen process
        return best_process;
    }

    return NULL; // Return NULL if no suitable process was found
      
            
         

}

/* Move the process with matching pid from Ready to Suspended Queue.
 * Follow the specification for this function.
 * - Do not create a copy of the process in the Suspended Queue.  
 * - Insert the SAME process removed from the Ready Queue to the Suspended Queue
 * Returns a 0 on success or a -1 on any error (such as process not found).
 */
int hake_suspend(Hake_schedule_s *schedule, pid_t pid) {
   if (schedule == NULL || schedule->ready_queue == NULL || schedule->suspended_queue == NULL) {
        // Check for NULL pointers
        return -1; // Return -1 on error
    }

    // Search for the process in the Ready Queue or suspend the first process
    Hake_process_s *process_to_suspend = NULL;
    Hake_process_s *prev = NULL;
   if (pid == 0) {
        // Suspend the first (head) process in the Ready Queue
        process_to_suspend = schedule->ready_queue->head;
        if (process_to_suspend != NULL) {
            schedule->ready_queue->head = process_to_suspend->next;
        }
    } else {
        // Search for the process with the given PID in the Ready Queue
        Hake_process_s *current = schedule->ready_queue->head;

        while (current != NULL) {
            if (current->pid == pid) {
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    schedule->ready_queue->head = current->next;
                }
                process_to_suspend = current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    if (process_to_suspend != NULL) {
        // Set the Suspended State bit of the state member to 1
        process_to_suspend->state = (process_to_suspend->state & 0xFFFFFFF) | 0x20000000;

        // Set the next of the removed process to NULL
        process_to_suspend->next = NULL;

        // Insert the suspended process in ascending PID order to the Suspended Queue
        Hake_process_s *current = schedule->suspended_queue->head;
        Hake_process_s *prev = NULL;

        while (current != NULL && current->pid < process_to_suspend->pid) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            // Insert at the beginning of the Suspended Queue
            process_to_suspend->next = schedule->suspended_queue->head;
            schedule->suspended_queue->head = process_to_suspend;
        } else {
            // Insert after 'prev'
            prev->next = process_to_suspend;
            process_to_suspend->next = current;
        }

        return 0; // Return 0 on success
    }

    return -1; // Return -1 if the process was not found
}

/* Move the process with matching pid from Suspended to Ready Queue.
 * Follow the specification for this function.
 * - Do not create a copy of the process in the Ready Queue.  
 * - Insert the SAME process removed from the Suspended Queue to the Ready Queue
 * Returns a 0 on success or a -1 on any error (such as process not found).
 */
int hake_resume(Hake_schedule_s *schedule, pid_t pid) {
  if (schedule == NULL || schedule->ready_queue == NULL || schedule->suspended_queue == NULL) {
        // Check for NULL pointers
        return -1; // Return -1 on error
    }

    // Search for the process in the Suspended Queue or resume the first process
    Hake_process_s *process_to_resume = NULL;
    Hake_process_s *prev = NULL;

    if (pid == 0) {
        // Resume the first (head) process in the Suspended Queue
        process_to_resume = schedule->suspended_queue->head;
        if (process_to_resume != NULL) {
            schedule->suspended_queue->head = process_to_resume->next;
        }
    } else {
        // Search for the process with the given PID in the Suspended Queue
        Hake_process_s *current = schedule->suspended_queue->head;

        while (current != NULL) {
            if (current->pid == pid) {
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    schedule->suspended_queue->head = current->next;
                }
                process_to_resume = current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    if (process_to_resume != NULL) {
        // Set the Ready State bit of the state member to 1
        process_to_resume->state = (process_to_resume->state & 0xFFFFFFF) | 0x80000000;

        // Set the next of the removed process to NULL
        process_to_resume->next = NULL;

        // Insert the resumed process in ascending PID order to the Ready Queue
        Hake_process_s *current = schedule->ready_queue->head;
        Hake_process_s *prev = NULL;

        while (current != NULL && current->pid < process_to_resume->pid) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            // Insert at the beginning of the Ready Queue
            process_to_resume->next = schedule->ready_queue->head;
            schedule->ready_queue->head = process_to_resume;
        } else {
            // Insert after 'prev'
            prev->next = process_to_resume;
            process_to_resume->next = current;
        }

        return 0; // Return 0 on success
    }

    return -1; // Return -1 if the process was not found
}

/* This is called when a process exits normally that was just Running.
 * Put the given node into the Terminated Queue and set the Exit Code 
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Follow the project documentation for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int hake_exited(Hake_schedule_s *schedule, Hake_process_s *process, int exit_code) {
   if (schedule == NULL || schedule->terminated_queue == NULL || process == NULL) {
        // Check for NULL pointers
        return -1; // Return -1 on error
    }

    // Set the Terminated State bit of the state member to 1
    process->state = (process->state & 0xFFFFFFF) | 0x10000000;

    // Set the lower 27 bits of the state member to the exit_code
    process->state = (process->state & 0xFF000000) | (exit_code & 0x07FFFFFF);

    // Insert the process into the Terminated Queue in ascending PID order
    Hake_process_s *current = schedule->terminated_queue->head;
    Hake_process_s *prev = NULL;

    while (current != NULL && current->pid < process->pid) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        // Insert at the beginning of the Terminated Queue
        process->next = schedule->terminated_queue->head;
        schedule->terminated_queue->head = process;
    } else {
        // Insert after 'prev'
        prev->next = process;
        process->next = current;
    }

    return 0; // Return 0 on success
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
  if (schedule == NULL || schedule->terminated_queue == NULL) {
        // Check for NULL pointers
        return -1; // Return -1 on error
    }

    // Search for the process in the Ready Queue or Suspended Queue
    Hake_process_s *process_to_terminate = NULL;
    Hake_process_s *prev = NULL;

    // Search in the Ready Queue
    Hake_process_s *current = schedule->ready_queue->head;
    while (current != NULL) {
        if (current->pid == pid) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                schedule->ready_queue->head = current->next;
            }
            process_to_terminate = current;
            break;
        }
        prev = current;
        current = current->next;
    }

    // If not found in Ready Queue, search in the Suspended Queue
    if (process_to_terminate == NULL) {
        current = schedule->suspended_queue->head;
        prev = NULL;
        while (current != NULL) {
            if (current->pid == pid) {
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    schedule->suspended_queue->head = current->next;
                }
                process_to_terminate = current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    if (process_to_terminate != NULL) {
        // Set the Terminated State bit of the state member to 1
        process_to_terminate->state = (process_to_terminate->state & 0x40000000) | 0x10000000;

        // Set the lower 27 bits of the state member to the exit_code
        process_to_terminate->state = (process_to_terminate->state & 0xF8000000) | (exit_code & 0x07FFFFFF);

        // Insert the terminated process in ascending PID order to the Terminated Queue
        current = schedule->terminated_queue->head;
        prev = NULL;

        while (current != NULL && current->pid < process_to_terminate->pid) {
            prev = current;
            current = current->next;
        }

        if (prev == NULL) {
            // Insert at the beginning of the Terminated Queue
            process_to_terminate->next = schedule->terminated_queue->head;
            schedule->terminated_queue->head = process_to_terminate;
        } else {
            // Insert after 'prev'
            prev->next = process_to_terminate;
            process_to_terminate->next = current;
        }

        return 0; // Return 0 on success
    }

    return -1; // Return -1 if the PID was not found
}

/* Frees all allocated memory in the Hake_schedule_s, all of the Queues, and all of their Nodes.
 * Follow the project documentation for this function.
 * Returns void.
 */
void hake_deallocate(Hake_schedule_s *schedule)
{
   if (schedule == NULL) {
        return; // Return if the schedule is already NULL
    }

    // Free all Nodes from the Ready Queue
    Hake_process_s *current = schedule->ready_queue->head;
    while (current != NULL) {
        Hake_process_s *next = current->next;
        free(current->cmd); // Free the cmd String
        free(current); // Free the process node
        current = next;
    }

    // Free all Nodes from the Suspended Queue
    current = schedule->suspended_queue->head;
    while (current != NULL) {
        Hake_process_s *next = current->next;
        free(current->cmd); // Free the cmd String
        free(current); // Free the process node
        current = next;
    }

    // Free all Nodes from the Terminated Queue
    current = schedule->terminated_queue->head;
    while (current != NULL) {
        Hake_process_s *next = current->next;
        free(current->cmd); // Free the cmd String
        free(current); // Free the process node
        current = next;
    }

    // Free the Queues
    free(schedule->ready_queue);
    free(schedule->suspended_queue);
    free(schedule->terminated_queue);

    // Free the Hake Schedule
    free(schedule);
}
