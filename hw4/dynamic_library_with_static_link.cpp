#include <iostream>

void hello();

double myDiv(double, double);

int main() {
	
	hello();

	double ans = myDiv(7, 2);

	std::cout << ans << std::endl;
	
	return 0;
}