#include <iostream>

using namespace std;

struct ComplexNum{
    float real;
    float imag;
};

int main()
{
    ComplexNum x;
    ComplexNum *ptr;

    ptr = &x; 	// ptr is now a pointer to x

    x.real = 100.3;
    x.imag = 8.2223;

    cout << "real: " << x.real << "\n";
    cout << "imag: " << x.imag << "\n";

    ptr->real = 3;     // illustrates that we are changing the value in "x" through ptr.
    
    cout << "real (again): " << x.real << "\n";

    return 0;
}
