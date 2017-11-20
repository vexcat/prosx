#include "api.h"
#include "ifi/v5_api.h"
#include <errno.h>

void task_a_fn(void* ign) {
	vexDisplayString(2, "Errno from A is: %d\n", errno);
	task_delay(1000);
	errno = ENOEXEC;
	while (1) {
		vexDisplayString(2, "Errno from A is: %d\n", errno);
		task_delay(10);
	}
}

void task_b_fn(void* ign) {
	while (1) {
		vexDisplayString(3, "Errno from B is: %d\n", errno);
		task_delay(10);
	}
}

void test_errno_reentrancy() {
	task_create(task_a_fn, NULL, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Task A");
	task_create(task_b_fn, NULL, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Task B");
}
