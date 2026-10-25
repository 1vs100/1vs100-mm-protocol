#include "check.h"

int g_Failures = 0;

void TestMmProtocol();

int main()
{
	TestMmProtocol();
	if (g_Failures)
		printf("%d check(s) failed\n", g_Failures);
	else
		printf("all checks passed\n");
	return g_Failures ? 1 : 0;
}
