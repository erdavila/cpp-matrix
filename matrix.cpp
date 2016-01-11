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
		assert("Exception not thrown" && false); \
	} catch(EXCEPTION&) { \
		assert(true); \
	}

#define assert_not_compilable(EXPR) \
	assert("Should not compile" && false); EXPR;


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
		char ch;
		int count;
		int *ptr;

		std::string id() {
			char chCount = '0' + count;
			return std::string() + ch + chCount;
		}

		Probe();
		Probe(const Probe&);
		Probe(Probe&& p)
			: ch(std::move(p.ch)),
			  count(p.count + 1),
			  ptr(p.ptr)
		{
			if(throwing_ch_on_move == p.ch) {
				log.push_back("throwing on move");
				throw std::runtime_error("throwing!");
			}
			p.ptr = nullptr;
			log.push_back(id() + ": move-constructed");
		}
		Probe(int ch) : ch(ch), count(0), ptr(new int(ch - 'A')) {
			log.push_back(id() + ": char-constructed");
		}
		~Probe() {
			delete ptr;
			log.push_back(id() + ": destructed");
		}
		Probe& operator=(const Probe&);
		Probe& operator=(Probe&&);
		Probe& operator=(char ch) {
			log.push_back(id() + "->" + ch + ": char-assigned");
			this->ch = ch;
			count = 0;
			*ptr = ch - 'A';
			return *this;
		}
		operator char&() {
			return ch;
		}
		operator const char&() const;

		static char throwing_ch_on_move;
		static std::vector<std::string> log;

		static void reset() {
			throwing_ch_on_move = -1;
			log.clear();
		}
	};
	char Probe::throwing_ch_on_move;
	std::vector<std::string> Probe::log;

	void testConstructWithProvider() {
		auto provider = [](unsigned index) {
			return Probe('A' + index);
		};
		matrix::safely_constructed_array<Probe, 3> sca(provider);

		assert(sca[0] == 'A');
		assert(sca[1] == 'B');
		assert(sca[2] == 'C');
	}

	void testConstructWithArrayAndChangeValue() {
		matrix::safely_constructed_array<Probe, 3> sca({ 'A', 'B', 'C' });

		sca[1] = 'x';

		assert(sca[0] == 'A');
		assert(sca[1] == 'x');
		assert(sca[2] == 'C');
	}

	void testConstructionThrowingOnMove() {
		Probe::reset();
		Probe::throwing_ch_on_move = 'B';

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
			"A0: char-constructed",
			"A1: move-constructed",
			"A0: destructed",
			"B0: char-constructed",
			"throwing on move",
			"B0: destructed",
			"A1: destructed",
			"exception caught"
		}));
	}

	void test() {
		testConstructWithProvider();
		testConstructWithArrayAndChangeValue();
		testConstructionThrowingOnMove();
	}
} /* namespace safely_constructed_array */


namespace base {
	template <typename T, typename F, typename Expected>
	struct test_with_qualifiers_of {
		using Result = typename matrix::__impl::with_qualifiers_of<F, T>::type;
		static constexpr bool value = std::is_same<Result, Expected>::value;
	};

	void testWithQualifiersOf() {
		assert((test_with_qualifiers_of<const char& ,       int  ,       char  >::value));
		assert((test_with_qualifiers_of<const char&&,       int  ,       char  >::value));
		assert((test_with_qualifiers_of<      char  , const int  , const char  >::value));
		assert((test_with_qualifiers_of<      char  ,       int& ,       char& >::value));
		assert((test_with_qualifiers_of<      char  , const int& , const char& >::value));
		assert((test_with_qualifiers_of<      char  ,       int&&,       char&&>::value));
		assert((test_with_qualifiers_of<      char  , const int&&, const char&&>::value));
	}

	void test() {
		testWithQualifiersOf();
	}
} /* namespace base */


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

	void testDefaultConstructor() {
		matrix::smatrix<int, 2, 3> m;
		assert(m.element_at(0, 0) == 0);
		assert(m.element_at(0, 1) == 0);
		assert(m.element_at(0, 2) == 0);
		assert(m.element_at(1, 0) == 0);
		assert(m.element_at(1, 1) == 0);
		assert(m.element_at(1, 2) == 0);
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

	template <typename SMatrix>
	bool checkRowIndexSubscript(unsigned row_index, SMatrix& m) {
		using rows_ref_type = decltype(
			m[row_index]
		);

		return std::is_same<
				rows_ref_type,
				matrix::smatrix_rows_reference<SMatrix, 1, SMatrix::cols()>
			>();
	}

	void testRowIndexSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkRowIndexSubscript(1, m));
	}

	template <unsigned Rows, typename SMatrix>
	bool checkRowsRangeSubscript(unsigned first_row, SMatrix& m) {
		using rows_ref_type = decltype(
			m[matrix::srange<Rows>(first_row)]
		);

		return std::is_same<
				rows_ref_type,
				matrix::smatrix_rows_reference<SMatrix, Rows, SMatrix::cols()>
			>();
	}

	void testRowsRangeSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkRowsRangeSubscript<1>(1, m));
		assert(checkRowsRangeSubscript<2>(0, m));
		assert(checkRowsRangeSubscript<3>(0, m));
	}

	template <typename SMatrix>
	bool checkAllRowsSubscript(SMatrix& m) {
		using rows_ref_type = decltype(
			m[matrix::all]
		);

		return std::is_same<
				rows_ref_type,
				matrix::smatrix_rows_reference<SMatrix, SMatrix::rows(), SMatrix::cols()>
			>();
	}

	void testAllRowsSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkAllRowsSubscript(m));
	}

	void testSingleRowSingleColumnRowsReference() {
		matrix::smatrix<int, 5, 1> m({ { 1 },
		                               { 2 },
		                               { 3 },
		                               { 4 },
		                               { 5 } });

		assert(m[1] == (matrix::smatrix<int, 1, 1>({ { 2 } })));

		assert(m[3].element_at(0, 0) == 4);
		int& n = m[3].element_at(0, 0);
		assert(n == 4);

		assert(m[4] == 5);
		int& r = m[4];
		assert(r == 5);

		m[0] = matrix::smatrix<int, 1, 1>({ { 6 } });
		m[1].element_at(0, 0) = 7;
		m[2] = 8;
		n = 9;
		r = 0;

		assert(m == (matrix::smatrix<int, 5, 1>({ { 6 },
		                                          { 7 },
		                                          { 8 },
		                                          { 9 },
		                                          { 0 } })));

		//assert_not_compilable(m[2] = (matrix::smatrix<int, 1, 2>({ { 3, 4 } })));
	}

	void testMultiRowOrMultiColumnRowsReference() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(m[2] == (matrix::smatrix<int, 1, 3>({ { 7, 8, 9 } })));
		assert(m[2].element_at(0, 1) == 8);
		int& n = m[2].element_at(0, 1);
		assert(n == 8);

		m[0] = matrix::smatrix<int, 1, 3>({ { 3, 4, 7 } });
		m[1].element_at(0, 0) = -1;
		n = 100;

		assert(m == (matrix::smatrix<int, 3, 3>({ {  3,   4, 7 },
		                                          { -1,   5, 6 },
		                                          {  7, 100, 9 } })));

		//assert_not_compilable(m[2] == 0);
		//assert_not_compilable(m[2] = 0);
		//assert_not_compilable(int r = m[2]);
		//assert_not_compilable(m[2] = (matrix::smatrix<int, 1, 2>({ { 3, 4 } })));
	}

	template <typename RowsRef>
	bool checkColumnIndexSubscript(unsigned col_index, RowsRef rows_ref) {
		using area_ref_type = decltype(
			rows_ref[col_index]
		);

		return std::is_same<
				area_ref_type,
				matrix::smatrix_area_reference<
						typename RowsRef::referred_matrix_type,
						RowsRef::rows(), 1
					>
			>();
	}

	void testColumnIndexSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkColumnIndexSubscript(1, m[matrix::srange<2>(1)]));
	}

	template <unsigned Cols, typename RowsRef>
	bool checkColumnsRangeSubscript(unsigned first_col, RowsRef rows_ref) {
		using area_ref_type = decltype(
			rows_ref[matrix::srange<Cols>(first_col)]
		);

		return std::is_same<
				area_ref_type,
				matrix::smatrix_area_reference<
						typename RowsRef::referred_matrix_type,
						RowsRef::rows(), Cols
					>
			>();
	}

	void testColumnsRangeSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkColumnsRangeSubscript<1>(1, m[matrix::srange<2>(0)]));
		assert(checkColumnsRangeSubscript<2>(0, m[matrix::srange<2>(1)]));
		assert(checkColumnsRangeSubscript<3>(0, m[matrix::srange<2>(0)]));
	}

	template <typename RowsRef>
	bool checkAllColumnsSubscript(RowsRef rows_ref) {
		using area_ref_type = decltype(
			rows_ref[matrix::all]
		);

		return std::is_same<
				area_ref_type,
				matrix::smatrix_area_reference<
						typename RowsRef::referred_matrix_type,
						RowsRef::rows(), RowsRef::cols()
					>
			>();
	}

	void testAllColumnsSubscript() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(checkAllColumnsSubscript(m[matrix::srange<2>(1)]));
	}

	void testSingleRowSingleColumnAreaReference() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(m[0][1] == (matrix::smatrix<int, 1, 1>({ { 2 } })));

		assert(m[0][2][0][0] == 3);

		assert(m[1][0].element_at(0, 0) == 4);
		int& n = m[1][0].element_at(0, 0);
		assert(n == 4);

		assert(m[1][1] == 5);
		int& r = m[1][1];
		assert(r == 5);

		n = 10;
		r = 11;
		m[1][2] = matrix::smatrix<int, 1, 1>({ { 12 } });
		m[2][0].element_at(0, 0) = 13;
		m[2][1] = 14;

		assert(m == (matrix::smatrix<int, 3, 3>({ {  1,  2,  3 },
		                                          { 10, 11, 12 },
		                                          { 13, 14,  9 } })));

		//assert_not_compilable(m[1][0] = (matrix::smatrix<int, 1, 2>({ { 3, 4 } })));
	}

	void testMultiRowOrMultiColumnAreaReference() {
		matrix::smatrix<int, 3, 3> m({ { 1, 2, 3 },
		                               { 4, 5, 6 },
		                               { 7, 8, 9 } });

		assert(m[2][matrix::all] == (matrix::smatrix<int, 1, 3>({ { 7, 8, 9 } })));

		assert(m[1][matrix::all][0][1] == 5);

		assert(m[2][matrix::all].element_at(0, 1) == 8);
		int& n = m[2][matrix::all].element_at(0, 1);
		assert(n == 8);

		m[0][matrix::all] = matrix::smatrix<int, 1, 3>({ { 3, 4, 7 } });
		m[1][matrix::all].element_at(0, 2) = -1;
		n = 100;

		assert(m == (matrix::smatrix<int, 3, 3>({ { 3,   4,  7 },
		                                          { 4,   5, -1 },
		                                          { 7, 100,  9 } })));

		//assert_not_compilable(m[2][matrix::all] == 0);
		//assert_not_compilable(m[2][matrix::all] = 0);
		//assert_not_compilable(int r = m[2][matrix::all]);
		//assert_not_compilable(m[2][matrix::all] = (matrix::smatrix<int, 1, 2>({ { 3, 4 } })));
	}

	void test() {
		testBasics();
		testArrayConstructorAndElementAt();
		testDefaultConstructor();
		testMatrixMatrixComparison();
		testMatrixScalarComparison();
		testRowIndexSubscript();
		testRowsRangeSubscript();
		testAllRowsSubscript();
		testSingleRowSingleColumnRowsReference();
		testMultiRowOrMultiColumnRowsReference();
		testColumnIndexSubscript();
		testColumnsRangeSubscript();
		testAllColumnsSubscript();
		testSingleRowSingleColumnAreaReference();
		testMultiRowOrMultiColumnAreaReference();
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

	void testDefaultConstructor() {
		matrix::dmatrix<int> m(3, 2);

		assert(m.element_at(0, 0) == 0);
		assert(m.element_at(0, 1) == 0);
		assert(m.element_at(1, 0) == 0);
		assert(m.element_at(1, 1) == 0);
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

	template <typename DMatrix>
	bool checkRowIndexSubscript(unsigned row_index, DMatrix& m) {
		auto rows_ref = m[row_index];
		using rows_ref_type = decltype(rows_ref);

		return rows_ref.rows() == 1
		    && rows_ref.cols() == m.cols()
		    && std::is_same<
				rows_ref_type,
				matrix::dmatrix_rows_reference<DMatrix>
			>();
	}

	void testRowIndexSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkRowIndexSubscript(1, m));
	}

	template <typename DMatrix>
	bool checkRowsRangeSubscript(unsigned rows, unsigned first_row, DMatrix& m) {
		auto rows_ref = m[matrix::drange(rows, first_row)];
		using rows_ref_type = decltype(rows_ref);

		return rows_ref.rows() == rows
		    && rows_ref.cols() == m.cols()
		    && std::is_same<
				rows_ref_type,
				matrix::dmatrix_rows_reference<DMatrix>
			>();
	}

	void testRowsRangeSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkRowsRangeSubscript(1, 1, m));
		assert(checkRowsRangeSubscript(2, 0, m));
		assert(checkRowsRangeSubscript(3, 0, m));
	}

	template <typename DMatrix>
	bool checkAllRowsSubscript(DMatrix& m) {
		auto rows_ref = m[matrix::all];
		using rows_ref_type = decltype(rows_ref);

		return rows_ref.rows() == m.rows()
		    && rows_ref.cols() == m.cols()
		    && std::is_same<
				rows_ref_type,
				matrix::dmatrix_rows_reference<DMatrix>
			>();
	}

	void testAllRowsSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkAllRowsSubscript(m));
	}

	void testSingleRowSingleColumnRowsReference() {
		matrix::dmatrix<int> m({ { 1 },
		                         { 2 },
		                         { 3 },
		                         { 4 },
		                         { 5 } });

		assert(m[1] == (matrix::dmatrix<int>({ { 2 } })));

		assert(m[3].element_at(0, 0) == 4);
		int& n = m[3].element_at(0, 0);
		assert(n == 4);

		assert(m[4] == 5);
		int& r = m[4];
		assert(r == 5);

		m[0] = matrix::dmatrix<int>({ { 6 } });
		m[1].element_at(0, 0) = 7;
		m[2] = 8;
		n = 9;
		r = 0;

		assert(m == (matrix::dmatrix<int>({ { 6 },
		                                    { 7 },
		                                    { 8 },
		                                    { 9 },
		                                    { 0 } })));

		assert_throws(m[2] = (matrix::dmatrix<int>({ { 3, 4 } })), matrix::incompatible_operands);
	}

	void testMultiRowOrMultiColumnRowsReference() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(m[2] == (matrix::dmatrix<int>({ { 7, 8, 9 } })));
		assert(m[2].element_at(0, 1) == 8);
		int& n = m[2].element_at(0, 1);
		assert(n == 8);

		m[0] = matrix::dmatrix<int>({ { 3, 4, 7 } });
		m[1].element_at(0, 0) = -1;
		n = 100;

		assert(m == (matrix::dmatrix<int>({ {  3,   4, 7 },
		                                    { -1,   5, 6 },
		                                    {  7, 100, 9 } })));

		assert_throws((void)(m[2] == 0), matrix::incompatible_operands);
		assert_throws(m[2] = 0, matrix::incompatible_operands);
		assert_throws(int r = m[2]; (void)r, matrix::incompatible_operands);
		assert_throws(m[2] = (matrix::dmatrix<int>({ { 3, 4 } })), matrix::incompatible_operands);
	}

	template <typename RowsRef>
	bool checkColumnIndexSubscript(unsigned col_index, RowsRef rows_ref) {
		auto area_ref = rows_ref[col_index];
		using area_ref_type = decltype(area_ref);

		return area_ref.rows() == rows_ref.rows()
		    && area_ref.cols() == 1
		    && std::is_same<
				area_ref_type,
				matrix::dmatrix_area_reference<typename RowsRef::referred_matrix_type>
			>();
	}

	void testColumnIndexSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkColumnIndexSubscript(1, m[matrix::drange(2, 1)]));
	}

	template <typename RowsRef>
	bool checkColumnsRangeSubscript(unsigned cols, unsigned first_col, RowsRef rows_ref) {
		auto area_ref = rows_ref[matrix::drange(cols, first_col)];
		using area_ref_type = decltype(area_ref);

		return area_ref.rows() == rows_ref.rows()
		    && area_ref.cols() == cols
		    && std::is_same<
				area_ref_type,
				matrix::dmatrix_area_reference<typename RowsRef::referred_matrix_type>
			>();
	}

	void testColumnsRangeSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkColumnsRangeSubscript(1, 1, m[matrix::drange(2, 0)]));
		assert(checkColumnsRangeSubscript(2, 0, m[matrix::drange(2, 1)]));
		assert(checkColumnsRangeSubscript(3, 0, m[matrix::drange(2, 0)]));
	}

	template <typename RowsRef>
	bool checkAllColumnsSubscript(RowsRef rows_ref) {
		auto area_ref = rows_ref[matrix::all];
		using area_ref_type = decltype(area_ref);

		return area_ref.rows() == rows_ref.rows()
		    && area_ref.cols() == rows_ref.cols()
		    && std::is_same<
				area_ref_type,
				matrix::dmatrix_area_reference<typename RowsRef::referred_matrix_type>
			>();
	}

	void testAllColumnsSubscript() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(checkAllColumnsSubscript(m[matrix::drange(2, 1)]));
	}

	void testSingleRowSingleColumnAreaReference() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(m[0][1] == (matrix::dmatrix<int>({ { 2 } })));

		assert(m[0][2][0][0] == 3);

		assert(m[1][0].element_at(0, 0) == 4);
		int& n = m[1][0].element_at(0, 0);
		assert(n == 4);

		assert(m[1][1] == 5);
		int& r = m[1][1];
		assert(r == 5);

		n = 10;
		r = 11;
		m[1][2] = matrix::dmatrix<int>({ { 12 } });
		m[2][0].element_at(0, 0) = 13;
		m[2][1] = 14;

		assert(m == (matrix::dmatrix<int>({ {  1,  2,  3 },
		                                    { 10, 11, 12 },
		                                    { 13, 14,  9 } })));

		assert_throws(m[1][0] = (matrix::dmatrix<int>({ { 3, 4 } })), matrix::incompatible_operands);
	}

	void testMultiRowOrMultiColumnAreaReference() {
		matrix::dmatrix<int> m({ { 1, 2, 3 },
		                         { 4, 5, 6 },
		                         { 7, 8, 9 } });

		assert(m[2][matrix::all] == (matrix::dmatrix<int>({ { 7, 8, 9 } })));

		assert(m[1][matrix::all][0][1] == 5);

		assert(m[2][matrix::all].element_at(0, 1) == 8);
		int& n = m[2][matrix::all].element_at(0, 1);
		assert(n == 8);

		m[0][matrix::all] = matrix::dmatrix<int>({ { 3, 4, 7 } });
		m[1][matrix::all].element_at(0, 2) = -1;
		n = 100;

		assert(m == (matrix::dmatrix<int>({ { 3,   4,  7 },
		                                    { 4,   5, -1 },
		                                    { 7, 100,  9 } })));

		assert_throws((void)(m[2][matrix::all] == 0), matrix::incompatible_operands);
		assert_throws(m[2][matrix::all] = 0, matrix::incompatible_operands);
		assert_throws(int r = m[2][matrix::all]; (void)r, matrix::incompatible_operands);
		assert_throws(m[2][matrix::all] = (matrix::dmatrix<int>({ { 3, 4 } })), matrix::incompatible_operands);
	}

	void test() {
		testBasics();
		testInitializerListConstructorAndElementAt();
		testInitializerListConstructorWithMissingValues();
		testSizesWithInitializerListConstructor();
		testDefaultConstructor();
		testMatrixMatrixComparison();
		testMatrixScalarComparison();
		testRowIndexSubscript();
		testRowsRangeSubscript();
		testAllRowsSubscript();
		testSingleRowSingleColumnRowsReference();
		testMultiRowOrMultiColumnRowsReference();
		testColumnIndexSubscript();
		testColumnsRangeSubscript();
		testAllColumnsSubscript();
		testSingleRowSingleColumnAreaReference();
		testMultiRowOrMultiColumnAreaReference();
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
	base::test();
	smatrix::test();
	dmatrix::test();
	common::test();
}
