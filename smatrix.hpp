#ifndef SMATRIX_HPP_
#define SMATRIX_HPP_

#include "safely_constructed_array.hpp"
#include <type_traits>
#include <utility>


namespace matrix {


template <typename M>
class static_matrix : public matrix<M> {};


// TODO: extract common members from smatrix_rows_reference and smatrix_area_reference to a common base


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_area_reference;


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_rows_reference {
private:
	template <unsigned ARows, unsigned ACols>
	using area_reference = smatrix_area_reference<SMatrix, ARows, ACols>;

	template <unsigned ARows, unsigned ACols>
	using const_area_reference = const smatrix_area_reference<const SMatrix, ARows, ACols>;

public:
	using element_type = typename SMatrix::element_type;

	smatrix_rows_reference() = delete;

	smatrix_rows_reference(const smatrix_rows_reference&);

	smatrix_rows_reference(smatrix_rows_reference&&);

	smatrix_rows_reference(SMatrix& smatrix, unsigned first_row, unsigned first_col)
		: smatrix(smatrix), first_row(first_row), first_col(first_col)
	{}

	~smatrix_rows_reference() = default;

	element_type& element_at(unsigned row, unsigned col) {
		return smatrix.element_at(first_row + row, first_col + col);
	}

	const element_type& element_at(unsigned row, unsigned col) const;

	area_reference<Rows, 1> operator[](unsigned col) {
		return { smatrix, first_row, first_col + col };
	}

	//const_area_reference<Rows, 1> operator[](unsigned col) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_area_reference<const SMatrix, Rows, 1> operator[](unsigned col) const;

private:
	SMatrix& smatrix;
	const unsigned first_row;
	const unsigned first_col;
};


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_area_reference : public static_matrix<smatrix_area_reference<SMatrix, Rows, Cols>> {
private:
	template <unsigned RRows, unsigned RCols>
	using rows_reference = smatrix_rows_reference<SMatrix, RRows, RCols>;

	template <unsigned RRows, unsigned RCols>
	using const_rows_reference = const smatrix_rows_reference<const SMatrix, RRows, RCols>;

public:
	using element_type = typename SMatrix::element_type;

	static constexpr unsigned rows() noexcept { return Rows; }
	static constexpr unsigned cols() noexcept { return Cols; }

	smatrix_area_reference() = delete;

	smatrix_area_reference(const smatrix_area_reference&);

	smatrix_area_reference(smatrix_area_reference&&);

	smatrix_area_reference(SMatrix& smatrix, unsigned first_row, unsigned first_col)
		: smatrix(smatrix), first_row(first_row), first_col(first_col)
	{}

	~smatrix_area_reference() = default;

	typename std::enable_if<Rows==1 && Cols==1, smatrix_area_reference>::type&
	operator=(const element_type&);

	typename std::enable_if<Rows==1 && Cols==1, smatrix_area_reference>::type&
	operator=(element_type&& value) {
		element_at(0, 0) = std::move(value);
		return *this;
	}

	element_type& element_at(unsigned row, unsigned col) {
		return smatrix.element_at(first_row + row, first_col + col);
	}

	const element_type& element_at(unsigned row, unsigned col) const {
		return smatrix.element_at(first_row + row, first_col + col);
	}

	rows_reference<1, Cols> operator[](unsigned row) {
		return { smatrix, first_row + row, first_col };
	}

	//const_rows_reference<1, Cols> operator[](unsigned row) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_rows_reference<const SMatrix, 1, Cols> operator[](unsigned row) const;

	operator typename std::enable_if<Rows==1 && Cols==1, element_type>::type&() {
		return element_at(0, 0);
	}

	operator const typename std::enable_if<Rows==1 && Cols==1, element_type>::type&() const;

private:
	SMatrix& smatrix;
	const unsigned first_row;
	const unsigned first_col;
};


template <typename T, unsigned Rows, unsigned Cols>
class smatrix : public static_matrix<smatrix<T, Rows, Cols>> {
private:
	template <unsigned RRows, unsigned RCols>
	using rows_reference = smatrix_rows_reference<smatrix, RRows, RCols>;

	template <unsigned RRows, unsigned RCols>
	using const_rows_reference = const smatrix_rows_reference<const smatrix, RRows, RCols>;

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

	rows_reference<1, Cols> operator[](unsigned row) {
		return { *this, row, 0 };
	}

	//const_rows_reference<1, Cols> operator[](unsigned row) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_rows_reference<const smatrix, 1, Cols> operator[](unsigned row) const;

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

template <typename M>
inline void static_assert_static_matrix_1x1() {
	static_assert(M::rows() == 1  &&  M::cols() == 1, "The smatrix must be 1x1 for this operation");
}

// TODO: remove this function when all operators below use static_matrix arguments instead of smatrix
template <typename M>
inline void static_assert_static_matrix_1x1(const static_matrix<M>&) {
	static_assert_static_matrix_1x1<M>();
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator==(const smatrix<TL, RowsL, ColsL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_smatrix_same_shape<RowsL, ColsL, RowsR, ColsR>();
	return equal_to(lhs, rhs);
}

template <typename M>
inline
bool operator==(const static_matrix<M>& lhs, const typename M::element_type& rhs) {
	static_assert_static_matrix_1x1<M>();
	return element_at(lhs, 0, 0) == rhs;
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
	static_assert_static_matrix_1x1(lhs);
	static_assert_static_matrix_1x1(rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename T, unsigned RowsL, unsigned ColsL>
inline
bool operator<(const smatrix<T, RowsL, ColsL>& lhs, const T& rhs) {
	static_assert_static_matrix_1x1(lhs);
	return lhs.element_at(0, 0) < rhs;
}

template <typename T, unsigned Rows, unsigned Cols>
inline
bool operator<(const T& lhs, const smatrix<T, Rows, Cols>& rhs) {
	static_assert_static_matrix_1x1(rhs);
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
