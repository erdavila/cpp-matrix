#include "matrix.hpp"
#include "storage.hpp"
#include <cassert>
#include <type_traits>


#define assert_throws(EXPR, EXCEPTION) \
	try { \
		EXPR; \
		assert(false); \
	} catch(EXCEPTION&) { \
		assert(true); \
	}


namespace storage {
	template <typename T>
	using verified_storage = matrix::storage<T, true>;

	void testRegularFlow()
	try {
		verified_storage<int> s;
		s.construct_value(7);
		assert(s.value_reference() == 7);
		s.value_reference() = 3;
		assert(s.value_reference() == 3);
		s.destruct_value();
	} catch(matrix::storage_verifier::exception&) {
		assert(false);
	}

	void testUseValueNotConstructed()
	try {
		{
			verified_storage<int> s;
			assert_throws(s.value_reference(), matrix::storage_verifier::exception);
		}

		{
			verified_storage<int> s;
			s.construct_value();
			s.destruct_value();
			assert_throws(s.value_reference(), matrix::storage_verifier::exception);
		}
	} catch(matrix::storage_verifier::exception&) {
		assert(false);
	}

	void testDoubleConstruction()
	try {
		verified_storage<int> s;
		s.construct_value();
		assert_throws(s.construct_value(), matrix::storage_verifier::exception);
		s.destruct_value();
	} catch(matrix::storage_verifier::exception&) {
		assert(false);
	}

	void testDoubleDestruction()
	try {
		{
			verified_storage<int> s;
			assert_throws(s.destruct_value(), matrix::storage_verifier::exception);
		}

		{
			verified_storage<int> s;
			s.construct_value();
			s.destruct_value();
			assert_throws(s.destruct_value(), matrix::storage_verifier::exception);
		}
	} catch(matrix::storage_verifier::exception&) {
		assert(false);
	}

	void test() {
		testRegularFlow();
		testUseValueNotConstructed();
		testDoubleConstruction();
		testDoubleDestruction();
	}
} /* namespace storage */


namespace safely_constructed_array {
	void test() {

	}
} /* namespace safely_constructed_array */


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
	storage::test();
	safely_constructed_array::test();
	smatrix::test();
	dmatrix::test();
	common::test();
}
