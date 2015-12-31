#ifndef DMATRIX_HPP_
#define DMATRIX_HPP_


namespace matrix {


template <typename T>
class dmatrix {
public:
	using element_type = T;

	dmatrix(unsigned rows, unsigned cols)
		: _rows(rows), _cols(cols)
	{}

	unsigned rows() const noexcept { return _rows; };

	unsigned cols() const noexcept { return _cols; };

private:
	unsigned _rows;
	unsigned _cols;
};


} /* namespace matrix */


#endif /* DMATRIX_HPP_ */
