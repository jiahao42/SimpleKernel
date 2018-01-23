#include "inc/test.h"
#include "inc/tcb.h"
#include "stdio.h"

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
  TCB_node *node0 = create_tcb_node(&dummy0);
  TCB_node *node1 = create_tcb_node(&dummy1);
  TCB_node *node2 = create_tcb_node(&dummy2);
  TCB_node *node3 = create_tcb_node(&dummy3);
  TCB_node *node4 = create_tcb_node(&dummy4);
  TCB_node *node5 = create_tcb_node(&dummy5);
  TCB_list *tcb_list = create_tcb_list();

  /* Test begin */
  tcb_append(tcb_list, node0); // [node0]
  EXPECT_EQ_TCB_NODE(node0, tcb_get_node(tcb_list, 0));
  EXPECT_EQ_TCB_NODE(node0, tcb_list->head);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->tail);

  tcb_append(tcb_list, node1); // node0 <--> [node1]
  EXPECT_EQ_TCB_NODE(node1, tcb_get_node(tcb_list, 1));
  EXPECT_EQ_TCB_NODE(node0, node1->prev);
  EXPECT_EQ_TCB_NODE(node1, node0->next);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->head);
  EXPECT_EQ_TCB_NODE(node1, tcb_list->tail);

  tcb_append(tcb_list, node2); // node0 <--> node1 <--> [node2]
  EXPECT_EQ_TCB_NODE(node2, tcb_get_node(tcb_list, 2));
  EXPECT_EQ_TCB_NODE(node1, node2->prev);
  EXPECT_EQ_TCB_NODE(node2, node1->next);
  EXPECT_EQ_TCB_NODE(node0, tcb_list->head);
  EXPECT_EQ_TCB_NODE(node2, tcb_list->tail);

  tcb_insert_after(tcb_list, node1, node3); // node0 <--> node1 <--> [node3] <--> node2
  EXPECT_EQ_TCB_NODE(node3, tcb_get_node(tcb_list, 2));
  EXPECT_EQ_TCB_NODE(node1, node3->prev);
  EXPECT_EQ_TCB_NODE(node2, node3->next);
  EXPECT_EQ_TCB_NODE(node3, node1->next);
  EXPECT_EQ_TCB_NODE(node3, node2->prev);
  
  tcb_insert_before(tcb_list, node0, node4); // [node4] <--> node0 <--> node1 <--> node3 <--> node2
  EXPECT_EQ_TCB_NODE(node4, tcb_get_node(tcb_list, 0));
  EXPECT_EQ_TCB_NODE(node0, node4->next);
  EXPECT_EQ_TCB_NODE(node4, node0->prev);

  // node4 <--> node0 <--> [node5] <--> node1 <--> node3 <--> node2
  tcb_insert_before(tcb_list, node1, node5);
  EXPECT_EQ_TCB_NODE(node5, tcb_get_node(tcb_list, 2));
  EXPECT_EQ_TCB_NODE(node0, node5->prev);
  EXPECT_EQ_TCB_NODE(node1, node5->next);
  EXPECT_EQ_TCB_NODE(node5, node0->next);
  EXPECT_EQ_TCB_NODE(node5, node1->prev);
}