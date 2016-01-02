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


template <unsigned RowsL, unsigned ColsL, unsigned RowsR, unsigned ColsR>
inline void static_assert_smatrix_same_shape() {
	static_assert(RowsL == RowsR  &&  ColsL == ColsR, "Both smatrices must have the same shape for this operation");
}

template <unsigned Rows, unsigned Cols>
inline void static_assert_smatrix_1x1() {
	static_assert(Rows == 1  &&  Cols == 1, "The smatrix must be 1x1 for this operation");
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator==(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_smatrix_same_shape<RowsL, ColsL, RowsR, ColsR>();
	return equal_to(lhs, rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator==(const smatrix<T, Rows, Cols>& lhs, const T& rhs) {
	static_assert_smatrix_1x1<Rows, Cols>();
	return lhs.element_at(0, 0) == rhs;
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator==(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	return rhs == lhs;
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator!=(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	return !(lhs == rhs);
}

template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator!=(const smatrix<TL, RowsL, ColsL>& lhs, const TR& rhs) {
	return !(lhs == rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator!=(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	return !(lhs == rhs);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_smatrix_1x1<RowsL, ColsL>();
	static_assert_smatrix_1x1<RowsR, ColsR>();
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator<(const smatrix<T, Rows, Cols>& lhs, const T& rhs) {
	static_assert_smatrix_1x1<Rows, Cols>();
	return lhs.element_at(0, 0) < rhs;
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator<(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	static_assert_smatrix_1x1<Rows, Cols>();
	return lhs < rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	return rhs < lhs;
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator>(const smatrix<T, Rows, Cols>& lhs, const T& rhs) {
	return rhs < lhs;
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator>(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	return rhs < lhs;
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<=(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	return !(lhs > rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator<=(const smatrix<T, Rows, Cols>& lhs, const T& rhs) {
	return !(lhs > rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator<=(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	return !(lhs > rhs);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>=(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	return !(lhs < rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator>=(const smatrix<T, Rows, Cols>& lhs, const T& rhs) {
	return !(lhs < rhs);
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator>=(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	return !(lhs < rhs);
}


} /* namespace matrix */


#endif /* SMATRIX_HPP_ */
