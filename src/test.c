#include "kernel.h"
#include "string.h"
#include "stdio.h"
#include "test.h"


int test_count = 0;
int test_pass = 0;
int main_ret = 0;

void internal_test_suite() {
  test_tcb();
  test_mailbox();
  printf("Kernel Test: %d/%d (%3.2f%%) passed\n", test_pass, test_count,
        test_pass * 100.0 / test_count);
}

#define EXPECT_EQ_ADDR(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), ((int)&(*(expect))), ((int)&(*(actual))), "%x")

void test_tcb() {
  /* Init */
  listobj *node0 = create_listobj(create_TCB());
  listobj *node1 = create_listobj(create_TCB());
  listobj *node2 = create_listobj(create_TCB());
  listobj *node3 = create_listobj(create_TCB());
  listobj *node4 = create_listobj(create_TCB());
  node4->pTask->DeadLine = 0x4444;
  listobj *node5 = create_listobj(create_TCB());
  node5->pTask->DeadLine = 0x5555;
  list *tcb_list = create_list();

  /* Test begin */
  list_append(tcb_list, node0); // [node0]
  EXPECT_EQ_ADDR(node0, tcb_list->pHead);
  EXPECT_EQ_ADDR(node0, tcb_list->pTail);

  list_append(tcb_list, node1); // node0 <--> [node1]
  EXPECT_EQ_ADDR(node0, list_get_head(tcb_list));
  EXPECT_EQ_ADDR(node0, node1->pPrevious);
  EXPECT_EQ_ADDR(node1, node0->pNext);
  EXPECT_EQ_ADDR(node0, tcb_list->pHead);
  EXPECT_EQ_ADDR(node1, tcb_list->pTail);

  list_append(tcb_list, node2); // node0 <--> node1 <--> [node2]
  EXPECT_EQ_ADDR(node0, list_get_head(tcb_list));
  EXPECT_EQ_ADDR(node1, node2->pPrevious);
  EXPECT_EQ_ADDR(node2, node1->pNext);
  EXPECT_EQ_ADDR(node0, tcb_list->pHead);
  EXPECT_EQ_ADDR(node2, tcb_list->pTail);

  list_prepend(tcb_list, node3); // [node3] <--> node0 <--> node1 <--> node2
  EXPECT_EQ_ADDR(node3, list_get_head(tcb_list));
  EXPECT_EQ_ADDR(node3, node0->pPrevious);
  EXPECT_EQ_ADDR(node0, node3->pNext);
  EXPECT_EQ_ADDR(node3, tcb_list->pHead);

  list_insert_by_ddl(tcb_list, node4); // [node4] <--> node3 <--> node0 <--> node1 <--> node2
  EXPECT_EQ_ADDR(node4, list_get_head(tcb_list));
  EXPECT_EQ_ADDR(node3, node4->pNext);
  EXPECT_EQ_ADDR(node4, node3->pPrevious);
  EXPECT_EQ_ADDR(node4, tcb_list->pHead);

  list_insert_by_ddl(tcb_list, node5); // node4 <--> [node5] <--> node3 <--> node0 <--> node1 <--> node2
  EXPECT_EQ_ADDR(node4, list_get_head(tcb_list));
  EXPECT_EQ_ADDR(node4, node5->pPrevious);
  EXPECT_EQ_ADDR(node5, node4->pNext);
  EXPECT_EQ_ADDR(node4, tcb_list->pHead);

  destroy_list(tcb_list);
}

void test_mailbox() {
  mailbox *mb = create_mailbox(2, sizeof(int)); // for wait msg
  msg m0;
  m0.Status = SENDER;
  msg m1;
  m1.Status = RECEIVER;
  mailbox_push_wait_msg(mb, &m0); // [m0]
  EXPECT_EQ_ADDR(&m0, mb->pHead);
  EXPECT_EQ_ADDR(&m0, mb->pTail);
  EXPECT_EQ_INT(1, mb->nBlockedMsg);
  mailbox_push_wait_msg(mb, &m1); // [m0] <--> [m1]
  EXPECT_EQ_ADDR(&m0, mb->pHead);
  EXPECT_EQ_ADDR(&m1, mb->pTail);
  EXPECT_EQ_ADDR(&m1, m0.pNext);
  EXPECT_EQ_ADDR(&m0, m1.pPrevious);
  EXPECT_EQ_INT(2, mb->nBlockedMsg);
  msg* pm0 = mailbox_pop_wait_msg(mb); // [m1]
  EXPECT_EQ_ADDR(&m0, pm0);
  EXPECT_EQ_ADDR(&m1, mb->pHead);
  EXPECT_EQ_ADDR(&m1, mb->pTail);
  EXPECT_EQ_INT(1, mb->nBlockedMsg);
  msg* pm1 = mailbox_pop_wait_msg(mb); //
  EXPECT_EQ_ADDR(&m1, pm1);
  EXPECT_EQ_INT(0, mb->nBlockedMsg);

  mailbox_push_no_wait_msg(mb, &m0); // [m0]
  EXPECT_EQ_INT(1, mb->nMessages);
  mailbox_push_no_wait_msg(mb, &m1); // [m0] <--> [m1]
  EXPECT_EQ_INT(2, mb->nMessages);
  msg m2;
  // mailbox will be FULL! erase oldest msg
  mailbox_push_no_wait_msg(mb, &m2); // m[1] <--> [m2]
  EXPECT_EQ_INT(2, mb->nMessages);
  pm1 = mailbox_pop_no_wait_msg(mb); // [m2]
  EXPECT_EQ_ADDR(&m1, pm1);
  EXPECT_EQ_INT(1, mb->nMessages);
  msg* pm2 = mailbox_pop_no_wait_msg(mb); // 
  EXPECT_EQ_ADDR(&m2, pm2);
  EXPECT_EQ_INT(0, mb->nMessages);
}

