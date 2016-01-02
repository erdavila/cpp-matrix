#include "matrix.hpp"
#include "safely_constructed_array.hpp"
#include "storage.hpp"
#include <cassert>
#include <string>
#include <type_traits>
#include <vector>


#define assert_throws(EXPR, EXCEPTION) \
	try { \
		EXPR; \
		assert(false); \
	} catch(EXCEPTION&) { \
		assert(true); \
	}

#define assert_not_compilable(EXPR) \
	assert(false); EXPR;


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
	struct Probe {
		char id;
		int *p;

		Probe();
		Probe(const Probe&);
		Probe(Probe&& p) : id(std::move(p.id)), p(p.p) {
			if(throwing_id_on_move == p.id) {
				log.push_back("throwing");
				throw std::runtime_error("throwing!");
			}
			id += 32; // to lower case
			*this->p += 32;
			p.p = nullptr;
			log.push_back(id + std::string(": move-constructed"));
		}
		Probe(char id) : id(id), p(new int(id - 'A')) {
			log.push_back(id + std::string(": char-constructed"));
		}
		~Probe() {
			delete p;
			log.push_back(id + std::string(": destructed"));
		}
		Probe& operator=(const Probe&);
		Probe& operator=(Probe&&);
		Probe& operator=(char id) {
			this->id = id;
			*p = id - 'A';
			log.push_back(id + std::string(": char-assigned"));
			return *this;
		}
		operator char&() {
			return id;
		}
		operator const char&() const;

		static char throwing_id_on_move;
		static std::vector<std::string> log;

		static void reset() {
			throwing_id_on_move = -1;
			log.clear();
		}
	};
	char Probe::throwing_id_on_move;
	std::vector<std::string> Probe::log;

	void testConstructWithProvider() {
		auto provider = [](unsigned index) {
			return Probe('A' + index);
		};
		matrix::safely_constructed_array<Probe, 3> sca(provider);

		assert(sca[0] == 'a');
		assert(sca[1] == 'b');
		assert(sca[2] == 'c');
	}

	void testConstructWithArrayAndChangeValue() {
		matrix::safely_constructed_array<Probe, 3> sca({ 'A', 'B', 'C' });

		sca[1] = 'X';

		assert(sca[0] == 'a');
		assert(sca[1] == 'X');
		assert(sca[2] == 'c');
	}

	void testConstructionThrowingOnMove() {
		Probe::reset();
		Probe::throwing_id_on_move = 'B';

		try {
			matrix::safely_constructed_array<Probe, 3> array(
				[](unsigned index) {
					return Probe('A' + index);
				}
			);
			assert(false);
		} catch(std::runtime_error&) {
			Probe::log.push_back("exception caught");
		}

		assert((Probe::log == std::vector<std::string>{
			"A: char-constructed",
			"a: move-constructed",
			"A: destructed",
			"B: char-constructed",
			"throwing",
			"B: destructed",
			"a: destructed",
			"exception caught"
		}));
	}

	void test() {
		testConstructWithProvider();
		testConstructWithArrayAndChangeValue();
		testConstructionThrowingOnMove();
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

	void testArrayConstructorAndElementAt() {
		matrix::smatrix<int, 2, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 } });
		assert(m.element_at(0, 0) == 1);
		assert(m.element_at(0, 1) == 2);
		assert(m.element_at(0, 2) == 3);
		assert(m.element_at(1, 0) == 4);
		assert(m.element_at(1, 1) == 5);
		assert(m.element_at(1, 2) == 6);
	}

	void testMatrixMatrixComparison() {
		matrix::smatrix<int, 2, 3>  mA({ { 1, 2, 3 },
		                                 { 4, 5, 6 } });

		matrix::smatrix<char, 2, 3> mB({ { 1, 2, 3 },
		                                 { 4, 5, 6 } });

		matrix::smatrix<int, 2, 3>  mC({ { 1, 2, 3 },
		                                 { 6, 6, 6 } });

		assert(  mA == mA ); assert(!(mA != mA));
		assert(  mA == mB ); assert(!(mA != mB));
		assert(!(mA == mC)); assert(  mA != mC );


		matrix::smatrix<int, 3, 2> mX({ { 1, 2 },
		                                { 3, 4 },
		                                { 5, 6 } });
		//assert_not_compilable(mA == mX);
		//assert_not_compilable(mA != mX);
	}

	void testMatrixScalarComparison() {
		matrix::smatrix<int , 1, 1> mA({ { 7 } });
		matrix::smatrix<char, 1, 1> mB({ { 7 } });
		matrix::smatrix<int , 1, 1> mC({ { 3 } });
		matrix::smatrix<int , 1, 1> mD({ { 9 } });

		matrix::smatrix<int , 3, 2> mX({ { 1, 2 },
		                                 { 3, 4 },
		                                 { 5, 6 } });

		assert(  mA == mA ); assert(!(mA != mA));
		assert(  mA == mB ); assert(!(mA != mB));
		assert(!(mA == mC)); assert(  mA != mC );
		assert(!(mA == mD)); assert(  mA != mD );
		assert(!(mA == 3));  assert(  mA != 3 );
		assert(  mA == 7 );  assert(!(mA != 7));
		assert(!(mA == 9));  assert(  mA != 9 );
		//assert_not_compilable(mX == mA);
		//assert_not_compilable(mX != mA);
		//assert_not_compilable(mX == 7);
		//assert_not_compilable(mX != 7);

		assert(!(mA < mA));  assert(  mA >= mA );
		assert(!(mA < mB));  assert(  mA >= mB );
		assert(!(mA < mC));  assert(  mA >= mC );
		assert(  mA < mD );  assert(!(mA >= mD));
		assert(!(mA < 3));   assert(  mA >= 3 );
		assert(!(mA < 7));   assert(  mA >= 7 );
		assert(  mA < 9 );   assert(!(mA >= 9));
		//assert_not_compilable(mX <  mA);
		//assert_not_compilable(mX >= mA);
		//assert_not_compilable(mX <  7);
		//assert_not_compilable(mX >= 7);

		assert(!(mA > mA));  assert(  mA <= mA );
		assert(!(mA > mB));  assert(  mA <= mB );
		assert(  mA > mC );  assert(!(mA <= mC));
		assert(!(mA > mD));  assert(  mA <= mD );
		assert(  mA > 3 );   assert(!(mA <= 3));
		assert(!(mA > 7));   assert(  mA <= 7 );
		assert(!(mA > 9));   assert(  mA <= 9 );
		//assert_not_compilable(mX >  mA);
		//assert_not_compilable(mX <= mA);
		//assert_not_compilable(mX >  7);
		//assert_not_compilable(mX <= 7);
	}

	void test() {
		testBasics();
		testArrayConstructorAndElementAt();
		testMatrixMatrixComparison();
		testMatrixScalarComparison();
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

	void testInitializerListConstructorAndElementAt() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 } });
		assert(m.element_at(0, 0) == 1);
		assert(m.element_at(0, 1) == 2);
		assert(m.element_at(0, 2) == 3);
		assert(m.element_at(1, 0) == 4);
		assert(m.element_at(1, 1) == 5);
		assert(m.element_at(1, 2) == 6);
	}

	void testInitializerListConstructorWithMissingValues() {
		matrix::dmatrix<int> m({ { 1       },
		                         { 4, 5, 6 },
		                         { 7, 8    }});

		assert(m.element_at(0, 0) == 1);
		assert(m.element_at(0, 1) == 0);
		assert(m.element_at(0, 2) == 0);
		assert(m.element_at(1, 0) == 4);
		assert(m.element_at(1, 1) == 5);
		assert(m.element_at(1, 2) == 6);
		assert(m.element_at(2, 0) == 7);
		assert(m.element_at(2, 1) == 8);
		assert(m.element_at(2, 2) == 0);
	}

	void testSizesWithInitializerListConstructor() {
		matrix::dmatrix<int> m(3, 2, { { 1       },
		                               { 2, 3, 4 }});

		assert(m.element_at(0, 0) == 1);
		assert(m.element_at(0, 1) == 0);
		assert(m.element_at(1, 0) == 2);
		assert(m.element_at(1, 1) == 3);
		assert(m.element_at(2, 0) == 0);
		assert(m.element_at(2, 1) == 0);
	}

	void testMatrixMatrixComparison() {
		matrix::dmatrix<int>  mA({ { 1, 2, 3 },
		                           { 4, 5, 6 } });

		matrix::dmatrix<char> mB({ { 1, 2, 3 },
		                           { 4, 5, 6 } });

		matrix::dmatrix<int>  mC({ { 1, 2, 3 },
		                           { 6, 6, 6 } });

		matrix::dmatrix<int>  mD({ { 1, 2 },
		                           { 3, 4 },
		                           { 5, 6 } });

		assert(  mA == mA ); assert(!(mA != mA));
		assert(  mA == mB ); assert(!(mA != mB));
		assert(!(mA == mC)); assert(  mA != mC );
		assert_throws((void)(mA == mD), matrix::incompatible_operands);
		assert_throws((void)(mA != mD), matrix::incompatible_operands);
	}

	void testMatrixScalarComparison() {
		matrix::dmatrix<int>  mA({ { 7 } });
		matrix::dmatrix<char> mB({ { 7 } });
		matrix::dmatrix<int>  mC({ { 3 } });
		matrix::dmatrix<int>  mD({ { 9 } });
		matrix::dmatrix<int>  mE({ { 1, 2, 3 },
		                           { 4, 5, 6 } });

		assert(  mA == mA ); assert(!(mA != mA));
		assert(  mA == mB ); assert(!(mA != mB));
		assert(!(mA == mC)); assert(  mA != mC );
		assert(!(mA == mD)); assert(  mA != mD );
		assert_throws((void)(mA == mE), matrix::incompatible_operands);
		assert_throws((void)(mA != mE), matrix::incompatible_operands);
		assert(!(mA == 3));  assert(  mA != 3 );
		assert(  mA == 7 );  assert(!(mA != 7));
		assert(!(mA == 9));  assert(  mA != 9 );

		assert(!(mA < mA));  assert(  mA >= mA );
		assert(!(mA < mB));  assert(  mA >= mB );
		assert(!(mA < mC));  assert(  mA >= mC );
		assert(  mA < mD );  assert(!(mA >= mD));
		assert_throws((void)(mA <  mE), matrix::incompatible_operands);
		assert_throws((void)(mA >= mE), matrix::incompatible_operands);
		assert(!(mA < 3));   assert(  mA >= 3 );
		assert(!(mA < 7));   assert(  mA >= 7 );
		assert(  mA < 9 );   assert(!(mA >= 9));

		assert(!(mA > mA));  assert(  mA <= mA );
		assert(!(mA > mB));  assert(  mA <= mB );
		assert(  mA > mC );  assert(!(mA <= mC));
		assert(!(mA > mD));  assert(  mA <= mD );
		assert_throws((void)(mA >  mE), matrix::incompatible_operands);
		assert_throws((void)(mA <= mE), matrix::incompatible_operands);
		assert(  mA > 3 );   assert(!(mA <= 3));
		assert(!(mA > 7));   assert(  mA <= 7 );
		assert(!(mA > 9));   assert(  mA <= 9 );
	}

	void test() {
		testBasics();
		testInitializerListConstructorAndElementAt();
		testInitializerListConstructorWithMissingValues();
		testSizesWithInitializerListConstructor();
		testMatrixMatrixComparison();
		testMatrixScalarComparison();
	}
} /* namespace dmatrix */


namespace common {
	void testSMatrixDMatrixComparison() {
		matrix::smatrix<int, 2, 3> smA({ { 1, 2, 3 },
		                                 { 4, 5, 6 } });

		matrix::smatrix<int, 2, 3> smB({ { 1, 2, 3 },
		                                 { 6, 6, 6 } });

		matrix::smatrix<int, 3, 2> smC({ { 1, 2 },
		                                 { 3, 4 },
		                                 { 5, 6 } });

		matrix::dmatrix<char> dmA({ { 1, 2, 3 },
		                            { 4, 5, 6 } });

		matrix::dmatrix<char> dmB({ { 1, 2, 3 },
		                            { 6, 6, 6 } });

		matrix::dmatrix<char> dmC({ { 1, 2 },
		                            { 3, 4 },
		                            { 5, 6 } });

		assert(  smA == dmA);  assert(!(smA != dmA));
		assert(!(smA == dmB)); assert(  smA != dmB);
		assert_throws(static_cast<void>(smA == dmC), matrix::incompatible_operands);
		assert_throws(static_cast<void>(smA != dmC), matrix::incompatible_operands);
		assert_throws(static_cast<void>(dmA == smC), matrix::incompatible_operands);
		assert_throws(static_cast<void>(dmA != smC), matrix::incompatible_operands);
	}

	void testScalarSMatrixDMatrixComparison() {
		matrix::smatrix<int, 1, 1> smA({ { 7 } });
		matrix::smatrix<int, 1, 1> smB({ { 3 } });
		matrix::smatrix<int, 1, 1> smC({ { 9 } });
		matrix::smatrix<int, 2, 3> smX({ { 1, 2, 3 },
		                                 { 4, 5, 6 } });
		matrix::dmatrix<char> dmA({ { 7 } });
		matrix::dmatrix<char> dmB({ { 3 } });
		matrix::dmatrix<char> dmC({ { 9 } });
		matrix::dmatrix<char> dmX({ { 1, 2, 3 },
		                            { 4, 5, 6 } });

		assert(  smA == dmA);  assert(!(smA != dmA));
		assert(!(smA == dmB)); assert(  smA != dmB );
		assert(!(smA == dmC)); assert(  smA != dmC );
		assert_throws(static_cast<void>(smA == dmX), matrix::incompatible_operands);
		assert_throws(static_cast<void>(smA != dmX), matrix::incompatible_operands);
		assert(!(dmA == smB)); assert(  dmA != smB );
		assert(!(dmA == smC)); assert(  dmA != smC );
		assert_throws(static_cast<void>(dmA == smX), matrix::incompatible_operands);
		assert_throws(static_cast<void>(dmA != smX), matrix::incompatible_operands);

		assert(!(smA < dmA)); assert(  smA >= dmA );
		assert(!(smA < dmB)); assert(  smA >= dmB );
		assert(  smA < dmC ); assert(!(smA >= dmC));
		assert_throws(static_cast<void>(smA <  dmX), matrix::incompatible_operands);
		assert_throws(static_cast<void>(smA >= dmX), matrix::incompatible_operands);
		assert(!(dmA < smB)); assert(  dmA >= smB );
		assert(  dmA < smC ); assert(!(dmA >= smC));
		//assert_not_compilable(dmA <  smX);
		//assert_not_compilable(dmA >= smX);

		assert(!(smA > dmA)); assert(  smA <= dmA );
		assert(  smA > dmB ); assert(!(smA <= dmB));
		assert(!(smA > dmC)); assert(  smA <= dmC );
		assert_throws(static_cast<void>(smA >  dmX), matrix::incompatible_operands);
		assert_throws(static_cast<void>(smA <= dmX), matrix::incompatible_operands);
		assert(  dmA > smB ); assert(!(dmA <= smB));
		assert(!(dmA > smC)); assert(  dmA <= smC );
		//assert_not_compilable(dmA >  smX);
		//assert_not_compilable(dmA <= smX);
	}

	void test() {
		testSMatrixDMatrixComparison();
		testScalarSMatrixDMatrixComparison();
	}
} /* namespace common */


int main() {
	storage::test();
	safely_constructed_array::test();
	smatrix::test();
	dmatrix::test();
	common::test();
}
