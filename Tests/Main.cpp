#include <Testing/Testing.h>

extern void MemoryTests();
extern void UTFTests();

int main()
{
	Testing::Begin();
	MemoryTests();
	UTFTests();
	Testing::End();
}