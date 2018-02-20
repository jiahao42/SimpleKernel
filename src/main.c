#include "kernel.h"
#include "test.h"

TCB *Running;
mailbox *mb;
void task1();
void task2();
#define TEST_PATTERN_1 0xAA
#define TEST_PATTERN_2 0x55

#define EXAMPLE 2

#if EXAMPLE == 1

int nTest1 = 0, nTest2 = 0, nTest3 = 0;
int main(void) {
  internal_test_suite();
  if (init_kernel() != OK) {
    /* Memory allocation problems */
    while (1)
      ;
  }
  if (create_task(task1, 2000) != OK) {
    /* Memory allocation problems */
    while (1)
      ;
  }

  if (create_task(task2, 4000) != OK) {
    /* Memory allocation problems */
    while (1)
      ;
  }

  if ((mb = create_mailbox(1, sizeof(int))) == NULL) {
    /* Memory allocation problems */
    while (1)
      ;
  }

  run(); /* First in readylist is task1 */
}

void task1(void) {
  volatile int nData_t1 = TEST_PATTERN_1;
  wait(10); /* task2 börjar köra */
  if (no_messages(mb) != 1)
    terminate(); /* ERROR */
  if (send_wait(mb, &nData_t1) == DEADLINE_REACHED)
    terminate(); /* ERROR */
  wait(10);      /* task2 börjar köra */
  /* start test 2 */
  nData_t1 = TEST_PATTERN_2;
  if (send_wait(mb, &nData_t1) == DEADLINE_REACHED)
    terminate(); /* ERROR */
  wait(10);      /* task2 börjar köra */
  /* start test 3 */
  if (send_wait(mb, &nData_t1) == DEADLINE_REACHED) {
    if (no_messages(mb) != 0)
      terminate(); /* ERROR */
    nTest3 = 1;
    if (nTest1 * nTest2 * nTest3) {
      /* Blinka lilla lysdiod */
      /* Test ok! */
    }
    terminate(); /* PASS, no receiver */
  } else {
    terminate(); /* ERROR */
  }
}

void task2(void) {
  volatile int nData_t2 = 0;
  if (receive_wait(mb, &nData_t2) == DEADLINE_REACHED) /* t1 kör nu */
    terminate();                                       /* ERROR */
  if (no_messages(mb) != 0)
    terminate(); /* ERROR */
  if (nData_t2 == TEST_PATTERN_1)
    nTest1 = 1;
  wait(20); /* t1 kör nu */
  /* start test 2 */
  if (no_messages(mb) != 1)
    terminate();                                       /* ERROR */
  if (receive_wait(mb, &nData_t2) == DEADLINE_REACHED) /* t1 kör nu */
    terminate();                                       /* ERROR */
  if (no_messages(mb) != 0)
    terminate(); /* ERROR */
  if (nData_t2 == TEST_PATTERN_2)
    nTest2 = 1;
  /* Start test 3 */
  terminate();
}

#endif

#if EXAMPLE == 2

int nTest1 = 0, nTest2 = 0;
int main() {
  internal_test_suite();
  if (init_kernel() != OK) { /* 1 */
    while (1)
      ;
  }
  if ((mb = create_mailbox(1, 15)) == NULL) { /* 2 */
    while (1)
      ;
  }
  if (create_task(task1, 100) != OK) { /* 3 */
    while (1)
      ;
  }
  run(); /* 4 */
}

void task1(void) {
  int msg = TEST_PATTERN_1;
  if (create_task(task2, 200) != OK) { /* 5 */
    while (1)
      ;
  }
  if (send_wait(mb, &msg) != OK) { /* 6 */
    while (1)
      ;
  }
  if (send_wait(mb, &msg) == DEADLINE_REACHED) { /* 7 */
    nTest2 = 1;
  }
  terminate();
}

void task2(void) {
  int msg = 0;
  if (receive_no_wait(mb, &msg) != SENDER) { /* 8 */
    while (1)
      ;
  }
  if (msg == TEST_PATTERN_1) {
    nTest1 = 1;
  }
  wait(99); /* 9 */
  terminate();
}

#endif