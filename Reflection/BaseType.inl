
/*================================
			AObject
================================*/

mxFORCEINLINE
bool AObject::IsA( const mxClass& type ) const
{
	return this->rttiGetClass().IsDerivedFrom( type );
}

mxFORCEINLINE
bool AObject::IsA( TypeIDArg typeCode ) const
{
	return this->rttiGetClass().IsDerivedFrom( typeCode );
}

mxFORCEINLINE
bool AObject::IsInstanceOf( const mxClass& type ) const
{
	return ( this->rttiGetClass() == type );
}

mxFORCEINLINE
bool AObject::IsInstanceOf( TypeIDArg typeCode ) const
{
	return ( this->rttiGetClass().GetTypeID() == typeCode );
}

mxFORCEINLINE
const char* AObject::rttiGetTypeName() const
{
	return this->rttiGetClass().GetTypeName();
}

mxFORCEINLINE
const TypeID AObject::rttiGetTypeID() const
{
	return this->rttiGetClass().GetTypeID();
}

mxFORCEINLINE
mxClass & AObject::MetaClass()
{
	return ms_staticTypeInfo;
}

mxFORCEINLINE
mxClass & AObject::rttiGetClass() const
{
	return ms_staticTypeInfo;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
