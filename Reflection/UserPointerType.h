/*
=============================================================================
	File:	UserPointerType.h
	Desc:	interface for reflecting custom user-defined data
			(which usually is wildly different in binary/textual forms)
=============================================================================
*/
#pragma once

#include <Base/Object/TypeDescriptor.h>

struct mxUserPointerType : public mxType
{
public:
	inline mxUserPointerType( const Chars& typeName, const STypeDescription& typeInfo )
		: mxType( ETypeKind::Type_UserData, typeName, typeInfo )
	{
	}

	// binary ids are written in place of pointers
	mxSTATIC_ASSERT( sizeof(void*) >= sizeof(UINT32) );

	virtual UINT32 GetPersistentBinaryId( const void* o ) const = 0;
	virtual const char* GetPersistentStringId( const void* o ) const = 0;

	virtual void SetFromBinaryId( void* o, const UINT32 id ) const = 0;
	virtual void SetFromStringId( void* o, const char* id ) const = 0;
};

//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//!=- this macro should be placed in header files
//
#define mxBEGIN_USER_POINTER_TYPE( TYPE )\
	template<>\
	struct TypeDeducer< TYPE >\
	{\
		static inline ETypeKind GetTypeKind()\
		{\
			return ETypeKind::Type_UserData;\
		}\
		static inline const mxType& GetType()\
		{\
			static struct TYPE##Reflector : public mxUserPointerType\
			{\
				inline TYPE##Reflector()\
					: mxUserPointerType\
					(\
						mxEXTRACT_TYPE_NAME(TYPE),\
						STypeDescription::For_Type<TYPE>()\
					)\
				{}\

// the user has only to override the pure virtual functions
//!--------------------------------------------------------------------------


//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//!=- this macro should be placed in header files
//
#define mxEND_USER_POINTER_TYPE\
			} staticTypeInfo();\
			return staticTypeInfo;\
		}\
	};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
