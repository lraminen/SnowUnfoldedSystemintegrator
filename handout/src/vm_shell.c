/* DO NOT EDIT THIS FILE
 * - vm_shell.c (Trilby VM)
 * - Copyright: Prof. Kevin Andrea, George Mason University.  All Rights Reserved
 * - Date: Aug 2023
 *
 *   User Interaction Shell for the Trilby VM Environment
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* Linux System API Includes */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
/* Trilby VM Includes */
#include "vm.h"
#include "vm_support.h"
#include "vm_process.h"
#include "vm_printing.h"
#include "vm_cs.h"

/* Local Definitions */

/* Built-In Commands */
enum builtin_commands {
  QUIT, EXIT, HELP, DEBUG, START, STOP, SUSPEND, RESUME,
  SCHEDULE, STATUS, TERMINATE, DELAYTIME, RUNTIME,
  NUM_BUILTINS
};
static char *builtin_commands[] = {
  "quit", "exit", "help", "debug", "start", "stop", "suspend", "resume", 
  "schedule", "status", "terminate", "delaytime", "runtime"
};

/* Local Prototypes */
static int get_user_input(char *line);
static void execute_builtin(Process_data_s *data);
static void run_quit(Process_data_s *data);
static void run_debug();
static void run_start();
static void run_stop();
static void run_suspend(Process_data_s *data);
static void run_resume(Process_data_s *data);
static void run_terminate(Process_data_s *data);
static void run_delaytime(Process_data_s *data);
static void run_runtime(Process_data_s *data);
static void execute_command(Process_data_s *data);
static int builtin_string_to_enum(char *str);
static int is_builtin(char *str);
static pid_t extract_pid(char *str);
static suseconds_t extract_time(char *str);
static void print_process_data(Process_data_s *data);
static int is_whitespace(char *str);
static void print_help();
static Process_data_s *initialize_data(const char *str);
static Process_data_s *parse_input(char *str);

/* Run the Virtual System with User Shell Access */
void shell() {
  char buffer[MAX_CMD_LINE] = {0};
  Process_data_s *proc_data = NULL;
  int ret = 0;

  print_cs_status();
  PRINT_STATUS( "Debug Mode: %s", g_debug_mode?"On":"Off");
  PRINT_STATUS( "Type help for reference on TRILBY's Built-In Commands.");
  
  // No conditions here. The program will either crash or the user will type quit to break the loop.
  while(1) {
    print_prompt();

    // Step 1: Get the raw user input
    // Check for any errors getting input.
    do {
      ret = get_user_input(buffer);
      // Check for errors on input
      if(ret != 0) {
        // Special case where the error was interrupted by a signal, try again.
        if(ret == EINTR) {
          continue;
        }
        // On all other cases of input failure, abort program.
        ABORT_ERROR("Error on getting user input.");
      }
    } while(ret != 0);

    // Step 2: Parse the User Input
    proc_data = parse_input(buffer);
    // If there was an issue parsing it, ignore the input and get a new command
    if(proc_data == NULL) {
      continue;
    }

    // Only prints if DEBUG mode is ON
    print_process_data(proc_data);

    // Step 3: Execute the Input
    if(is_builtin(proc_data->cmd)) {
      // The input was a built-in command; run the function.
      execute_builtin(proc_data);
      // Free Command (if Built-In)
      free_data_proc(proc_data);
      proc_data = NULL;
    }
    // Otherwise, it's a program to run (Command).
    else {
      // Step 4: Add the Command to the Jobs Tracker then Execute It
      execute_command(proc_data);
    }
  }
  
  return;
}

/* Executes a Trilby Built-In Instruction */
static void execute_builtin(Process_data_s *data) {
  // If this was called with invalid inputs, ignore the input and return.
  if(data == NULL || data->cmd == NULL) {
    return;
  }

  // Convert command string to enum
  int builtin_cmd = builtin_string_to_enum(data->cmd);
  
  // Execute the builtin command
  switch(builtin_cmd) {
    case QUIT: // Fallthrough to Exit
    case EXIT:  run_quit(data);           break;
    case HELP:  print_help();             break; // Self-contained action.
    case DEBUG: run_debug();              break;
    case START: run_start();              break;
    case STOP:  run_stop();               break;
    case SUSPEND: run_suspend(data);      break;
    case RESUME: run_resume(data);        break;
    case SCHEDULE: print_schedule(get_schedule(), get_on_cpu()); break; // Self-contained action.
    case STATUS: print_cs_status();       break; // Self-contained action.
    case TERMINATE: run_terminate(data);  break;
    case DELAYTIME: run_delaytime(data);  break;
    case RUNTIME: run_runtime(data);      break;
    default: // This should never happen, but if it does, assume user entered something wrong.
      print_help();   
  }

  // All commands simply return from here.
}

/* Handle the built-ins QUIT or EXIT */
static void run_quit(Process_data_s *data) {
  PRINT_STATUS("Exiting Program.");
  free_data_proc(data);
  exit(EXIT_SUCCESS);
}

/* Handle the built-in for DEBUG mode toggling */
static void run_debug() {
  // This toggles the global controlling the debug mode
  g_debug_mode = (g_debug_mode)?0:1; // Toggles 1->0 or 0->1
  PRINT_STATUS( "Debug Mode: %s", g_debug_mode?"On":"Off");
  }

/* Handle the built-in for START */
static void run_start() {
  // Starts the CS System
  print_start_cs();
  start_cs();
}

/* Handle the built-in for STOP */
static void run_stop() {
  // Stops the CS System
  print_stop_cs();
  stop_cs();
}

/* Handle the built-in for SUSPEND */
static void run_suspend(Process_data_s *data) {
  // Get PID from Arguments
  pid_t pid = extract_pid(data->argv[1]);

  // If no argument, use 0 (which is a code for the default action)
  if(pid == -1) {
    pid = 0;
  }

  // Finally, suspend that process
  cs_suspend(pid);
}

/* Handle the built-in for RESUME */
static void run_resume(Process_data_s *data) {
  // Get PID from Arguments
  pid_t pid = extract_pid(data->argv[1]);

  // If no argument, use 0 (which is a code for the default action)
  if(pid == -1) {
    pid = 0;
  }

  // Finally, resume that process
  cs_resume(pid);
}

/* Handle the built-in for TERMINATE */
static void run_terminate(Process_data_s *data) {
  // Get PID from Arguments
  pid_t pid = extract_pid(data->argv[1]);

  // Terminate the Process immediately (if PID is valid)
  if(pid > 1) {
    kill(pid, SIGKILL);
  }
  else {
    PRINT_WARNING("You need a valid pid.\n\teg. terminate 35962");
    return;
  }
}

/* Change the Runtime Quantum (how long each process runs for before Switching) */
static void run_runtime(Process_data_s *data) {
  // Get time from Arguments
  suseconds_t time = extract_time(data->argv[1]);

  // Set the runtime if valid
  if(time >= SLEEP_MIN_USEC && time <= SLEEP_MAX_USEC) {
    set_run_usec(time);
  }
  // If 0 was given, reset to the default value
  else if(time == 0) {
    set_run_usec(SLEEP_USEC);
  }
  // Otherwise, provide the user some help.
  else {
    PRINT_WARNING("You need a valid time in usec or 0 for Default.\n\teg. runtime %d", SLEEP_USEC);
    PRINT_INFO("The minimum runtime allowed is %d usec", SLEEP_MIN_USEC);
    PRINT_INFO("The maximum runtime allowed is %d usec", SLEEP_MAX_USEC);
    PRINT_INFO("The current runtime is %d usec", get_run_usec());
    return;
  }
}

/* Change the Delay (how long to sleep between each process running) */
static void run_delaytime(Process_data_s *data) {
  // Get time from Arguments
  suseconds_t time = extract_time(data->argv[1]);

  // Set the runtime if valid
  if(time >= BETWEEN_MIN_USEC && time <= BETWEEN_MAX_USEC) {
    set_run_usec(time);
  }
  // If 0 was given, reset to the default value
  else if(time == 0) {
    set_run_usec(BETWEEN_USEC);
  }
  // Otherwise, provide the user some help.
  else {
    PRINT_WARNING("You need a valid time in usec or 0 for Default.\n\teg. delaytime %d", BETWEEN_USEC);
    PRINT_INFO("The minimum runtime allowed is %d usec", BETWEEN_MIN_USEC);
    PRINT_INFO("The maximum runtime allowed is %d usec", BETWEEN_MAX_USEC);
    PRINT_INFO("The current delaytime is %d usec", get_between_usec());
    return;
  }
}

/* Executes a local (or /usr/bin) command */
static void execute_command(Process_data_s *data) {
  // Creates the process and loads it into the Ready Queue
  create_process(data);
}

/* Gets line from user, ensures is valid, and strips the newline from it.
 * - Returns ERRNO code, -1 on other errors, or 0 on success
 */
static int get_user_input(char *line) {
  memset(line, 0, MAX_CMD_LINE); // Clear out the current command line

  // Get a new input from the user and check for errors
  errno = 0; // Reset to ensure we're getting the right code on an error
  char *p_line = fgets(line, MAX_CMD_LINE, stdin);
  if(errno) {
    return errno;
  }

  // Basic catch-all for failure
  if(p_line == NULL) {
    return -1;
  }

  // Convert the first newline (left behind by fgets) to a null terminator.
  strtok(line, "\n");

  return 0; // The line was processed by reference, so changes don't need to be returned
}

/* Convert a built-in command string to the enum.
 * On failure, interpret command as Help.
 */
static int builtin_string_to_enum(char *str) {
  if(str == NULL) {
    return HELP;
  }

  // Iterate all builtin commands to look for a matching string.  
  // The index of the matching string will be the corresponding enum value we want!
  for(int cmd = 0; cmd < NUM_BUILTINS; cmd++) {
    if(strncmp(builtin_commands[cmd], str, strlen(builtin_commands[cmd])) == 0) {
      return cmd;
    }
  }

  // No match was found, so we'll help them by interpreting their input as a help
  return HELP;
}

/* Returns 1 if the data entered is a Built-In command for Trilby-VM */
static int is_builtin(char *str) {
  if(str == NULL) {
    return 0;
  }

  // Iterate to find str in the list of builtins
  for(int cmd = 0; cmd < NUM_BUILTINS; cmd++) {
    if(strncmp(builtin_commands[cmd], str, strlen(builtin_commands[cmd])) == 0) {
      return 1;
    }
  }

  // Nothing found that matched, so not a built-in
  return 0;
}

/* Extracts and validates PID from a string.  
 * Returns -1 (invalid PID, but legal pid_t) on error.
 */
static pid_t extract_pid(char *str) {
  // Check for valid PID to terminate
  if(str == NULL || is_whitespace(str)) {
    return -1;
  }

  // Convert the PID
  errno = 0;
  char *pid_str = str;
  pid_t pid = (pid_t)strtol(str, &pid_str, 10);
  if(*pid_str == '\0') {
    return pid;
  }
  else {
    return -1;
  }
}

/* Extracts and validates a Time in usec from a string.  
 * Returns -1 on error. (suseconds_t is min range [-1,1000000])
 */
static suseconds_t extract_time(char *str) {
  // Check for valid time in microseconds
  if(str == NULL || is_whitespace(str)) {
    return -1;
  }

  // Convert the entered time to microseconds
  errno = 0;
  char *p_time = str;
  suseconds_t time = (suseconds_t)strtol(str, &p_time, 10);
  // Apply the new runtime if valid.
  if(*p_time == '\0') {
    return time;
  }
  else {
    return -1;
  }
}

/* Prints out the Command Information */
static void print_process_data(Process_data_s *data) {
  if(g_debug_mode == 0 || data == NULL) {
    return;
  }
  
  PRINT_DEBUG(".----------------------------");
  PRINT_DEBUG( "| [Input: %s]", data->input_orig);
  PRINT_DEBUG( "| - [PID: %d]", data->pid);
  PRINT_DEBUG( "| - [CMD: %s]", data->cmd);
  PRINT_DEBUG( "| - [Priority: %d]", data->priority_level);
  PRINT_DEBUG( "| - [Is Critical: %s]", data->is_critical?"Yes":"No");
  for(int i = 0; i < MAX_ARGS && data->argv[i] != NULL; i++) {
    PRINT_DEBUG( "| - [Arg %2d: %s]", i, data->argv[i]);
  }
  PRINT_DEBUG(".----------------------------");

  return;
}

/* Parse user command input, return NULL if empty */
static Process_data_s *parse_input(char *str) {
  if(str == NULL || strlen(str) <= 0 || is_whitespace(str)) {
    return NULL;
  }

  // Step 1: Initialize the user data struct
  Process_data_s *data = initialize_data(str);
  if(data == NULL) {
    ABORT_ERROR("Failed to Allocate Memory for new Command String");
  }

  // Step 2: Extract Command
  char *p_tok = strtok(data->input_toks, " "); 
  data->cmd = p_tok;  // Guaranteed in-scope as it's pointing to data->input_toks
  data->is_critical = 0; // Initialize to Non-Critical
  data->priority_level = 0; // Default Priority Level
  data->argv[0] = data->cmd;
  
  // Optionally restrict commands to local directory binaries only (set in inc/vm_settings.h)
  // - This, of course, doesn't look for the location of the command, but instead ensures
  //   that you can't use absolute paths or relative path adjustments to break out of local.
#if LOCAL_CMDS_ONLY > 0
  if(strchr(data->cmd, '/')) {
    PRINT_WARNING("Only Local Commands Are Allowed.");
    if(strstr(data->cmd, "./")) {
      PRINT_STATUS("Note: ./ is not needed for local commands.");
    }
    free_data_proc(data);
    return NULL;
  }
#endif

  // Step 3: Populate Arguments
  int arg = 1;
  data->is_critical = 0;  // Without -c, non-critical process
  data->priority_level = DEFAULT_PRIORITY; // Without -p #, will be default

  // Iterate through all input tokens to perform the population
  do {
    p_tok = strtok(NULL, " ");
    if(p_tok != NULL) {
      // Look for the critical flag
      if(strncmp(p_tok, "-c", 2) == 0) {
        data->is_critical = 1;
      }
      // Look for the priority level flag (and subsequent level value)
      else if(strncmp(p_tok, "-p", 2) == 0) {
        p_tok = strtok(NULL, " ");
        data->priority_level = strtol(p_tok, NULL, 10);
        // Validate the returned value and normalize to within bounds
        if(data->priority_level < MIN_PRIORITY) {
          data->priority_level = MIN_PRIORITY;
        }
        else if(data->priority_level > MAX_PRIORITY) {
          data->priority_level = MAX_PRIORITY;
        }
      }      
      // Must be an argument if not a flag, so add it to the list of args
      else {
        data->argv[arg++] = p_tok; // All pointers reference data->input_toks
      }
    }
    // Repeat until we're out of tokens (words) the user entered
  } while(p_tok != NULL);

  return data;
}

/* Initializes a clean input data structure */
static Process_data_s *initialize_data(const char *str) {
  Process_data_s *data = calloc(1, sizeof(Process_data_s));
  if(data == NULL) {
    return NULL;
  }

  // Initialize all members to NULL (0)
  data->cmd = NULL;  // Will point to the binary name/builtin-command
  memset(data->argv, 0, sizeof(data->argv)); // Array of char pointers, one per argument
  strncpy(data->input_orig, str, MAX_CMD); // Never strtok this directly.
  strncpy(data->input_toks, str, MAX_CMD); // This you strtok.
  data->pid = 0;     // For safety, this should never be -1 (if you kill -1, you kill all owned processes)
  data->next = NULL; // For the Jobs Queue Membership

  return data;
}

/* Return 1 if the string is entirely whitespace */
static int is_whitespace(char *str) {
  int i = 0;
  // If any character in the string is non whitespace, return 0
  for(i = 0; i < strlen(str); i++) {
    switch(str[i]) {
      case ' ':  // Space
      case '\n': // Newline
      case '\t': // Tab
      case '\r': // Carriage Return (what is this, DOS?)
        break;
      default: // Any non-whitespace character is found, return 0 immediately
        return 0;
    }
  }
  // Entire string is nothing but whitespace
  return 1;
}

static void print_help() {
  PRINT_STATUS( ".------[HELP]------");
  PRINT_STATUS( "| help        Prints out this reference.");
  PRINT_STATUS( "| start       Starts the CS Engine.");
  PRINT_STATUS( "| stop        Stops the CS Engine.");
  PRINT_STATUS( "| Ctrl-C      Toggle (Start/Stop) the CS Engine.");
  PRINT_STATUS( "| schedule    Prints out the Current State of all Queues.");
  PRINT_STATUS( "| terminate X Terminate Process with PID X.");
  PRINT_STATUS( "| status      Prints out the Current Settings.");
  PRINT_STATUS( "| debug       Toggles Debug Information.");
  PRINT_STATUS( "| runtime X   Sets the runtime to X usec.");
  PRINT_STATUS( "| delaytime X Sets the delaytime to X usec.");
  PRINT_STATUS( "| quit        Exits TRILBY-VM.");
  PRINT_STATUS( "+------------------");
  }

/*|**************************************************************
 *+----------- Hic Sunt Quisquiliae----------------------------+*
 **************************************************************V*/