#include "kernel.h"
#include "string.h"
#include "stdio.h"
#include "test.h"


int test_count = 0;
int test_pass = 0;
int main_ret = 0;

void test_suite() {
  test_tcb();
  printf("Kernel Test: %d/%d (%3.2f%%) passed\n", test_pass, test_count,
        test_pass * 100.0 / test_count);
}

#define NODE_ADDR(NODE) (int)&(*(NODE)) // node
#define EXPECT_EQ_TCB_NODE(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), NODE_ADDR(expect), NODE_ADDR(actual), "%x")

void test_tcb() {
  /* Init */
  TCB dummy0, dummy1, dummy2, dummy3, dummy4, dummy5;
  listobj *node0 = create_listobj(&dummy0);
  listobj *node1 = create_listobj(&dummy1);
  listobj *node2 = create_listobj(&dummy2);
  listobj *node3 = create_listobj(&dummy3);
  listobj *node4 = create_listobj(&dummy4);
  listobj *node5 = create_listobj(&dummy5);
  list *tcb_list = create_list();

  /* Test begin */
  list_append(tcb_list, node0); // [node0]
  // EXPECT_EQ_TCB_NODE(node0, tcb_get_node(tcb_list, 0));
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pTail);

  list_append(tcb_list, node1); // node0 <--> [node1]
  // EXPECT_EQ_TCB_NODE(node1, tcb_get_node(tcb_list, 1));
  EXPECT_EQ_TCB_NODE(node0, node1->pPrevious);
  EXPECT_EQ_TCB_NODE(node1, node0->pNext);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node1, tcb_list->pTail);

  list_append(tcb_list, node2); // node0 <--> node1 <--> [node2]
  // EXPECT_EQ_TCB_NODE(node2, tcb_get_node(tcb_list, 2));
  EXPECT_EQ_TCB_NODE(node1, node2->pPrevious);
  EXPECT_EQ_TCB_NODE(node2, node1->pNext);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->pHead);
  EXPECT_EQ_TCB_NODE(node2, tcb_list->pTail);
}