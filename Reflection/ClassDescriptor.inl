mxFORCEINLINE
bool mxClass::operator == ( const mxClass& other ) const
{
	return ( this == &other );
}

mxFORCEINLINE
bool mxClass::operator != ( const mxClass& other ) const
{
	return ( this != &other );
}

mxFORCEINLINE
const char* mxClass::GetTypeName() const
{
	return m_name.buffer;
}

mxFORCEINLINE
TypeIDArg mxClass::GetTypeID() const
{
	return m_uid;
}

mxFORCEINLINE
const mxClass * mxClass::GetParent() const
{
	return m_base;
}

mxFORCEINLINE
size_t mxClass::GetInstanceSize() const
{
	return m_size;
}

mxFORCEINLINE
bool mxClass::IsAbstract() const
{
	return m_constructor == nil;
}

mxFORCEINLINE
bool mxClass::IsConcrete() const
{
	return m_constructor != nil;
}

mxFORCEINLINE
F_CreateObject * mxClass::GetCreator() const
{
	return m_creator;
}

mxFORCEINLINE
F_ConstructObject *	mxClass::GetConstructor() const
{
	return m_constructor;
}

mxFORCEINLINE
F_DestructObject * mxClass::GetDestructor() const
{
	return m_destructor;
}

mxFORCEINLINE
const mxClassLayout& mxClass::GetLayout() const
{
	return m_members;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
