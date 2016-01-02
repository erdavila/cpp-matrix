#ifndef BASE_HPP_
#define BASE_HPP_


namespace matrix {


template <typename M>
class matrix {};


template <typename M>
inline
M& concrete_matrix(matrix<M>&);

template <typename M>
inline
const M& concrete_matrix(const matrix<M>& m) {
	return static_cast<const M&>(m);
}


template <typename M>
inline
unsigned rows(const matrix<M>& m) {
	return concrete_matrix(m).rows();
}


template <typename M>
inline
unsigned cols(const matrix<M>& m) {
	return concrete_matrix(m).cols();
}


template <typename M>
inline
const typename M::element_type&
element_at(const matrix<M>& m, unsigned row, unsigned col) {
	return concrete_matrix(m).element_at(row, col);
}


template <typename ML, typename MR>
// TODO: replace arguments with matrix<T>
bool equal_to(const ML& lhs, const MR& rhs) {
	for(unsigned row = 0; row < lhs.rows(); ++row) {
		for(unsigned col = 0; col < lhs.cols(); ++col) {
			if(lhs.element_at(row, col) != rhs.element_at(row, col)) {
				return false;
			}
		}
	}
	return true;
}


} /* namespace matrix */


#endif /* BASE_HPP_ */
