#ifndef SMATRIX_HPP_
#define SMATRIX_HPP_

#include "safely_constructed_array.hpp"


namespace matrix {


template <typename T, unsigned Rows, unsigned Cols>
class smatrix {
public:
	using element_type = T;

	static constexpr unsigned rows() noexcept { return Rows; }

	static constexpr unsigned cols() noexcept { return Cols; }

	smatrix() = default;

	smatrix(const smatrix&);

	smatrix(smatrix&&);

	template <typename U>
	smatrix(const smatrix<U, Rows, Cols>&);

	smatrix(const T(&)[Rows][Cols]);

	smatrix(T(&& array)[Rows][Cols])
		: elements(
			[&](unsigned index) -> T& {
				indexes i = from_linear_index(index);
				return array[i.row][i.col];
			}
		)
	{}

	template <typename U>
	smatrix(const U(&)[Rows][Cols]);

	~smatrix() = default;

	smatrix& operator=(const smatrix&) &;

	smatrix& operator=(smatrix&&) &;

	template <typename U>
	smatrix& operator=(const smatrix<U, Rows, Cols>&) &;

	T& element_at(unsigned row, unsigned col) noexcept {
		unsigned index = to_linear_index(row, col);
		return elements[index];
	}

	const T& element_at(unsigned row, unsigned col) const noexcept {
		unsigned index = to_linear_index(row, col);
		return elements[index];
	}

private:
	safely_constructed_array<T, Rows * Cols> elements;

	struct indexes {
		unsigned row;
		unsigned col;
	};

	static unsigned to_linear_index(unsigned row, unsigned col) noexcept {
		return row * Cols + col;
	}

	static indexes from_linear_index(unsigned index) noexcept {
		return { index / Cols, index % Cols };
	}
};


template <typename TL, typename TR, unsigned Rows, unsigned Cols>
bool operator==(const smatrix<TL, Rows, Cols>& lhs, const smatrix<TR, Rows, Cols>& rhs) {
	for(unsigned row = 0; row < Rows; ++row) {
		for(unsigned col = 0; col < Cols; ++col) {
			if(lhs.element_at(row, col) != rhs.element_at(row, col)) {
				return false;
			}
		}
	}
	return true;
}

template <typename TL, typename TR>
inline
bool operator==(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return lhs.element_at(0, 0) == rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator==(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return lhs.element_at(0, 0) == rhs;
}

template <typename T>
inline
bool operator==(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return rhs == lhs;
}


template <typename TL, typename TR, unsigned Rows, unsigned Cols>
inline
bool operator!=(const smatrix<TL, Rows, Cols>& lhs, const smatrix<TR, Rows, Cols>& rhs) {
	return !(lhs == rhs);
}

template <typename TL, typename TR>
inline
bool operator!=(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return !(lhs == rhs);
}

template <typename T>
inline
bool operator!=(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return !(lhs == rhs);
}

template <typename T>
inline
bool operator!=(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return !(lhs == rhs);
}


template <typename TL, typename TR>
inline
bool operator<(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator<(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return lhs.element_at(0, 0) < rhs;
}

template <typename T>
inline
bool operator<(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return lhs < rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator>(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return rhs < lhs;
}

template <typename T>
inline
bool operator>(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return rhs < lhs;
}

template <typename T>
inline
bool operator>(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return rhs < lhs;
}


template <typename TL, typename TR>
inline
bool operator<=(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return !(lhs > rhs);
}

template <typename T>
inline
bool operator<=(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return !(lhs > rhs);
}

template <typename T>
inline
bool operator<=(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return !(lhs > rhs);
}


template <typename TL, typename TR>
inline
bool operator>=(const smatrix<TL, 1, 1>& lhs, const smatrix<TR, 1, 1>& rhs) {
	return !(lhs < rhs);
}

template <typename T>
inline
bool operator>=(const smatrix<T, 1, 1>& lhs, const T& rhs) {
	return !(lhs < rhs);
}

template <typename T>
inline
bool operator>=(const T& lhs, const smatrix<T, 1, 1>& rhs) {
	return !(lhs < rhs);
}


} /* namespace matrix */


#endif /* SMATRIX_HPP_ */
