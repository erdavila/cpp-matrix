#ifndef SMATRIX_HPP_
#define SMATRIX_HPP_


namespace matrix {


template <typename T, unsigned Rows, unsigned Cols>
class smatrix {
public:
	using element_type = T;

	static constexpr unsigned rows() noexcept { return Rows; }

	static constexpr unsigned cols() noexcept { return Cols; }
};


} /* namespace matrix */


#endif /* SMATRIX_HPP_ */
