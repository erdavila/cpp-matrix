#ifndef COMMON_HPP_
#define COMMON_HPP_


namespace matrix {


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator==(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "==", rhs);
	return equal_to(lhs, rhs, RowsL, ColsL);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator==(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "==", rhs);
	return equal_to(lhs, rhs, RowsR, ColsR);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator!=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "!=", rhs);
	return !equal_to(lhs, rhs, RowsL, ColsL);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator!=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "!=", rhs);
	return !equal_to(lhs, rhs, RowsR, ColsR);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator<(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	throw incompatible_operands(lhs, "<", rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	throw incompatible_operands(lhs, "<", rhs);
}

template <typename TL, typename TR>
inline
bool operator<(const smatrix<TL, 1, 1>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}

template <typename TL, typename TR>
inline
bool operator<(const dmatrix<TL>& lhs, const smatrix<TR, 1, 1>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "<", rhs);
	return lhs.element_at(0, 0) < rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator>(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	throw incompatible_operands(lhs, ">", rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	throw incompatible_operands(lhs, ">", rhs);
}

template <typename TL, typename TR>
inline
bool operator>(const smatrix<TL, 1, 1>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}

template <typename TL, typename TR>
inline
bool operator>(const dmatrix<TL>& lhs, const smatrix<TR, 1, 1>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, ">", rhs);
	return lhs.element_at(0, 0) > rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator<=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	throw incompatible_operands(lhs, "<=", rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator<=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	throw incompatible_operands(lhs, "<=", rhs);
}

template <typename TL, typename TR>
inline
bool operator<=(const smatrix<TL, 1, 1>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}

template <typename TL, typename TR>
inline
bool operator<=(const dmatrix<TL>& lhs, const smatrix<TR, 1, 1>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, "<=", rhs);
	return lhs.element_at(0, 0) <= rhs.element_at(0, 0);
}


template <typename TL, unsigned RowsL, unsigned ColsL, typename TR>
inline
bool operator>=(const smatrix<TL, RowsL, ColsL>& lhs, const dmatrix<TR>& rhs) {
	throw incompatible_operands(lhs, ">=", rhs);
}

template <typename TL, typename TR, unsigned RowsR, unsigned ColsR>
inline
bool operator>=(const dmatrix<TL>& lhs, const smatrix<TR, RowsR, ColsR>& rhs) {
	throw incompatible_operands(lhs, ">=", rhs);
}

template <typename TL, typename TR>
inline
bool operator>=(const smatrix<TL, 1, 1>& lhs, const dmatrix<TR>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}

template <typename TL, typename TR>
inline
bool operator>=(const dmatrix<TL>& lhs, const smatrix<TR, 1, 1>& rhs) {
	incompatible_operands::throw_if_unmatching_dimensions(lhs, ">=", rhs);
	return lhs.element_at(0, 0) >= rhs.element_at(0, 0);
}


} /* namespace matrix */


#endif /* COMMON_HPP_ */
