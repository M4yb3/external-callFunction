#include <iostream>
#include <Windows.h>

static int value = 1;

static void increase(int* variable, int incr) {
	*variable += incr;
}

int main() {
	while (true) {
		std::cout << "Func addr:\t" << &increase << std::endl;

		std::cout << "Value addr:\t" << &value << std::endl;
		std::cout << "Value:\t\t" << value << std::endl;

		Sleep(1000);

		increase(&value, 1);
		system("cls");
	}

	return 0;
}
