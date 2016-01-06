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

	storage() : dummy() {};

	storage(const storage&);

	storage(storage&&);

	~storage() {
		this->verify_constructed(false);
	}

	storage& operator=(const storage&) &;

	storage& operator=(storage&&) &;

	template <typename... Args>
	void construct_value(Args&&... args) {
		this->verify_constructed(false);
		new(&value) T(std::forward<Args>(args)...);
		this->set_constructed(true);
	}

	void destruct_value() {
		this->verify_constructed(true);
		value.~T();
		this->set_constructed(false);
	}

	T& value_reference() {
		this->verify_constructed(true);
		return value;
	}

	const T& value_reference() const {
		this->verify_constructed(true);
		return value;
	}

private:
	union {
		T value;
		char dummy;
	};
};


static_assert(sizeof(storage<int, false>) == sizeof(int), "");
static_assert(sizeof(storage<int, true> ) >  sizeof(int), "");


} /* namespace matrix */


#endif /* STORAGE_HPP_ */
