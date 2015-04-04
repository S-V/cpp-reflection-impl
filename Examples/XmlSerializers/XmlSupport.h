#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "XmlSupport.lib" )
#endif //MX_AUTOLINK

#include <Core/ObjectModel.h>

namespace XML
{
	ERet EncodeObject( const void* o, const mxType& type, AStreamWriter &stream );
	ERet DecodeObject( AStreamReader& reader, const mxType& type, void *o );
}//namespace XML

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
