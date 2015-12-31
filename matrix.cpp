#include "matrix.hpp"
#include <cassert>
#include <type_traits>


namespace smatrix {
	void testBasics() {
		using Matrix = matrix::smatrix<int, 2, 3>;
		assert((std::is_same<Matrix::element_type, int>()));
		assert(Matrix::rows() == 2);
		assert(Matrix::cols() == 3);

		Matrix m;
		assert(m.rows() == 2);
		assert(m.cols() == 3);
	}

	void test() {
		testBasics();
	}
} /* namespace smatrix */


namespace dmatrix {
	void testBasics() {
		using Matrix = matrix::dmatrix<int>;
		assert((std::is_same<Matrix::element_type, int>()));

		Matrix m(2, 3);
		assert(m.rows() == 2);
		assert(m.cols() == 3);
	}

	void test() {
		testBasics();
	}
} /* namespace dmatrix */


namespace common {
	void test() {

	}
} /* namespace common */


int main() {
	smatrix::test();
	dmatrix::test();
	common::test();
}
