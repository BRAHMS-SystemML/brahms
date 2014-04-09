
#include <iostream>
using namespace std;
#include "embed.h"

#include <sys/time.h>

#include "stdint.h"

#define INT64 int64_t

inline INT64 count()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	INT64 ret = tv.tv_sec;
	ret *= 1000000;
	ret += tv.tv_usec;
	return ret;
}

void myeval(const char *string)
{
  std::cout << "Eval <" << string << ">\n";
  octave_call(string);
}

int main(int argc, char *argv[])
{
  // Puts error output into file
  //std::ofstream err("cerr.txt");
  //std::cerr.rdbuf(err.rdbuf());

  // Puts non-error output into file
  //std::ofstream err("cout.txt");

  //octave_stdout.rdbuf(err.rdbuf());

	INT64 t0 = count();

  octave_init(argc, argv);
  
  	cout << "init: " << (count() - t0) << endl;
  
//  myeval("a = [1 2 3 4 5]");
//  myeval("b = [1 2 3 4] ./ [4 4 0]");

  myeval("b = 1;");
	t0 = count();
	int N = 10000;
	for (int i=0; i<10000; i++)
		octave_call("b = b + 1;");
	cout << "T: " << ((count() - t0) / N) << "us" << endl;

  myeval("b = tf;");
  
//  myeval("a = [1 2 3 4 5]");
//  myeval("b = [1 2 3 4] ./ [4 4 0]");

	t0 = count();
  octave_exit();
  	cout << "term: " << (count() - t0) << endl;

  return 0;
}

