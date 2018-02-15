#ifndef LAMB_KERNEL_TEST_H_
#define LAMB_KERNEL_TEST_H_

#include "kernel.h"

void test_tcb();
void internal_test_suite();
extern TCB* create_TCB();
extern void init_listobj(listobj *node, TCB *data);
extern listobj *create_listobj(TCB *data);
extern void destroy_listobj(listobj *node);
extern list *create_list();
extern void list_append(list *t_list, listobj *node);
extern void list_prepend(list *t_list, listobj *node);
extern void list_insert_after(list *t_list, listobj *pos, listobj *n_node);
extern void list_insert_before(list *t_list, listobj *pos, listobj *n_node);
extern void list_insert_by_ddl(list *t_list, listobj *n_node);
extern void list_remove_head(list *t_list);
extern void list_remove_tail(list *t_list);
extern TCB* list_get_head_task(list *t_list);
extern listobj *list_get_head(list *t_list);
extern uint if_node_in_list(list *l, listobj *node);
extern void node_remove(list *t_list, listobj *node);
extern void node_destroy_by_task(list *t_list, TCB* tcb);
extern listobj* node_fetch_by_task(list *t_list, TCB *tcb);
extern void node_transfer_list(list *src, list *dest, listobj* node);
extern void destroy_list(list *t_list);

extern void mailbox_push_msg(mailbox* mBox, msg* m);
extern msg* mailbox_pop_msg(mailbox *mBox);
extern mailbox* create_mailbox(uint nMessages, uint nDataSize);
extern int no_messages(mailbox* mBox);
extern exception remove_mailbox(mailbox* mBox);

extern int test_count;
extern int test_pass;
extern int main_ret;

/*
 * Simple Test Framework
 */
extern int test_count;
extern int test_pass;
extern int main_ret;
#define EXPECT_EQ_BASE(equality, expect, actual, format)                       \
    do {                                                                       \
        test_count++;                                                          \
        if (equality) {                                                        \
            test_pass++;                                                       \
        } else {                                                               \
            main_ret = 1;                                                      \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n",  \
                    __FILE__, __LINE__, expect, actual);                       \
        }                                                                      \
    } while (0)

#define EXPECT_EQ_INT(expect, actual)                                          \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_STRING(expect, actual)                                       \
    EXPECT_EQ_BASE(actual.compare(expect) == 0, expect, actual.c_str(), "%s")
#define EXPECT_EQ_SIZE(expect, actual)                                         \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%zu")

#define EXPECT_EQ_DOUBLE(expect, actual)                                       \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual)                                                   \
    EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#endif