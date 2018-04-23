#include "staticlib/MyLib.h"
#include <stdio.h>

int main() {
	MyLib my;

	printf("%c\n", my.returnChar());

	int x = 99;
	int y = 1;
	
	printf("%d\n", my.sum(x, y));

	return 0;
}