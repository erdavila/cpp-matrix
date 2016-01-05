#ifndef SAFELY_CONSTRUCTED_ARRAY_HPP_
#define SAFELY_CONSTRUCTED_ARRAY_HPP_

#include "storage.hpp"


namespace matrix {


template <typename T, unsigned Size, bool Verified = false>
class safely_constructed_array {
public:
	using value_type = T;
	enum { size = Size };

	safely_constructed_array()
		: safely_constructed_array(
			[](unsigned _index) {
				return T();
			}
		)
	{}

	safely_constructed_array(const safely_constructed_array&);

	safely_constructed_array(safely_constructed_array&&);

	template <typename U, bool V>
	safely_constructed_array(const safely_constructed_array<U, Size, V>&);

	safely_constructed_array(const T(&)[Size]);

	safely_constructed_array(T(&& array)[Size])
		: safely_constructed_array(
			[&](unsigned index) -> T&& {
				return std::move(array[index]);
			}
		)
	{}

	template <typename U>
	safely_constructed_array(const U(& array)[Size])
		: safely_constructed_array(
			[&](unsigned index) -> const U& {
				return array[index];
			}
		)
	{}

	template <typename P>
	safely_constructed_array(P provider) {
		unsigned index;
		try {
			for(index = 0; index < Size; ++index) {
				values[index].construct_value(provider(index));
			}
		} catch(...) {
			destruct(index);
			throw;
		}
	}

	~safely_constructed_array() {
		destruct(Size);
	}

	safely_constructed_array& operator=(const safely_constructed_array&) &;

	safely_constructed_array& operator=(safely_constructed_array&&) &;

	template <typename U, bool V>
	safely_constructed_array& operator=(const safely_constructed_array<U, Size, V>&) &;

	T& operator[](unsigned index) {
		return values[index].value_reference();
	}

	const T& operator[](unsigned index) const {
		return values[index].value_reference();
	}

private:
	storage<T, Verified> values[Size];

	void destruct(unsigned count) {
		while(count > 0) {
			values[--count].destruct_value();
		}
	}
};


} /* namespace matrix */


#endif /* SAFELY_CONSTRUCTED_ARRAY_HPP_ */
