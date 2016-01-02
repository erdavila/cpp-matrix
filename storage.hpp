#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>


namespace matrix {


class storage_verifier {
public:
	class exception : public std::logic_error {
		using std::logic_error::logic_error;
	};

protected:
	void set_constructed(bool constructed) noexcept {
		this->constructed = constructed;
	}

	void verify_constructed(bool expected) const {
		if(constructed != expected) {
			const char* msg = expected
			                ? "The object was expected to be constructed"
			                : "The object was expected to be not constructed";
            throw exception(msg);
		}
	}

private:
	bool constructed = false;
};


class null_storage_verifier {
protected:
	void set_constructed(bool) noexcept {}
	void verify_constructed(bool) const {}
};


template <typename T, bool Verified = false>
class storage : private std::conditional<Verified, storage_verifier, null_storage_verifier>::type {
public:
	using value_type = T;

	storage() = default;

	storage(const storage&);

	storage(storage&&);

	~storage() {
		verify_constructed(false);
	}

	storage& operator=(const storage&) &;

	storage& operator=(storage&&) &;

	template <typename... Args>
	void construct_value(Args&&... args) {
		verify_constructed(false);
		new(&store) T(std::forward<Args>(args)...);
		set_constructed(true);
	}

	void destruct_value() {
		verify_constructed(true);
		value_reference().~T();
		set_constructed(false);
	}

	T& value_reference() {
		return *value_pointer();
	}

	const T& value_reference() const {
		return *value_pointer();
	}

private:
	using verifier_type = typename std::conditional<Verified, storage_verifier, null_storage_verifier>::type;
	using verifier_type::verify_constructed;
	using verifier_type::set_constructed;

	using store_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
	store_type store;

	T* value_pointer() {
		verify_constructed(true);
		void* p = &store;
		return static_cast<T*>(p);
	}

	const T* value_pointer() const {
		verify_constructed(true);
		const void* p = &store;
		return static_cast<const T*>(p);
	}
};


static_assert(sizeof(storage<int, false>) == sizeof(int), "");
static_assert(sizeof(storage<int, true> ) >  sizeof(int), "");


} /* namespace matrix */


#endif /* STORAGE_HPP_ */
