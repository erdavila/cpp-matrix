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
