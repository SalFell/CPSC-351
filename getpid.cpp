#include <iostream>
#include <unistd.h>
using namespace std;

/* getpid() is a system call declared in unistd.h.  It returns */
/* a value of type pid_t.  This pid_t is a special type for process ids. */
/* It's equivalent to int. */

int main(void)
{

	for (int i = 0;  i < 5;  i++)
	{
		cout << "This is process " << getpid() << endl;
		/* sleep is a system call or library function that suspends
		   this process for the indicated number of seconds */
		sleep(10);
	}

	return 0;
}
