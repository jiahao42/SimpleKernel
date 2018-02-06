#include "kernel.h"
#include "string.h"
#include "stdio.h"
#include "test.h"


int test_count = 0;
int test_pass = 0;
int main_ret = 0;

void internal_test_suite() {
  test_tcb();
  printf("Kernel Test: %d/%d (%3.2f%%) passed\n", test_pass, test_count,
        test_pass * 100.0 / test_count);
}

#define NODE_ADDR(NODE) (int)&(*(NODE)) // node
#define EXPECT_EQ_TCB_NODE(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), NODE_ADDR(expect), NODE_ADDR(actual), "%x")

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
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pTail);

  list_append(tcb_list, node1); // node0 <--> [node1]
  EXPECT_EQ_TCB_NODE(node0, node1->pPrevious);
  EXPECT_EQ_TCB_NODE(node1, node0->pNext);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node1, tcb_list->pTail);

  list_append(tcb_list, node2); // node0 <--> node1 <--> [node2]
  EXPECT_EQ_TCB_NODE(node1, node2->pPrevious);
  EXPECT_EQ_TCB_NODE(node2, node1->pNext);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node2, tcb_list->pTail);

  list_prepend(tcb_list, node3); // [node3] <--> node0 <--> node1 <--> node2
  EXPECT_EQ_TCB_NODE(node3, node0->pPrevious);
  EXPECT_EQ_TCB_NODE(node0, node3->pNext);
  EXPECT_EQ_TCB_NODE(node3, tcb_list->pHead);

  list_insert_by_ddl(tcb_list, node4); // [node4] <--> node3 <--> node0 <--> node1 <--> node2
  EXPECT_EQ_TCB_NODE(node3, node4->pNext);
  EXPECT_EQ_TCB_NODE(node4, node3->pPrevious);
  EXPECT_EQ_TCB_NODE(node4, tcb_list->pHead);


  list_insert_by_ddl(tcb_list, node5); // node4 <--> [node5] <--> node3 <--> node0 <--> node1 <--> node2
  EXPECT_EQ_TCB_NODE(node4, node5->pPrevious);
  EXPECT_EQ_TCB_NODE(node5, node4->pNext);
  EXPECT_EQ_TCB_NODE(node4, tcb_list->pHead);
}