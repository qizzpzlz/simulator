#pragma once
#include <ostream>

namespace Utils
{
	template<typename T>
	struct enum_strings
	{
		static char const* data[];
	};

	template<typename T>
	struct enum_ref_holder
	{
		T& enumVal;
		enum_ref_holder(T& enumVal) : enumVal(enumVal) {}
	};

	template<typename T>
	struct enum_const_ref_holder
	{
		T const& enumVal;
		enum_const_ref_holder(T const& enumVal) : enumVal(enumVal) {}
	};

	template<typename T>
	std::ostream& operator<<(std::ostream& str, enum_const_ref_holder<T> const& data)
	{
		return str << enum_strings<T>::data[static_cast<int>(data.enumVal)];
	}

	template<typename T>
	std::istream& operator>>(std::istream& str, enum_ref_holder<T> const& data)
	{
		std::string value;
		str >> value;

		static auto begin = std::begin(enum_strings<T>::data);
		static auto end = std::end(enum_strings<T>::data);

		auto find = std::find(begin, end, value);
		if (find != end)
		{
			data.enumVal = static_cast<T>(std::distance(begin, find));
		}
		return str;
	}

	template<typename T>
	enum_const_ref_holder<T> enum_to_string(T const& e) { return enum_const_ref_holder<T>(e); }

	template<typename T>
	enum_ref_holder<T> enum_from_string(T& e) { return enum_ref_holder<T>(e); }
}

