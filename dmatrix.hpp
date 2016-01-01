#ifndef DMATRIX_HPP_
#define DMATRIX_HPP_

#include <algorithm>
#include <initializer_list>
#include <iterator>
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

	const T& element_at(unsigned row, unsigned col) const noexcept;

private:
	unsigned _rows;
	unsigned _cols;
	std::vector<T> elements;

	unsigned to_linear_index(unsigned row, unsigned col) noexcept {
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


} /* namespace matrix */


#endif /* DMATRIX_HPP_ */
