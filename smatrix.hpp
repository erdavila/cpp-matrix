#ifndef SMATRIX_HPP_
#define SMATRIX_HPP_

#include "safely_constructed_array.hpp"
#include <type_traits>
#include <utility>


namespace matrix {


template <typename M>
class static_matrix : public matrix<M> {};


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_region_reference_base {
public:
	using element_type = typename SMatrix::element_type;

	smatrix_region_reference_base() = delete;

	smatrix_region_reference_base(const smatrix_region_reference_base&);

	smatrix_region_reference_base(smatrix_region_reference_base&&);

	smatrix_region_reference_base(SMatrix& smatrix, unsigned first_row, unsigned first_col)
		: smatrix(smatrix), first_row(first_row), first_col(first_col)
	{}

	~smatrix_region_reference_base() = default;

	element_type& element_at(unsigned row, unsigned col) {
		return smatrix.element_at(first_row + row, first_col + col);
	}

	const element_type& element_at(unsigned row, unsigned col) const {
		return smatrix.element_at(first_row + row, first_col + col);
	}

protected:
	SMatrix& smatrix;
	const unsigned first_row;
	const unsigned first_col;
};


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_area_reference;


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_rows_reference : public smatrix_region_reference_base<SMatrix, Rows, Cols> {
private:
	template <unsigned ARows, unsigned ACols>
	using area_reference = smatrix_area_reference<SMatrix, ARows, ACols>;

	template <unsigned ARows, unsigned ACols>
	using const_area_reference = const smatrix_area_reference<const SMatrix, ARows, ACols>;

	using base = smatrix_region_reference_base<SMatrix, Rows, Cols>;
	using base::base;  // Inherit constructors

public:
	using typename base::element_type;

	area_reference<Rows, 1> operator[](unsigned col) {
		return { this->smatrix, this->first_row, this->first_col + col };
	}

	//const_area_reference<Rows, 1> operator[](unsigned col) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_area_reference<const SMatrix, Rows, 1> operator[](unsigned col) const;

	template <unsigned RangeCols>
	area_reference<Rows, RangeCols> operator[](srange<RangeCols> col_range) {
		return { this->smatrix, this->first_row, this->first_col + col_range.first };
	}

	template <unsigned RangeCols>
	//const_area_reference<Rows, RangeCols> operator[](range<RangeCols>) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_area_reference<const SMatrix, Rows, RangeCols> operator[](srange<RangeCols>) const;

	area_reference<Rows, Cols> operator[](all_t) {
		return { this->smatrix, this->first_row, this->first_col };
	}

	//const_area_reference<Rows, Cols> operator[](all_t) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_area_reference<const SMatrix, Rows, Cols> operator[](all_t) const;
};


template <typename SMatrix, unsigned Rows, unsigned Cols>
class smatrix_area_reference
	: public static_matrix<smatrix_area_reference<SMatrix, Rows, Cols>>,
	  public smatrix_region_reference_base<SMatrix, Rows, Cols>
{
private:
	template <unsigned RRows, unsigned RCols>
	using rows_reference = smatrix_rows_reference<SMatrix, RRows, RCols>;

	template <unsigned RRows, unsigned RCols>
	using const_rows_reference = const smatrix_rows_reference<const SMatrix, RRows, RCols>;

	using base = smatrix_region_reference_base<SMatrix, Rows, Cols>;
	using base::base;  // Inherit constructors

public:
	using typename base::element_type;

	static constexpr unsigned rows() noexcept { return Rows; }

	static constexpr unsigned cols() noexcept { return Cols; }

	template <typename M>
	smatrix_area_reference& operator=(const static_matrix<M>& m);

	template <typename M>
	typename std::enable_if<
			std::is_same<
					typename M::element_type,
					element_type
				>::value,
			smatrix_area_reference
		>::type&
	operator=(static_matrix<M>&& m) {
		static_assert(M::rows() == Rows  &&  M::cols() == Cols, "The static_matrix shape is not compatible");
		move_to(*this, std::move(m));
		return *this;
	}

	smatrix_area_reference& operator=(const element_type&) /* {
		static_assert_static_matrix_1x1(*this);
		...
	}*/;

	smatrix_area_reference& operator=(element_type&& value) {
		static_assert_static_matrix_1x1(*this);
		this->element_at(0, 0) = std::move(value);
		return *this;
	}

	rows_reference<1, Cols> operator[](unsigned row) {
		return { this->smatrix, this->first_row + row, this->first_col };
	}

	//const_rows_reference<1, Cols> operator[](unsigned row) const; //  <-- This line makes GCC 4.8.4 crash!
	const smatrix_rows_reference<const SMatrix, 1, Cols> operator[](unsigned row) const;

	operator element_type&() {
		static_assert_static_matrix_1x1(*this);
		return this->element_at(0, 0);
	}

	operator const element_type&() const /*{
		static_assert_static_matrix_1x1(*this);
		...
	}*/;
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


template <typename ML, typename MR>
inline void static_assert_static_matrix_same_shape() {
	static_assert(ML::rows() == MR::rows()  &&  ML::cols() == MR::cols(), "Both static_matrix'es must have the same shape for this operation");
}

template <typename M>
inline void static_assert_static_matrix_1x1() {
	static_assert(M::rows() == 1  &&  M::cols() == 1, "The static_matrix must be 1x1 for this operation");
}

// TODO: remove this function when all operators below use static_matrix arguments instead of smatrix
template <typename M>
inline void static_assert_static_matrix_1x1(const static_matrix<M>&) {
	static_assert_static_matrix_1x1<M>();
}


template <typename ML, typename MR>
inline
bool operator==(const static_matrix<ML>& lhs, const static_matrix<MR>& rhs) {
	static_assert_static_matrix_same_shape<ML, MR>();
	return equal_to(lhs, rhs);
}

template <typename M>
inline
bool operator==(const static_matrix<M>& lhs, const typename M::element_type& rhs) {
	static_assert_static_matrix_1x1<M>();
	return element_at(lhs, 0, 0) == rhs;
}

template <typename M>
inline
bool operator==(const typename M::element_type& lhs, const static_matrix<M>& rhs) {
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
