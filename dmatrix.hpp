#ifndef DMATRIX_HPP_
#define DMATRIX_HPP_

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <stdexcept>
#include <vector>


namespace matrix {


template <typename T>
class dmatrix {
public:
	using element_type = T;

	dmatrix() = delete;

	dmatrix(const dmatrix&);

	dmatrix(dmatrix&&);

	template <typename U>
	dmatrix(const dmatrix<U>&);

	dmatrix(unsigned rows, unsigned cols)
		: _rows(rows), _cols(cols)
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


class incompatible_operands : std::invalid_argument {
public:
	template <typename TL, typename TR>
	static void throw_if_incompatible(const dmatrix<TL>& lhs, const std::string& operation, const dmatrix<TR>& rhs) {
		if(lhs.rows() != rhs.rows()  ||  lhs.cols() != rhs.cols()) {
			throw incompatible_operands("dmatrix" + dimensions(lhs), operation, "dmatrix" + dimensions(rhs));
		}
	}

	template <typename TL>
	static void throw_if_not_scalar_matrix_at_left(const dmatrix<TL>& lhs, const std::string& operation) {
		if(!is_scalar_matrix(lhs)) {
			throw incompatible_operands("dmatrix" + dimensions(lhs), operation, "scalar");
		}
	}

	template <typename TR>
	static void throw_if_not_scalar_matrix_at_right(const std::string& operation, const dmatrix<TR>& rhs) {
		if(!is_scalar_matrix(rhs)) {
			throw incompatible_operands("scalar", operation, "dmatrix" + dimensions(rhs));
		}
	}

	template <typename TL, typename TR>
	static void throw_if_not_scalar_matrices(const dmatrix<TL>& lhs, const std::string& operation, const dmatrix<TR>& rhs) {
		if(!is_scalar_matrix(lhs)  ||  !is_scalar_matrix(rhs)) {
			throw incompatible_operands("dmatrix" + dimensions(lhs), operation, "dmatrix" + dimensions(rhs));
		}
	}

private:
	incompatible_operands(const std::string& lhs, const std::string& operation, const std::string& rhs)
		: invalid_argument(lhs + ' ' + operation + ' ' + rhs)
	{}

	template <typename T>
	static bool is_scalar_matrix(const dmatrix<T>& m) noexcept {
		return (m.rows() == 1  &&  m.cols() == 1);
	}

	template <typename T>
	static std::string dimensions(const dmatrix<T>& m) {
		return '[' + std::to_string(m.rows()) + 'x' + std::to_string(m.cols()) + ']';
	}
};


template <typename ML, typename MR>
bool equal_to(const ML& lhs, const MR& rhs, unsigned rows, unsigned cols) {
	for(unsigned row = 0; row < rows; ++row) {
		for(unsigned col = 0; col < cols; ++col) {
			if(lhs.element_at(row, col) != rhs.element_at(row, col)) {
				return false;
			}
		}
	}
	return true;
}


template <typename TL, typename TR>
inline
bool operator==(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_incompatible(lhs, "==", rhs);
	return equal_to(lhs, rhs, lhs.rows(), rhs.cols());
}

template <typename T>
inline
bool operator==(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, "==");
	return lhs.element_at(0, 0) == rhs;
}

template <typename T>
inline
bool operator==(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right("==", rhs);
	return lhs == rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator!=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_incompatible(lhs, "!=", rhs);
	return !equal_to(lhs, rhs, lhs.rows(), lhs.cols());
}

template <typename T>
inline
bool operator!=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, "!=");
	return lhs.element_at(0, 0) != rhs;
}

template <typename T>
inline
bool operator!=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right("!=", rhs);
	return lhs != rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator<(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrices(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator<(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, "<");
	return lhs.element_at(0, 0) < rhs;
}

template <typename T>
inline
bool operator<(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right("<", rhs);
	return lhs < rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator>(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrices(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator>(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, ">");
	return lhs.element_at(0, 0) > rhs;
}

template <typename T>
inline
bool operator>(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right(">", rhs);
	return lhs > rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator<=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrices(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator<=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, "<=");
	return lhs.element_at(0, 0) <= rhs;
}

template <typename T>
inline
bool operator<=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right("<=", rhs);
	return lhs <= rhs.element_at(0, 0);
}


template <typename TL, typename TR>
inline
bool operator>=(const dmatrix<TL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrices(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}

template <typename T>
inline
bool operator>=(const dmatrix<T>& lhs, const T& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_left(lhs, ">=");
	return lhs.element_at(0, 0) >= rhs;
}

template <typename T>
inline
bool operator>=(const T& lhs, const dmatrix<T>& rhs) {
	incompatible_operands::throw_if_not_scalar_matrix_at_right(">=", rhs);
	return lhs >= rhs.element_at(0, 0);
}


} /* namespace matrix */


#endif /* DMATRIX_HPP_ */
