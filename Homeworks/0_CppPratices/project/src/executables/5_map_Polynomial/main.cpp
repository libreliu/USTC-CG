#include "PolynomialMap.h"
#include "PolynomialList.h"

#include <map>
#include <iostream>
#include <time.h>

using namespace std;

// T - the implementation
template<typename T>
void do_poly_test(void) {
	std::vector<int> deg_vec;
	std::vector<double> cof_vec;

	clock_t t0 = clock();
	for (int i = 0; i < 500; i++) {
		deg_vec.push_back(rand());
		cof_vec.push_back((double)rand() / (rand() + 1) * rand());
	}
	clock_t vcg = clock();
	std::cout << "Vec Generation Time: " << vcg - t0 << std::endl;
	t0 = clock();

	T p_test(deg_vec, cof_vec);
	clock_t constr = clock();
	std::cout << "Poly Construction Time: " << constr - t0 << std::endl;

	t0 = clock();
	T p_mult(p_test * p_test);
	clock_t selfmul = clock();
	std::cout << "Poly Self-Multiplication and Assignment Time: " << selfmul - t0 << std::endl;

	t0 = clock();
	T p_add(p_test + p_test);
	clock_t selfadd = clock();
	std::cout << "Poly Self-Add and Assignment Time: " << selfadd - t0 << std::endl;
}

int main(int argc, char** argv) {
	PolynomialMap p1("../data/P3.txt");
	PolynomialMap p2("../data/P4.txt");
	PolynomialMap p3;
	p1.Print();
	p2.Print();

	p3 = p1 + p2;
	p3.Print();
	p3 = p1 - p2;
	p3.Print();

	p3 = p1 * p2;
	p3.Print();
	
	// my extended tests
	p3 = p3 + PolynomialMap({ 1 }, { 0.0000001 });
	p3.Print();
	p3.compress();
	p3.Print();

	// Polynomial Speed Test
	do_poly_test<PolynomialMap>();
	do_poly_test<PolynomialList>();

	return 0;
}