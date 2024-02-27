#include "hw_0.h"
#include "hw_2_hw_3.h"
#include "hw_4.h"
#include "hw_4_hw_1.h"
#include "hw_5.h"

#define CLR_PREF(v) "\033[1;" #v "m"
#define CLR_SUFF() "\033[0m"

#define RUN_TEST(ns) \
{                                           \
    std::cout << CLR_PREF(33) "TEST `" #ns "` BEGIN\n" CLR_SUFF();  \
    ns::test();                             \
    std::cout << CLR_PREF(32) "TEST `" #ns "` PASSED\n" CLR_SUFF();  \
}

int main()
{
    RUN_TEST(hw0);
    RUN_TEST(hw2_hw3_doms_and_loops);
    RUN_TEST(hw4_linear_order);
    RUN_TEST(hw1_hw4_fact);
    RUN_TEST(hw5_linear_scan);
    return 0;
}
