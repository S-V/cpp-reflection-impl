#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Object/FlagsType.h>

UINT mxFlagsType::GetItemIndexByName( const char* name ) const
{
	for( UINT i = 0; i < m_numFlags; i++ )
	{
		const Member& item = m_members[ i ];
		if( !strcmp( item.name, name ) ) {
			return i;
		}
	}
	ptERROR("Unknown flag: %s", name);
	mxDBG_UNREACHABLE;
	return -1;
}

mxFlagsType::Mask mxFlagsType::GetItemValueByName( const char* name ) const
{
	const UINT itemIndex = this->GetItemIndexByName( name );
	chkRET_NIL_IF_NOT(itemIndex != INDEX_NONE);
	return m_members[ itemIndex ].mask;
}
