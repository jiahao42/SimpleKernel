#include "kernel.h"
#include "test.h"

/*
There are 4 tests to check if the kernel works as predicted,
choose the test by define the macro
*/
#define TEST_INDEX 5
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

#if TEST_INDEX == 1
mailbox *mb;
void task1();
void task2();
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

#if TEST_INDEX == 2
mailbox *mb;
void task1();
void task2();
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

#if TEST_INDEX == 3
mailbox *mb;
void task1();
void task2();
int nTest1 = 0, nTest2 = 0;
int main() {
  EXPECT_EQ_OR_STUCK(OK, init_kernel());                             /* 1 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task1, 2000));                   /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task2, 1500));                   /* 3 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, mb = create_mailbox(1, sizeof(int))); /* 4 */
  run();                                                             /* 5 */
}

void task1(void) {
  int data2; /* 8/23 */
  /* Finally, it will stop here */
  EXPECT_NOT_EQ_OR_STUCK(DEADLINE_REACHED, receive_wait(mb, &data2)); /* 9/24 */
  if (data2 == TEST_PATTERN_1) { /* 13/25 */
    nTest1 = 1;
  }
  /* start test 2 */
  wait(20);                                            /* 14/26 */
  EXPECT_EQ_OR_STUCK(OK, receive_no_wait(mb, &data2)); /* 15/27 */
  if (data2 == TEST_PATTERN_2) {                       /* 16 */
    nTest2 = 1;
  }
  EXPECT_EQ_OR_STUCK(FAIL, receive_no_wait(mb, &data2)); /* 17 */
  terminate();                                           /* 18 */
}

void task2() {               /* execute first */
  int data = TEST_PATTERN_1; /* 6 */
  /* start test 1 */
  EXPECT_NOT_EQ_OR_STUCK(DEADLINE_REACHED, send_wait(mb, &data)); /* 7 */
  data = TEST_PATTERN_2;                                          /* 10 */

  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb, &data)); /* 11 */
  wait(100);                                       /* 12 */
  /* start test 3 */
  set_deadline(ticks() + 10);          /* 19 */
  EXPECT_EQ_OR_STUCK(deadline(), 110); /* 20 */
  create_task(task1, ticks() + 30);    /* 21 */
  terminate();                         /* 22 */
}

#endif

#if TEST_INDEX == 4
#define NUM_OF_MSG(mb) (mb->nMessages + mb->nBlockedMsg)
void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();
mailbox *mb1;
mailbox *mb2;
int nTest = 0, nTest1 = 0, nTest2 = 0, nTest3 = 0, nTest4 = 0, nTest5 = 0,
    nTest6 = 0, nTest7 = 0, nTest8 = 0;
int main() {
  EXPECT_EQ_OR_STUCK(OK, init_kernel());                              /* 1 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task1, ticks() + 10000));        /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task2, ticks() + 20000));        /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task3, ticks() + 30000));        /* 2 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task4, ticks() + 40000));        /* 2 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, mb1 = create_mailbox(2, sizeof(int))); /* 3 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, mb2 = create_mailbox(2, sizeof(int))); /* 4 */
  run();                                                              /* 5 */
}

void task1() {
  int SIG1 = 1;                                      /* 6 */
  EXPECT_EQ_OR_STUCK(OK, send_wait(mb1, &SIG1));     /* 7 */
  EXPECT_EQ_OR_STUCK(0, NUM_OF_MSG(mb1));            /* 11 */
  int SIG2 = 2;                                      /* 12 */
  EXPECT_EQ_OR_STUCK(OK, send_wait(mb1, &SIG2));     /* 13 */
  int recv3 = 0;                                     /* 23 */
  EXPECT_EQ_OR_STUCK(OK, receive_wait(mb1, &recv3)); /* 24 */
  EXPECT_EQ_OR_STUCK(0, NUM_OF_MSG(mb1));            /* 25 */
  if (recv3 == 3) {                                  /* 26 */
    nTest3 = 1;
  }
  wait(10); /* 27 */

  /* idle() for 10 ticks */ /* 38 */
  terminate();              /* 39 */
}

void task2() {
  int recv1 = 0;                                        /* 8 */
  EXPECT_EQ_OR_STUCK(1, NUM_OF_MSG(mb1));               /* 9 */
  EXPECT_EQ_OR_STUCK(OK, receive_no_wait(mb1, &recv1)); /* 10 */
  if (recv1 == 1) {                                     /* 14 */
    nTest1 = 1;
  }
  int SIG3 = 3;                                  /* 15 */
  EXPECT_EQ_OR_STUCK(OK, send_wait(mb1, &SIG3)); /* 16 */
  wait(20);                                      /* 28 */

  /* idle() for 10 ticks */ /* 40 */
  terminate();              /* 41 */
}

void task3() {
  int SIG4 = 4;                                      /* 17 */
  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb2, &SIG4));  /* 18 */
  EXPECT_EQ_OR_STUCK(1, NUM_OF_MSG(mb2));            /* 19 */
  EXPECT_EQ_OR_STUCK(2, NUM_OF_MSG(mb1));            /* 20 */
  int recv2 = 0;                                     /* 21 */
  EXPECT_EQ_OR_STUCK(OK, receive_wait(mb1, &recv2)); /* 22 */
  if (recv2 == 2) {                                  /* 29 */
    nTest2 = 1;
  }
  wait(30);                 /* 30 */
  /* idle() for 10 ticks */ /* 42 */
  terminate();              /* 43 */
}

void task4() {
  int recv_dummy = 0;                                          /* 31 */
  EXPECT_EQ_OR_STUCK(FAIL, receive_no_wait(mb1, &recv_dummy)); /* 32 */
  int recv4 = 0;                                               /* 33 */
  EXPECT_EQ_OR_STUCK(OK, receive_wait(mb2, &recv4));           /* 34 */
  if (recv4 == 4) {                                            /* 35 */
    nTest4 = 1;
  }
  recv_dummy = 0;                                                       /* 36 */
  EXPECT_EQ_OR_STUCK(DEADLINE_REACHED, receive_wait(mb1, &recv_dummy)); /* 37 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task5, ticks() + 50000));          /* 44 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task6, ticks() + 60000));          /* 45 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task7, ticks() + 70000));          /* 46 */
  int SIG_dummy = -1;                                                   /* 47 */
  /* The following msg will be overwritten */
  EXPECT_EQ_OR_STUCK(DEADLINE_REACHED, send_wait(mb1, &SIG_dummy)); /* 48 */
  /* nobody receive msg, idle until deadline reached */             /* 73 */
  nTest = nTest1 * nTest2 * nTest3 * nTest4 * nTest5 * nTest6 * nTest7 * nTest8;
  terminate(); /* 74 */
}

void task5() {
  int SIG5 = 5;                                  /* 49 */
  EXPECT_EQ_OR_STUCK(OK, send_wait(mb1, &SIG5)); /* 50 */
  terminate();                                   /* 55 */
}

void task6() {
  int SIG6 = 6;                                  /* 51 */
  EXPECT_EQ_OR_STUCK(OK, send_wait(mb1, &SIG6)); /* 52 */
  terminate();                                   /* 59 */
}

void task7() {
  int recv5 = 0;                                        /* 53 */
  EXPECT_EQ_OR_STUCK(OK, receive_no_wait(mb1, &recv5)); /* 54 */
  if (recv5 == 5) {                                     /* 56 */
    nTest5 = 1;
  }
  int recv6 = 0;                                     /* 57 */
  EXPECT_EQ_OR_STUCK(OK, receive_wait(mb1, &recv6)); /* 58 */
  if (recv6 == 6) {                                  /* 60 */
    nTest6 = 1;
  }
  /* Test msg overwritten in send_no_wait */
  EXPECT_EQ_OR_STUCK(0, NUM_OF_MSG(mb1)); /* 61 */
  /* This following msg will be overwritten */
  int SIG_dummy = 0;
  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb1, &SIG_dummy)); /* 62 */
  EXPECT_EQ_OR_STUCK(1, NUM_OF_MSG(mb1));                /* 63 */
  int SIG_7 = 7;
  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb1, &SIG_7)); /* 64 */
  EXPECT_EQ_OR_STUCK(2, NUM_OF_MSG(mb1));            /* 65 */
  int SIG_8 = 8;
  EXPECT_EQ_OR_STUCK(OK, send_no_wait(mb1, &SIG_8)); /* 66 */
  EXPECT_EQ_OR_STUCK(2, NUM_OF_MSG(mb1));            /* 67 */
  int recv7 = 0;
  EXPECT_EQ_OR_STUCK(OK, receive_no_wait(mb1, &recv7)); /* 68 */
  if (recv7 == 7) {                                     /* 69 */
    nTest7 = 1;
  }
  int recv8 = 0;
  EXPECT_EQ_OR_STUCK(OK, receive_wait(mb1, &recv8)); /* 70 */
  if (recv8 == 8) {                                  /* 71 */
    nTest8 = 1;
  }

  terminate(); /* 72 */
}

#endif

#if TEST_INDEX == 5
mailbox *mb1;
void task1();
int main() {
  EXPECT_EQ_OR_STUCK(OK, init_kernel());                              /* 1 */
  EXPECT_EQ_OR_STUCK(OK, create_task(task1, ticks() + 10000));        /* 2 */
  EXPECT_NOT_EQ_OR_STUCK(NULL, mb1 = create_mailbox(2, sizeof(int))); /* 3 */
  run();  
}

void task1() {
  int dummy = 0;
  send_no_wait(mb1, &dummy);
  send_no_wait(mb1, &dummy);
  send_no_wait(mb1, &dummy);
  terminate();
}

#endif