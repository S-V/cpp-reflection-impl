#pragma once

#include <Base/Object/Reflection.h>
#include <Base/Object/TypeDescriptor.h>
#include <Base/Object/ClassDescriptor.h>

/*
-----------------------------------------------------------------------------
	mxStruct

	C-style structure
	(i.e. inheritance is not allowed, only member fields are reflected)
-----------------------------------------------------------------------------
*/


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- can only be used on C-like structs
//!=- should be placed into header files
//
#define mxDECLARE_STRUCT( STRUCT )\
	extern mxClassLayout& PP_JOIN_TOKEN( Reflect_Struct_, STRUCT )();\
	template<>\
	struct TypeDeducer< STRUCT >\
	{\
		static inline ETypeKind GetTypeKind()\
		{\
			return ETypeKind::Type_Class;\
		}\
		static inline const mxType& GetType()\
		{\
			return GetClass();\
		}\
		static inline const mxClass& GetClass()\
		{\
			static mxClass staticTypeInfo(\
								mxEXTRACT_TYPE_NAME( STRUCT ),\
								mxEXTRACT_TYPE_GUID( STRUCT ),\
								nil /*parent class*/,\
								SClassDescription::For_Class_With_Default_Ctor< STRUCT >(),\
								PP_JOIN_TOKEN( Reflect_Struct_, STRUCT )()\
							);\
			return staticTypeInfo;\
		}\
	};

//!--------------------------------------------------------------------------




//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- can only be used on C-like structs
//!=- should be placed into source files
//
#define mxBEGIN_STRUCT( STRUCT )\
	mxClassLayout& PP_JOIN_TOKEN( Reflect_Struct_, STRUCT )() {\
		typedef STRUCT OuterType;\
		static mxField fields[] = {\

// use mxMEMBER_FIELD*
// and mxEND_REFLECTION

//!--------------------------------------------------------------------------





//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- can only be used on C-like structs
//!=- which inherit from CStruct class
//!=- should be placed into source files
//
#define mxREFLECT_STRUCT_VIA_STATIC_METHOD( STRUCT )\
	mxClassLayout& PP_JOIN_TOKEN( Reflect_Struct_, STRUCT )() {\
		return STRUCT::StaticLayout();\
	}\

//!--------------------------------------------------------------------------



//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- this macro is placed in struct declarations
//!=- so that reflection can access private and protected members
//
#define mxREFLECTION_IS_MY_FRIEND( STRUCT )\
	friend mxClassLayout& PP_JOIN_TOKEN( Reflect_Struct_, STRUCT )()

//!--------------------------------------------------------------------------



//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//!=- can only be used on C-like structs
//!=- should be placed into source files
//!=- basically allows to reinterpret the struct as another struct
//!=- (e.g.this can be used to alias __m128 as XMFLOAT4)
//
#define mxREFLECT_STRUCT_AS_ANOTHER_STRUCT( THIS_STRUCT, OTHER_STRUCT )\
	mxClassLayout& PP_JOIN_TOKEN( Reflect_Struct_, THIS_STRUCT )() {\
		mxSTATIC_ASSERT( sizeof THIS_STRUCT == sizeof OTHER_STRUCT );\
		return PP_JOIN_TOKEN( Reflect_Struct_, OTHER_STRUCT )();\
	}\
//!--------------------------------------------------------------------------



//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
