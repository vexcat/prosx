#ifndef _PROS_API_H_
#define _PROS_API_H_

#ifdef __cplusplus
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef bool bool_t;

/******************************************************************************/
/**                             RTOS FACILITIES                              **/
/**                                                                          **/
/**                                                                          **/
/** See https://pros.cs.purdue.edu/v5/tutorials/multitasking to learn more.  **/
/******************************************************************************/

#define TASK_PRIORITY_MAX 16
#define TASK_PRIORITY_MIN 1
#define TASK_PRIORITY_DEFAULT 8

#define TASK_STACK_DEPTH_DEFAULT 8192
#define TASK_STACK_DEPTH_MIN 256

#define TASK_NAME_MAX_LEN 32

typedef void* task_t;
typedef void (*task_fn_t)(void*);

typedef enum {
	E_TASK_STATE_RUNNING = 0,
	E_TASK_STATE_READY,
	E_TASK_STATE_BLOCKED,
	E_TASK_STATE_SUSPENDED,
	E_TASK_STATE_DELETED,
	E_TASK_STATE_INVALID
} task_state_e_t;

typedef enum {
	E_NOTIFY_ACTION_NONE,
	E_NOTIFY_ACTION_BITS,
	E_NOTIFY_ACTION_INCR,
	E_NOTIFY_ACTION_OWRITE,
	E_NOTIFY_ACTION_NO_OWRITE
} notify_action_e_t;

typedef void* sem_t;
typedef void* mutex_t;

/**
 * Refers to the current task handle
 */
#define CURRENT_TASK ((task_t)NULL)

/**
 * Returns the number of milliseconds since PROS initialized.
 */
uint32_t millis();

/**
 * Create a new task and add it to the list of tasks that are ready to run.
 *
 * @param out_handle
 *          Used to pass back a handle by which the newly created task can be
 *          referenced.
 * @param function
 *          Pointer to the task entry function
 * @param parameters
 *          Pointer to memory that will be used as a parameter for the task being
 *          created. This memory should not typically come from stack, but rather
 *          from dynamically (i.e., malloc'd) or statically allocated memory.
 * @param name
 *          A descriptive name for the task.  This is mainly used to facilitate
 *          debugging. The name may be up to 32 characters long.
 * @param prio
 *          The priority at which the task should run.
 *          TASK_PRIO_DEFAULT plus/minus 1 or 2 is typically used.
 * @param stack_depth
 *          The number of words (i.e. 4 * stack_depth) available on the task's
 *          stack. TASK_STACK_DEPTH_DEFAULT is typically sufficienct.
 * @return
 *          Will return a handle by which the newly created task can be
 *          referenced. If an error occurred, NULL will be returned and errno
 *          can be checked for hints as to why task_create failed.
 */
task_t task_create(task_fn_t function, void* parameters, uint8_t prio, uint16_t stack_depth, const char* name);

/**
 * Remove a task from the RTOS real time kernel's management.  The task being
 * deleted will be removed from all ready, blocked, suspended and event lists.
 *
 * Memory dynamically allocated by the task is not automatically freed, and
 * should be freed before the task is deleted.
 *
 * @param task
 *          The handle of the task to be deleted.  Passing NULL will cause the
 *          calling task to be deleted.
 */
int task_delete(task_t task);

/**
 * Delay a task for a given number of milliseconds.
 *
 * This is not the best method to have a task execute code at predefined
 * intervals, as the delay time is measured from when the delay is requested.
 * To delay cyclically, use task_delay_until().
 *
 * @param milliseconds
 *          The number of milliseconds to wait (1000 milliseconds per second)
 */
void task_delay(const unsigned long milliseconds);

/**
 * Delay a task until a specified time.  This function can be used by periodic
 * tasks to ensure a constant execution frequency.
 *
 * The task will be woken up at the time *prev_time + delta, and *prev_time will
 * be updated to reflect the time at which the task will unblock.
 *
 * @param prev_time
 *          A pointer to the location storing the setpoint time
 * @param delta
 *          The number of milliseconds to wait (1000 milliseconds per second)
 */
void task_delay_until(unsigned long* const prev_time, const unsigned long delta);

/**
 * Obtains the priority of the specified task.
 *
 * @param task
 *          The task to check
 * @return
 *          The priority of the task
 */
uint32_t task_get_priority(task_t task);

/**
 * Sets the priority of the specified task.
 *
 * If the specified task's state is available to be scheduled (e.g. not blocked)
 * and new priority is higher than the currently running task, a context switch
 * may occur.
 *
 * @param task
 *          The task to set
 * @param prio
 *          The new priority of the task
 */
void task_set_priority(task_t task, uint32_t prio);

/**
 * Obtains the state of the specified task.
 *
 * @param task
 *          The task to check
 * @return
 *          The state of the task
 */
task_state_e_t task_get_state(task_t task);

/**
 * Suspends the specified task, making it ineligible to be scheduled.
 *
 * @param task
 *          The task to suspend
 */
void task_suspend(task_t task);

/**
 * Resumes the specified task, making it eligible to be scheduled.
 *
 * @param task
 *          The task to resume
 */
void task_resume(task_t task);

/**
 * Returns the number of tasks the kernel is currently managing, including all
 * ready, blocked, or suspended tasks. A task that has been deleted, but not yet
 * reaped by the idle task will also be included in the count. Tasks recently
 * created may take one context switch to be counted.
 *
 * @return
 *          The number of tasks that are currently being managed by the kernel.
 */
uint32_t task_get_count();

/**
 * Obtains the name of the specified task.
 *
 * @param task
 *          The task to check
 * @return
 *          A pointer to the name of the task
 */
char const* task_get_name(task_t task);

/**
 * Obtains a task handle from the specified name
 *
 * The operation takes a relatively long time and should be used sparingly.
 *
 * @param name
 *          The name to query
 * @return
 *          A task handle with a matching name, or NULL if none were found.
 */
task_t task_get_by_name(char* name);

/**
 * Sends a notification to a task, optionally performing some action.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/notifications for details.
 *
 * @param task
 *          The task to notify
 * @param value
 *          The value used in performing the action
 * @param action
 *          An action to optionally perform on the receiving task's notification
 *          value
 * @return
 *          Dependent on the notification action.
 *          For NOTIFY_ACTION_NO_OWRITE: return 0 if the value could be written
 *          without needing to overwrite, 1 otherwise.
 *          For all other NOTIFY_ACTION values: always return 0
 */
uint32_t task_send(task_t task, uint32_t value, notify_action_e_t action);

/**
 * Sends a notification to a task, optionally performing some action. Will also
 * retrieve the value of the notification in the target task before modifying
 * the notification value.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/notifications for details.
 *
 * @param task
 *          The task to notify
 * @param value
 *          The value used in performing the action
 * @param action
 *          An action to optionally perform on the receiving task's notification
 *          value
 * @param   prev_value
 *          A pointer to store the previous value of the target task's notification
 * @return
 *          Dependent on the notification action.
 *          For NOTIFY_ACTION_NO_OWRITE: return 0 if the value could be written
 *          without needing to overwrite, 1 otherwise.
 *          For all other NOTIFY_ACTION values: always return 0
 */
uint32_t task_get_and_send(task_t task, uint32_t value, notify_action_e_t action, uint32_t* prev_value);

/**
 * Receives a notification and clears or decrements the value of the notification
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/notifications for details.
 *
 * @param clear_on_exit
 *          If true (1), then the notification value is cleared.
 *          If false (0), then the notification value is decremented.
 * @param timeout
 *          Specifies the amount of time to be spent waiting for a notification
 *          to occur. If 0, then the task waits forever.
 */
uint32_t task_recv(bool_t clear_on_exit, uint32_t timeout);

/**
 * Clears the notification for a task.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/notifications for details.
 *
 * @param task
 *          The task to clear
 * @return
 *          False if there was not a notification waiting, true if there was
 */
bool_t task_notify_clear(task_t task);

/**
 * Creates a mutex
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#mutexes for details.
 *
 * @return
 *          A handle to a newly created mutex
 */
mutex_t mutex_create();

/**
 * Takes and locks a mutex, waiting for up to a certain number of milliseconds
 * before timing out.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#mutexes for details.
 *
 * @param mutex
 *          Mutex to attempt to lock.
 * @param timeout
 *          Time to wait before the mutex becomes available. A timeout of 0 can
 *          be used to poll the mutex. TIMEOUT_MAX can be used to block indefinitely.
 * @return
 *          True if the mutex was successfully taken, false otherwise. If false
 *          is returned, then errno is set with a hint about why the the mutex
 *          couldn't be taken.
 */
bool_t mutex_take(mutex_t mutex, uint32_t timeout);

/**
 * Unlocks a mutex.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#mutexes for details.
 *
 * @param mutex
 *          Mutex to unlock.
 * @return
 *          True if the mutex was successfully returned, false otherwise. If false
 *          is returned, then errno is set with a hint about why the mutex couldn't
 *          be returned.
 */
bool_t mutex_give(mutex_t mutex);

/**
 * Creates a counting sempahore.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#semaphores for details.
 *
 * @param max_count
 *          The maximum count value that can be reached.
 * @param init_count
 *          The initial count value assigned to the new semaphore.
 * @return
 *          A newly created semaphore.
 */
sem_t sem_create(uint32_t max_count, uint32_t init_count);

/**
 * Waits for the semaphore's value to be greater than 0. If the value is already
 * greater than 0, this function immediately returns.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#semaphores for details.
 *
 * @param sem
 *          Semaphore to wait on
 * @param timeout
 *          Time to wait before the semaphore's becomes available. A timeout of 0
 *          can be used to poll the sempahore. TIMEOUT_MAX can be used to block
 *          indefinitely.
 * @return
 *          True if the semaphore was successfully take, false otherwise.
 *          If false is returned, then errno is set with a hint about why the
 *          sempahore couldn't be taken.
 */
bool_t sem_wait(sem_t sem, uint32_t timeout);

/**
 * Increments a semaphore's value.
 *
 * See https://pros.cs.purdue.edu/v5/tutorials/multitasking#semaphores for details.
 *
 * @param sem
 *          Semaphore to post
 * @return
 *          True if the value was incremented, false otherwise. If false is
 *          returned, then errno is set with a hint about why the semaphore
 *          couldn't be taken.
 */
bool_t sem_post(sem_t sem);

#ifdef __cplusplus
}
#endif

#endif /* _PROS_API_H_ */
