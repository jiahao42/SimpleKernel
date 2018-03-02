#include "kernel.h"
#include "test.h"

TCB *Running;
mailbox *mb;
void task1();
void task2();
#define TEST_PATTERN_1 0xAA
#define TEST_PATTERN_2 0x55

#define EXPECT_EQ_OR_STUCK(expect, actual)                                     \
  do {                                                                         \
    if ((expect) != (actual))                                                  \
      while (1)                                                                \
        ;                                                                      \
  } while (0)

#define EXPECT_NOT_EQ_OR_STUCK(expect, actual)                                 \
  do {                                                                         \
    if ((expect) == (actual))                                                  \
      while (1)                                                                \
        ;                                                                      \
  } while (0)

#define EXPECT_EQ_OR_TERMINATE(expect, actual)                                 \
  do {                                                                         \
    if ((expect) != (actual))                                                  \
      terminate();                                                             \
  } while (0)

#define EXPECT_NOT_EQ_OR_TERMINATE(expect, actual)                             \
  do {                                                                         \
    if ((expect) == (actual))                                                  \
      terminate();                                                             \
  } while (0)

#define EXAMPLE 3

#if EXAMPLE == 1

int nTest1 = 0, nTest2 = 0, nTest3 = 0;
int main(void) {
  internal_test_suite();
  EXPECT_EQ_OR_STUCK(OK, init_kernel()); /* Memory allocation problems */
  EXPECT_EQ_OR_STUCK(OK,
                     create_task(task1, 2000)); /* Memory allocation problems */
  EXPECT_EQ_OR_STUCK(OK,
                     create_task(task2, 4000)); /* Memory allocation problems */
  EXPECT_NOT_EQ_OR_STUCK(
      NULL,
      (mb = create_mailbox(1, sizeof(int)))); /* Memory allocation problems */

  run(); /* First in readylist is task1 */
}

void task1(void) {
  volatile int nData_t1 = TEST_PATTERN_1;
  wait(10); /* task2 börjar köra */
  EXPECT_EQ_OR_TERMINATE(1, no_messages(mb));
  EXPECT_NOT_EQ_OR_TERMINATE(DEADLINE_REACHED, send_wait(mb, &nData_t1));
  wait(10); /* task2 börjar köra */
  /* start test 2 */
  nData_t1 = TEST_PATTERN_2;
  EXPECT_NOT_EQ_OR_TERMINATE(DEADLINE_REACHED, send_wait(mb, &nData_t1));
  wait(10); /* task2 börjar köra */
  /* start test 3 */
  if (send_wait(mb, &nData_t1) == DEADLINE_REACHED) {
    EXPECT_EQ_OR_TERMINATE(0, no_messages(mb));
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
  EXPECT_NOT_EQ_OR_TERMINATE(DEADLINE_REACHED, receive_wait(mb, &nData_t2));
  EXPECT_EQ_OR_TERMINATE(0, no_messages(mb));
  if (nData_t2 == TEST_PATTERN_1)
    nTest1 = 1;
  wait(20); /* t1 kör nu */
  /* start test 2 */
  EXPECT_EQ_OR_TERMINATE(1, no_messages(mb));
  EXPECT_NOT_EQ_OR_TERMINATE(DEADLINE_REACHED, receive_wait(mb, &nData_t2));
  EXPECT_EQ_OR_TERMINATE(0, no_messages(mb));
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
  EXPECT_EQ_OR_STUCK(OK, init_kernel());                      /* 1 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, (mb = create_mailbox(1, 15))); /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task1, 100));            /* 3 */
  run();                                                      /* 4 */

}

void task1(void) {
  int msg = TEST_PATTERN_1;
  /* start test 1 */
  EXPECT_EQ_OR_TERMINATE(OK, create_task(task2, 200)); /* 5 */
  if (send_wait(mb, &msg) != OK) {                     /* 6 */
    while (1)
      ;
  }
  /* start test 2 */
  if (send_wait(mb, &msg) == DEADLINE_REACHED) { /* 7 */
    nTest2 = 1;
  }
  terminate();
}

void task2(void) {
  int msg = 0;
  EXPECT_EQ_OR_TERMINATE(SENDER, receive_no_wait(mb, &msg)); /* 8 */
  if (msg == TEST_PATTERN_1) {
    nTest1 = 1;
  }
  wait(99); /* 9 */
  terminate();
}

#endif

#if EXAMPLE == 3

int nTest1 = 0, nTest2 = 0;
int main() {
  EXPECT_EQ_OR_STUCK(OK, init_kernel());                             /* 1 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task1, 200));                    /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task2, 150));                   /* 3 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, mb = create_mailbox(1, sizeof(int))); /* 4 */
  run();                                                             /* 5 */
}

void task1(void) {
  int data2; /* 7 */
  EXPECT_NOT_EQ_OR_STUCK(DEADLINE_REACHED,
                             receive_wait(mb, &data2));    /* 8/9 */
  if (data2 == TEST_PATTERN_1) {
    nTest1 = 1;
  }                          
  /* start test 2 */
  wait(20);                                                /* 10 */
  EXPECT_NOT_EQ_OR_STUCK(OK, receive_no_wait(mb, &data2)); /* 11 */
  EXPECT_EQ_OR_STUCK(TEST_PATTERN_1, data2);
  if (data2 == TEST_PATTERN_2) {
    nTest2 = 1;
  }
  EXPECT_NOT_EQ_OR_STUCK(FAIL, receive_no_wait(mb, &data2)); /* 12 */
  terminate();                                               /* 13 */
}

void task2() {               /* execute first */
  int data = TEST_PATTERN_1; /* 14 */
  /* start test 1 */
  EXPECT_NOT_EQ_OR_STUCK(DEADLINE_REACHED,
                             send_wait(mb, &data)); /* 15/16 */
  data = TEST_PATTERN_2;
  int *a = malloc(sizeof(int) * 4);
  a = malloc(sizeof(int) * 4);
  a = malloc(sizeof(int) * 4);
  a = malloc(sizeof(int) * 4);
  *a++;
  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb, &data));
  /* 17 */                    // TODO: wrap with macro
  wait(100);                  /* 18 */
  /* start test 3 */
  set_deadline(ticks() + 10); /* 19 */
  EXPECT_EQ_OR_STUCK(deadline(), 110);
  create_task(task1, ticks() + 30); /* 20 */
  terminate();                      /* 21 */
}

#endif