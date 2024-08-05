#pragma once

// managed by Alexander Ilyin
// original belongs at Discoloured project
// third attempt; copy/paste from Boost trunk on 2011-01-31
// 2012-09-24, fourth attempt; trying to mix Chesnokov's and Watanabe's methods together

// replacement of 'decltype()' for old MSVC compilers
// Igor Chesnokov's method (MSVC 7.1) :  http://rsdn.ru/forum/src/1094305.aspx
// Steven Watanabe's method (MSVC 8.0) :  http://lists.boost.org/Archives/boost/2006/12/115006.php
// can be found with a search engine by tags :  VarTypeID CProvideCounterValue
// MSVC 6.0 method is done from 7.1 one with my correction to 'template<> struct count_of_t', where '__if_exists()' is not supported, but just 'count = 1' works correctly
// *tested on MSVC 6.0, 7.1, 8.0, 9.0, and about others I don't care for the moment

// *it doesn't work on MSVC 5.0 (_MSC_VER = 1100); the 'template<>VarTypeId()' line gives :  fatal error C1001: INTERNAL COMPILER ERROR
#if _MSC_VER < 1200
#error requires MSVC 6.0 at least
#endif

// *it doesn't work on MSVC 7.0 (_MSC_VER = 1300); I have tried the solution of Boost, but didn't succeed with it; this is a seldom compiler, and I don't pay attention at it
#if _MSC_VER == 1300
#error MSVC 2002 (7.0) is not supported
#endif

#if _MSC_VER >= 1600
#error MSVC 2010 (10.0) and above do not need the hack which is contained in this file; they have standard 'decltype()' for that instead
#endif

// no, even GCC 4.7.0 doesn't allow to omit 'typename' if try call 'type_reg_t::f()' or 'type_reg_t<>::f()'
#if 0
template<int id, typename T> struct type_reg_tt
{
	typedef T type_t;
	void f (T arg);
};
#endif

// below MSVC 8.0
#if _MSC_VER < 1400
template<int id> struct type_reg_root_tt
{
	struct id_to_type_t;
};

template<int id, typename T> struct type_reg_tt : type_reg_root_tt<id>
{
	struct type_reg_root_tt<id>::id_to_type_t
	{
		typedef T type_t;
	};
	typedef void dummy_t;
};
// MSVC 8.0, 9.0
// *this isn't cleaned/corrected, and a defines are used after, to make it compatible with other pieces of code
#else
struct msvc_extract_type_default_param {};
template<int ID, typename T = msvc_extract_type_default_param> struct msvc_extract_type;

template<int ID> struct msvc_extract_type<ID, msvc_extract_type_default_param>
{
	template<bool>
	struct id_to_type_impl_t; 

	typedef id_to_type_impl_t<true> id_to_type_t;
};

template<int ID, typename T> struct msvc_extract_type : msvc_extract_type<ID, msvc_extract_type_default_param> 
{ 
// VC8.0 specific bugfeature 
	template<>
	struct id_to_type_impl_t<true>
	{ 
		typedef T type_t;
	}; 
	template<bool>
	struct id_to_type_impl_t; 

	typedef id_to_type_impl_t<true> id_to_type_t;
};

// inplace another offer
#define type_reg_root_tt msvc_extract_type
#define type_reg_tt msvc_extract_type
#define dummy_t id_to_type_t
#endif

template<int N>
class counter_t;

// TUnused is required to force compiler to recompile 'count_of_t' class
template<typename TUnused, int NTested = 0>
struct count_of_t
{
	enum
	{
// '__if_exists' introduced in MSVC 2002 :  http://msdn.microsoft.com/en-us/library/2byy0fh6%28v=vs.80%29.aspx
#if _MSC_VER >= 1300
		__if_exists(counter_t<NTested>) { count = count_of_t<TUnused, NTested + 1>::count }
		__if_not_exists(counter_t<NTested>) { count = NTested }
// while MSVC 6.0 perfectly works with just this (tested on 4 different definitions in a row)
#else
		count = 1
#endif
	};
};

template<class TTypeReg, class TUnused, int NValue>
struct provide_counter_value_t
{
	enum
	{
		value = NValue
	};
};

// MSVC 7.0 and above
#if _MSC_VER >= 1300
#define EXTRA_TYPENAME typename
#else
#define EXTRA_TYPENAME
#endif

// *previously 'type_id()'
#define UNIQUE_TYPE_ID(type) \
	(provide_counter_value_t< \
		/*register TYPE--ID*/ EXTRA_TYPENAME type_reg_tt<count_of_t<type>::count, type>::dummy_t, \
		/*increment compile-time Counter*/ counter_t<count_of_t<type>::count>, \
		/*pass value of Counter*/ count_of_t<type>::count \
	>::value)

// Lets type_id() be > than 0
class __Increment_type_id
{
	enum
	{
		value = UNIQUE_TYPE_ID(__Increment_type_id)
	};
};

// type_id
//#define UNIQUE_TYPE_ID(_expr) type_reg_t<__COUNTER__ + 1>::f(_expr), __COUNTER__
// '__COUNTER__' is introduced in MSVC 2002 :  http://msdn.microsoft.com/en-us/library/2byy0fh6%28v=vs.80%29.aspx
//	int a = __COUNTER__;

// a way to convert a value of template into a real useable number/index, using 'sizeof()'
template<int sz>
struct sized
{
private:
	char m_sizer[sz];
};

// here an expression which has a type, is transformed into something else
// *originally there was 'T&' and not 'T'; idk why
template<typename T>
//typename sized<UNIQUE_TYPE_ID(T)> VarTypeId (T&);
typename sized<UNIQUE_TYPE_ID(T)> VarTypeId (T);

#if 0
template<int NSize>	class sized { char m_pad[NSize]; };
template<typename T> typename sized<UNIQUE_TYPE_ID(T)> VarTypeId(T&);
template<typename T> typename sized<UNIQUE_TYPE_ID(const T)> VarTypeId(const T&);
template<typename T> typename sized<UNIQUE_TYPE_ID(volatile  T)> VarTypeId(volatile T&);
template<typename T> typename sized<UNIQUE_TYPE_ID(const volatile T)> VarTypeId(const volatile T&);

#define typeof(expression) msvc_typeof_impl::msvc_extract_type<sizeof(msvc_typeof_impl::VarTypeId(expression))>::id_to_type_t::type
#endif

#define VAR_TYPE_ID(_expr) (sizeof(VarTypeId(_expr)))

#if 0
// type_of
//#define to(_expr) type_reg_t<UNIQUE_TYPE_ID(_expr)>::type_t
#define to(_expr) type_reg_t<VAR_TYPE_ID(_expr)>::type_t
#undef decltype
#define decltype to
//int a = __COUNTER__;
#endif

#define decltype(_expr) type_reg_root_tt<VAR_TYPE_ID(_expr)>::id_to_type_t::type_t

