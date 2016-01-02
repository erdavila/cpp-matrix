#ifndef BASE_HPP_
#define BASE_HPP_


namespace matrix {


template <typename ML, typename MR>
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
