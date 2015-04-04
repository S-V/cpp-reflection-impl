#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/EnumType.h>

UINT mxEnumType::MemberList::GetItemIndexByValue( const UINT nEnumValue ) const
{
	for( UINT i = 0; i < count; i++ )
	{
		const Member& member = items[ i ];
		if( member.value == nEnumValue ) {
			return i;
		}
	}
	return INDEX_NONE;
}

UINT mxEnumType::MemberList::GetItemIndexByString( const char* name ) const
{
	chkRET_X_IF_NIL( name, INDEX_NONE );
	for( UINT i = 0; i < count; i++ )
	{
		const Member& member = items[ i ];
		if( stricmp( member.name, name ) == 0 ) {
			return i;
		}
	}
	ptWARN("no enum member named '%s'\n",name);
	return INDEX_NONE;
}

UINT mxEnumType::GetItemIndexByValue( const UINT nEnumValue ) const
{
	return m_members.GetItemIndexByValue( nEnumValue );
}

UINT mxEnumType::GetItemIndexByString( const char* sEnumValue ) const
{
	return m_members.GetItemIndexByString( sEnumValue );
}

const char* mxEnumType::GetStringByValue( const UINT nEnumValue ) const
{
	const UINT enumItemIndex = this->GetItemIndexByValue( nEnumValue );
	chkRET_X_IF_NOT(enumItemIndex != INDEX_NONE, "INVALID_VALUE");
	return m_members.items[ enumItemIndex ].name;
}

UINT mxEnumType::GetValueByString( const char* sEnumValue ) const
{
	const UINT enumItemIndex = this->GetItemIndexByString( sEnumValue );
	chkRET_X_IF_NOT(enumItemIndex != INDEX_NONE,0);
	return m_members.items[ enumItemIndex ].value;
}
