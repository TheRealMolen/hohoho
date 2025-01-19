#include <iostream>

#include "../colour.h"


inline int do_expect(const char* msg, const auto& result, const auto& expected)
{
    if (result == expected)
        return 1;

    std::cout << "FAIL:  " << msg << "\n";
    std::cout << "    expected: " << expected << "\n";
    std::cout << "    actual:   " << result << "\n\n";
    return 0;
}
#define EXPECT_EQ(_expr, _expected)    succeeded += do_expect(#_expr, _expr, _expected)


int test_lerp()
{
    int succeeded = 0;
    
    const FHsv a { 0.125, 0.0, 0.0f };
    const FHsv b { 0.875, 1.f, 0.f };

    EXPECT_EQ(lerp(a, b, 0.5f).h, 0.0f);

    return succeeded;
}


int main()
{
    printf("hello bob\n\n");

    int succeeded = 0;

    succeeded += test_lerp();

    printf("\n=======================================\n");
    printf("%d succeeded\n", succeeded);

    return 0;
}