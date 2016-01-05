#ifndef COMPAT_HPP_
#define COMPAT_HPP_


#ifdef __CYGWIN__

# include <cstdio>
# include <string>

namespace std {

inline std::string to_string(int value);
inline std::string to_string(long value);
inline std::string to_string(long long value);
inline std::string to_string(unsigned value) {
	char buf[128];
	std::sprintf(buf, "%u", value);
	return buf;
}
inline std::string to_string(unsigned long value);
inline std::string to_string(unsigned long long value);
inline std::string to_string(float value);
inline std::string to_string(double value);
inline std::string to_string(long double value);

}

#endif


#endif /* COMPAT_HPP_ */
