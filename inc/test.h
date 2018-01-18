#ifndef LAMB_KERNEL_TEST_H_
#define LAMB_KERNEL_TEST_H_

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

void test_tcb();
void test_suite();


#endif