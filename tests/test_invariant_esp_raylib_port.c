#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Include the actual production header
#include "esp_raylib_port.h"

START_TEST(test_allocation_size_overflow_protection)
{
    // Invariant: Multiplication for allocation size must not overflow or must be validated
    // before allocation to prevent heap overflow
    
    // Test cases: boundary values that could cause overflow
    size_t test_cases[][2] = {
        {SIZE_MAX, 2},           // Overflow case
        {SIZE_MAX / 2 + 1, 2},   // Boundary overflow case
        {100, 200},              // Valid normal case
        {0, SIZE_MAX},           // Zero with large multiplier
        {SIZE_MAX, 1}            // Max size with multiplier 1
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_cases; i++) {
        size_t a = test_cases[i][0];
        size_t b = test_cases[i][1];
        
        // The security property: if multiplication would overflow,
        // the function must either detect it or use safe allocation
        // We're testing that the actual function doesn't crash or overflow
        void *result = allocate_buffer_safely(a, b);
        
        // If allocation succeeded, we should be able to use it safely
        if (result != NULL) {
            // Verify the allocated memory can be accessed without overflow
            // by checking that the size calculation didn't wrap
            size_t calculated_size = a * b;
            if (calculated_size / a != b) { // Overflow occurred
                // This should have been caught before allocation
                ck_assert_msg(0, "Multiplication overflow not caught before allocation");
            }
            free(result);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_allocation_size_overflow_protection);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}