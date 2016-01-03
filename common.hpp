#ifndef COMMON_HPP_
#define COMMON_HPP_


namespace matrix {


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator==(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "==", rhs);
	return equal_to(lhs, rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator==(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "==", rhs);
	return equal_to(lhs, rhs);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator!=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "!=", rhs);
	return !equal_to(lhs, rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator!=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	incompatible_operands::throw_if_not_same_shape(lhs, "!=", rhs);
	return !equal_to(lhs, rhs);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator<(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	static_assert_static_matrix_1x1(lhs);
	incompatible_operands::throw_if_not_same_shape(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_static_matrix_1x1(rhs);
	incompatible_operands::throw_if_not_same_shape(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator>(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	static_assert_static_matrix_1x1(lhs);
	incompatible_operands::throw_if_not_same_shape(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_static_matrix_1x1(rhs);
	incompatible_operands::throw_if_not_same_shape(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator<=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	static_assert_static_matrix_1x1(lhs);
	incompatible_operands::throw_if_not_same_shape(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_static_matrix_1x1(rhs);
	incompatible_operands::throw_if_not_same_shape(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator>=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	static_assert_static_matrix_1x1(lhs);
	incompatible_operands::throw_if_not_same_shape(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	static_assert_static_matrix_1x1(rhs);
	incompatible_operands::throw_if_not_same_shape(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}


} /* namespace matrix */


#endif /* COMMON_HPP_ */
