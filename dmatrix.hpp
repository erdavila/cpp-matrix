#ifndef DMATRIX_HPP_
#define DMATRIX_HPP_

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>


namespace matrix {


template <typename M> class dynamic_matrix;
template <typename M> class dmatrix;


class incompatible_operands : public std::invalid_argument {
public:
	template <typename ML, typename MR>
	static void throw_if_not_same_shape(const matrix<ML>& lhs, const std::string& operation, const matrix<MR>& rhs) {
		if(rows(lhs) != rows(rhs)  ||  cols(lhs) != cols(rhs)) {
			throw incompatible_operands(lhs, operation, rhs);
		}
	}

	template <typename ML>
	static void throw_if_not_scalar_dynamic_matrix_at_left(const dynamic_matrix<ML>& lhs, const std::string& operation) {
		if(!is_scalar_dynamic_matrix(lhs)) {
			throw incompatible_operands(lhs, operation, "scalar");
		}
	}

	template <typename MR>
	static void throw_if_not_scalar_dynamic_matrix_at_right(const std::string& operation, const dynamic_matrix<MR>& rhs) {
		if(!is_scalar_dynamic_matrix(rhs)) {
			throw incompatible_operands("scalar", operation, rhs);
		}
	}

	template <typename TL, typename TR>
	static void throw_if_not_scalar_dmatrices(const dmatrix<TL>& lhs, const std::string& operation, const dmatrix<TR>& rhs) {
		if(!is_scalar_dynamic_matrix(lhs)  ||  !is_scalar_dynamic_matrix(rhs)) {
			throw incompatible_operands(lhs, operation, rhs);
		}
	}

	template <typename TL, typename TR>
	incompatible_operands(const TL& lhs, const std::string& operation, const TR& rhs)
		: invalid_argument(type_string(lhs) + ' ' + operation + ' ' + type_string(rhs))
	{}

private:
	template <typename M>
	static bool is_scalar_dynamic_matrix(const dynamic_matrix<M>& m) noexcept {
		return (rows(m) == 1  &&  cols(m) == 1);
	}

	template <typename M>
	static std::string dimensions(const matrix<M>& m) {
		return '[' + std::to_string(rows(m)) + 'x' + std::to_string(cols(m)) + ']';
	}

	template <typename M>
	static std::string type_string(const matrix<M>& m) {
		return type_string(concrete_matrix(m));
	}

	template <typename M>
	static std::string type_string(const dynamic_matrix<M>& m) {
		return "dynamic_matrix" + dimensions(m);
	}

	template <typename T, unsigned Rows, unsigned Cols>
	static std::string type_string(const smatrix<T, Rows, Cols>& m) {
		return "smatrix" + dimensions(m);
	}

	static std::string type_string(const std::string& s) {
		return s;
	}
};


template <typename M>
class dynamic_matrix : public matrix<M> {};


template <typename DMatrix, typename M>
class dmatrix_region_reference_base : public dynamic_matrix<M> {
public:
	using element_type = typename DMatrix::element_type;
	using referred_matrix_type = typename std::remove_const<DMatrix>::type;

	dmatrix_region_reference_base() = delete;

	dmatrix_region_reference_base(const dmatrix_region_reference_base&);

	dmatrix_region_reference_base(dmatrix_region_reference_base&&);

	dmatrix_region_reference_base(
		DMatrix& dmatrix,
		unsigned rows, unsigned cols,
		unsigned first_row, unsigned first_col
	)
		: dmatrix(dmatrix),
		  _rows(rows), _cols(cols),
		  first_row(first_row), first_col(first_col)
	{}

	~dmatrix_region_reference_base() = default;

	template <typename OtherM>
	dmatrix_region_reference_base& operator=(const dynamic_matrix<OtherM>&);

	template <typename OtherM>
	typename std::enable_if<
			std::is_same<
					typename OtherM::element_type,
					element_type
				>::value,
			dmatrix_region_reference_base
		>::type&
	operator=(dynamic_matrix<OtherM>&& m) {
		incompatible_operands::throw_if_not_same_shape(*this, "=", m);
		move_to(*this, std::move(m));
		return *this;
	}

	dmatrix_region_reference_base& operator=(const element_type&);

	dmatrix_region_reference_base& operator=(element_type&& value) {
		incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(*this, "=");
		this->element_at(0, 0) = std::move(value);
		return *this;
	}

	unsigned rows() const noexcept { return _rows; };

	unsigned cols() const noexcept { return _cols; };

	element_type& element_at(unsigned row, unsigned col) {
		return dmatrix.element_at(first_row + row, first_col + col);
	}

	const element_type& element_at(unsigned row, unsigned col) const {
		return dmatrix.element_at(first_row + row, first_col + col);
	}

	operator element_type&() {
		incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right("=", *this);
		return this->element_at(0, 0);
	}

	operator const element_type&() const;

protected:
	DMatrix& dmatrix;
	const unsigned _rows;
	const unsigned _cols;
	const unsigned first_row;
	const unsigned first_col;
};


template <typename DMatrix>
class dmatrix_area_reference;


template <typename DMatrix>
class dmatrix_rows_reference
	: public dmatrix_region_reference_base<DMatrix,
	                dmatrix_rows_reference<DMatrix>>
{
private:
	using area_reference = dmatrix_area_reference<DMatrix>;
	using const_area_reference = const dmatrix_area_reference<const DMatrix>;

	using base = dmatrix_region_reference_base<DMatrix,
	                    dmatrix_rows_reference<DMatrix>>;
	using base::base;  // Inherit constructors

public:
	using typename base::element_type;

	template <typename M>
	dmatrix_rows_reference& operator=(const dynamic_matrix<M>&);

	template <typename M>
	dmatrix_rows_reference& operator=(dynamic_matrix<M>&& m) {
		base::operator=(std::move(m));
		return *this;
	}

	dmatrix_rows_reference& operator=(const element_type&);

	dmatrix_rows_reference& operator=(element_type&& value) {
		base::operator=(std::move(value));
		return *this;
	}

	area_reference operator[](unsigned col) {
		return {
			this->dmatrix,
			this->_rows, 1,
			this->first_row, this->first_col + col
		};
	}

	const_area_reference operator[](unsigned col) const;

	area_reference operator[](drange col_range) {
		return {
			this->dmatrix,
			this->_rows, col_range.size,
			this->first_row, this->first_col + col_range.first
		};
	}

	const_area_reference operator[](drange) const;

	area_reference operator[](all_t) {
		return {
			this->dmatrix,
			this->_rows, this->_cols,
			this->first_row, this->first_col
		};
	}

	const_area_reference operator[](all_t) const;
};


template <typename DMatrix>
class dmatrix_area_reference
	: public dmatrix_region_reference_base<DMatrix,
	                dmatrix_area_reference<DMatrix>>
{
private:
	using rows_reference = dmatrix_rows_reference<DMatrix>;
	using const_rows_reference = const dmatrix_rows_reference<const DMatrix>;

	using base = dmatrix_region_reference_base<DMatrix,
	                    dmatrix_area_reference<DMatrix>>;
	using base::base;  // Inherit constructors

public:
	using typename base::element_type;

	template <typename M>
	dmatrix_area_reference& operator=(const dynamic_matrix<M>&);

	template <typename M>
	dmatrix_area_reference& operator=(dynamic_matrix<M>&& m) {
		base::operator=(std::move(m));
		return *this;
	}

	dmatrix_area_reference& operator=(const element_type&);

	dmatrix_area_reference& operator=(element_type&& value) {
		base::operator=(std::move(value));
		return *this;
	}

	rows_reference operator[](unsigned row) {
		return {
			this->dmatrix,
			1, this->_cols,
			this->first_row + row, this->first_col
		};
	}

	const_rows_reference operator[](unsigned row) const;
};


template <typename T>
class dmatrix : public dynamic_matrix<dmatrix<T>> {
private:
	using rows_reference = dmatrix_rows_reference<dmatrix>;
	using const_rows_reference = const dmatrix_rows_reference<const dmatrix>;

public:
	using element_type = T;

	dmatrix() = delete;

	dmatrix(const dmatrix&);

	dmatrix(dmatrix&&);

	template <typename U>
	dmatrix(const dmatrix<U>&);

	dmatrix(unsigned rows, unsigned cols)
		: dmatrix(rows, cols, {})
	{}

	dmatrix(unsigned rows, unsigned cols, std::initializer_list<std::initializer_list<T>> values)
		: _rows(rows), _cols(cols)
	{
		elements.reserve(_rows * _cols);

		unsigned provided_rows = values.size();
		auto row_count = std::min(_rows, provided_rows);

		auto row_ptr = values.begin();
		for(unsigned row = 0; row < row_count; ++row, ++row_ptr) {
			unsigned provided_cols = row_ptr->size();
			unsigned col_count = std::min(_cols, provided_cols);

			std::copy_n(row_ptr->begin(), col_count, std::back_inserter(elements));

			// Fill missing values in the row
			for(unsigned i = col_count; i < _cols; ++i) {
				elements.emplace_back();
			}
		}

		// Fill missing rows
		for(unsigned row = row_count; row < _rows; ++row) {
			for(unsigned col = 0; col < _cols; ++col) {
				elements.emplace_back();
			}
		}
	}

	dmatrix(std::initializer_list<std::initializer_list<T>> values)
		: dmatrix(values.size(), largest_row_size(values), values)
	{}

	~dmatrix() = default;

	unsigned rows() const noexcept { return _rows; };

	unsigned cols() const noexcept { return _cols; };

	dmatrix& operator=(const dmatrix&) &;

	dmatrix& operator=(dmatrix&&) &;

	template <typename U>
	dmatrix& operator=(const dmatrix<U>&) &;

	T& element_at(unsigned row, unsigned col) noexcept {
		unsigned index = to_linear_index(row, col);
		return elements[index];
	}

	const T& element_at(unsigned row, unsigned col) const noexcept {
		unsigned index = to_linear_index(row, col);
		return elements[index];
	}

	rows_reference operator[](unsigned row) {
		return { *this, 1, _cols, row, 0 };
	}

	const_rows_reference operator[](unsigned row) const;

	rows_reference operator[](drange row_range) {
		return { *this, row_range.size, _cols, row_range.first, 0 };
	}

	const_rows_reference operator[](drange row_range) const;

	rows_reference operator[](all_t) {
		return { *this, _rows, _cols, 0, 0 };
	}

	const_rows_reference operator[](all_t) const;

private:
	unsigned _rows;
	unsigned _cols;
	std::vector<T> elements;

	unsigned to_linear_index(unsigned row, unsigned col) const noexcept {
		return row * _cols + col;
	}

	static unsigned largest_row_size(std::initializer_list<std::initializer_list<T>> values) {
		auto compare_size = [](std::initializer_list<T> a, std::initializer_list<T> b) {
			return a.size() < b.size();
		};
		auto largest_row_ptr = std::max_element(values.begin(), values.end(), compare_size);
		return largest_row_ptr->size();
	}
};


template <typename ML, typename MR>
inline
bool operator==(const dynamic_matrix<ML>& lhs, const dynamic_matrix<MR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "==", rhs);
	return equal_to(lhs, rhs);
}

template <typename M>
inline
bool operator==(const dynamic_matrix<M>& lhs, const typename M::element_type& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, "==");
	return element_at(lhs, 0, 0) == rhs;
}

template <typename T>
inline
bool operator==(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right("==", rhs);
	return lhs == rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator!=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "!=", rhs);
	return !equal_to(lhs, rhs);
}

template <typename T>
inline
bool operator!=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, "!=");
	return lhs.element_at(0, 0) != rhs;
}

template <typename T>
inline
bool operator!=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right("!=", rhs);
	return lhs != rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator<(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_dmatrices(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator<(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, "<");
	return lhs.element_at(0, 0) < rhs;
}

template <typename T>
inline
bool operator<(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right("<", rhs);
	return lhs < rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator>(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_dmatrices(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator>(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, ">");
	return lhs.element_at(0, 0) > rhs;
}

template <typename T>
inline
bool operator>(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right(">", rhs);
	return lhs > rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator<=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_dmatrices(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator<=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, "<=");
	return lhs.element_at(0, 0) <= rhs;
}

template <typename T>
inline
bool operator<=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right("<=", rhs);
	return lhs <= rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator>=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_dmatrices(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator>=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_left(lhs, ">=");
	return lhs.element_at(0, 0) >= rhs;
}

template <typename T>
inline
bool operator>=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_dynamic_matrix_at_right(">=", rhs);
	return lhs >= rhs.element_at(0, 0);
}


} /* namespace matrix */


#endif /* DMATRIX_HPP_ */
