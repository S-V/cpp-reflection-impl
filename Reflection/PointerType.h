/*
=============================================================================
	File:	PointerType.h
	Desc:	
=============================================================================
*/
#pragma once

#include <Base/Object/TypeDescriptor.h>

struct mxPointerType : public mxType
{
	const mxType &	pointee;	// type of the referenced object

public:
	inline mxPointerType( const Chars& typeName, const STypeDescription& typeInfo, const mxType& pointeeType )
		: mxType( ETypeKind::Type_Pointer, typeName, typeInfo )
		, pointee( pointeeType )
	{
	}
};

template< typename TYPE >
struct TypeDeducer< TYPE* >
{
	static inline const mxType& GetType()
	{
		const mxType& pointeeType = T_DeduceTypeInfo< TYPE >();

		static mxPointerType staticTypeInfo(
			mxEXTRACT_TYPE_NAME(Pointer),
			STypeDescription::For_Type< TYPE* >(),
			pointeeType
		);

		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Pointer;
	}
};

template< typename TYPE >
struct TypeDeducer< TYPE& >
{
	static inline const mxType& GetType()
	{
		const mxType& pointeeType = T_DeduceTypeInfo< TYPE >();

		static mxPointerType staticTypeInfo(
			mxEXTRACT_TYPE_NAME(Pointer),
			STypeDescription::For_Type< TYPE* >(),
			pointeeType
		);

		return staticTypeInfo;
	}
	static inline ETypeKind GetTypeKind()
	{
		return ETypeKind::Type_Pointer;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
