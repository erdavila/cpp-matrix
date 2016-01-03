#ifndef BASE_HPP_
#define BASE_HPP_


#include <type_traits>
#include <utility>


namespace matrix {


namespace __impl {


template <typename F, typename T>
struct with_qualifiers_of {
	using _noref_F = typename std::remove_reference<F>::type;
	using _noref_T = typename std::remove_reference<T>::type;
	using _T = typename std::remove_const<_noref_T>::type;

	using _cT = typename std::conditional<
			std::is_const<_noref_F>::value,
			typename std::add_const<_T>::type,
			_T
		>::type;

	using _lrT = typename std::conditional<
			std::is_lvalue_reference<F>::value,
			typename std::add_lvalue_reference<_cT>::type,
			_cT
		>::type;
	using _rT = typename std::conditional<
			std::is_rvalue_reference<F>::value,
			typename std::add_rvalue_reference<_lrT>::type,
			_lrT
		>::type;

	using type = _rT;
};


template <typename F, typename T>
typename with_qualifiers_of<F, T>::type&&
forward_with_qualifers_of(T&& value) {
	return static_cast<typename with_qualifiers_of<F, T>::type&&>(value);
}


} /* namespace __impl */


template <typename M>
class matrix {};


template <typename M>
inline
M& concrete_matrix(matrix<M>& m) {
	return static_cast<M&>(m);
}

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
typename M::element_type&
element_at(matrix<M>& m, unsigned row, unsigned col) {
	return concrete_matrix(m).element_at(row, col);
}

template <typename M>
inline
const typename M::element_type&
element_at(const matrix<M>& m, unsigned row, unsigned col) {
	return concrete_matrix(m).element_at(row, col);
}


template <typename ML, typename MR>
bool equal_to(const matrix<ML>& lhs, const matrix<MR>& rhs) {
	for(unsigned row = 0; row < rows(lhs); ++row) {
		for(unsigned col = 0; col < cols(lhs); ++col) {
			if(element_at(lhs, row, col) != element_at(rhs, row, col)) {
				return false;
			}
		}
	}
	return true;
}


template <typename F, typename M, typename... MM>
void for_each_element(F func, M&& m, MM&&... mm) {
	for(unsigned row = 0; row < rows(m); ++row) {
		for(unsigned col = 0; col < cols(m); ++col) {
			func(
				__impl::forward_with_qualifers_of<M >(element_at(m , row, col)),
				__impl::forward_with_qualifers_of<MM>(element_at(mm, row, col))...
			);
		}
	}
}


template <typename MT, typename MF>
void move_to(matrix<MT>& to, matrix<MF>&& from) {
	using element_type_to   = typename MT::element_type;
	using element_type_from = typename MF::element_type;
	auto move_element = [](element_type_to& to, element_type_from&& from) {
		to = std::move(from);
	};
	for_each_element(move_element, to, std::move(from));
}


enum class all_t { all };
constexpr const all_t& all = all_t::all;


} /* namespace matrix */


#endif /* BASE_HPP_ */
