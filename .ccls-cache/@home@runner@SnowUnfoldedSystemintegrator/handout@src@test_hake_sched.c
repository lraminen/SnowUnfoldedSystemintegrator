/* FEEL FREE TO EDIT THIS FILE
 * - test_hake_sched.c (Hake Scheduler Library Code)
 * - Copyright of Starter Code: Prof. Kevin Andrea, George Mason University.  All Rights Reserved
 * - Copyright of Student Code: You!
 * - Date: Aug 2023
 *
 *   Framework to demonstrate how you can write a Unit Tester for your Project
 *   - Not a requirement to use, but it may be helpful if you like.
 *   - This only has one example of a test case in it, so it is just to show how to write them.
 *
 *   This lets you test your hake_sched.c code WITHOUT any of the TRILBY code running.
 *
 *   Why run all tests every time instead of checking a function once to see that it works?
 *   - You could break existing code when adding a new feature.
 *   - Test files like this are useful to check all of the tests every time, so you can make sure
 *     you didn't break something that used to work when you add new code.
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* Local Includes */
#include "hake_sched.h" // Your header for the functions you're testing.
#include "vm_support.h" // Gives ABORT_ERROR, PRINT_WARNING, PRINT_STATUS, PRINT_DEBUG commands

/* Globals (static means it's private to this file only) */
int g_debug_mode = 1; // Hardcodes debug on for the custom print functions
 
/* Local Prototypes */
void test_hake_create();
static void test_queue_initialized(Hake_queue_s *queue);

/* This is an EXAMPLE tester file, change anything you like!
 * - This shows an example by testing hake_create.
 * - To make this useful, modify this function or create others for the rest of the
 *   functions that you want to test from your code.
 */
int main() {
  g_debug_mode = 1;  // These helper functions only print if g_debug_mode is 1

  // Here I'm Calling a local helper function to test your hake_create code.
  // - You can make any helper functions you like to test it however you want!
  
  // PRINT_STATUS is a helper macro to print a message when you run the code.
  // - The definitions are in int/vm_support.h
  // - It works just like printf, but it prints out in some nice colors.
  // - It also adds a newline at the end, so you don't need to here.
  // - The PRINT_* macros are available for your hake_sched.c as well, if you want to use them.
  PRINT_STATUS("Test 1: Testing OP Create");
  test_hake_create();

  // You would add more calls to testing helper functions that you like.
  // Then when done, you can print a nice message an then return.
  PRINT_STATUS("All tests complete!");
  return 0;
}

/* Local function to test schedule_create from hake_sched.c */
void test_hake_create() {
  /* PRINT_DEBUG, PRINT_STATUS, PRINT_WARNING, PRINT_ERROR, and ABORT_ERROR are all 
   *   helper macros that we wrote to print messages to the screen during debugging.
   * - Feel free to use these, or not, based on your own preferences.
   * - These work just like printf, but print different labels and they add in a newline at the end.
   * BTW: gdb works muuuuuch better in this test file than it does in vm
   */
  PRINT_DEBUG("...Calling hake_create()");

  /* Note: MARK is a really cool helper macro.  Like the others, it works just like printf.
   * It also prints out the file, function, and line number that it was called from.
   * You can use it to check values throuhgout the code and mark where you printed from.
   * BTW: gdb also does this much better too.
   */
  MARK("I can be used anywhere, even if debug mode is off.\n");
  MARK("I work just like printf! %s %d %lf\n", "Cool!", 42, 3.14);

  // Begin Testing
  // - You can just call the function you want to test with any arguments needed.
  // - You may need to set up some arguments first, depending on what you want to test.
  Hake_schedule_s *header = hake_create();
  
  // Now that we called it and got the pointer to the header, let's test to see if we did it right!
  if(header == NULL) {
    // ABORT_ERROR will kill the program when it hits, which is good if you hit a bug.
    ABORT_ERROR("...hake_create returned NULL!"); 
  }
  // Header is good, so let's test the queues to see if they're all initialized properly.
  PRINT_STATUS("...Checking the Ready Queue");
  test_queue_initialized(header->ready_queue); // This is another helper function I wrote in this file.
  PRINT_STATUS("...Checking the Suspended Queue");
  test_queue_initialized(header->suspended_queue); // This is another helper function I wrote in this file.
  PRINT_STATUS("...Checking the Terminated Queue");
  test_queue_initialized(header->terminated_queue); // This is another helper function I wrote in this file.

  // Last example code, how to print the schedule of all three linked lists.
  PRINT_STATUS("...Printing the Schedule");
  // You can use print_hake_debug to print out all of the schedules.
  // - Note: the second argument is for the Process on the CPU
  //         When testing, if you select a process, you can pass a pointer to that into the
  //           second argument to have it print out nice.
  // - In this case, we haven't selected any Process, so passing in NULL
  print_hake_debug(header, NULL); // the second argument here is used for the process on the CPU.
  PRINT_STATUS("...hake_create is looking good so far.");
}

/* Helper function to test if a queue is properly initialized
 * Exits the program with ABORT_ERROR on any failures.
 */
static void test_queue_initialized(Hake_queue_s *queue) {
  // Always test a pointer before dereferencing it!
  if(queue == NULL) {
    ABORT_ERROR("...tried to test a NULL queue!");
  }

  // Check that the head pointer is NULL.
  if(queue->head != NULL) {
    ABORT_ERROR("...the Queue doesn't have a NULL head pointer!");
  }
  // Check that the count is 0.
  if(queue->count != 0) {
    ABORT_ERROR("...the Queue's count is not initialized to 0!");
  }
}
