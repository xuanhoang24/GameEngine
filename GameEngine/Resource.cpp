#include "Resource.h"

Resource::Resource()
{
	m_val1 = 0;
	m_val2 = 0.0f;
	m_val3 = { };
}

Resource::~Resource()
{
}

void Resource::AssignNonDefaultValues()
{
	m_val1 = 2;
	m_val2 = 2.0f;
	m_val3 = '2';
}

void Resource::ToString()
{
	cout << "m_val1: " << m_val1 << " ";
	cout << "m_val2: " << m_val2 << " ";
	cout << "m_val3: " << m_val3 << " " << endl;
}

void Resource::Serialize(std::ostream& _stream)
{
	_stream.write(reinterpret_cast<char*>(&m_val1), sizeof(m_val1));
	_stream.write(reinterpret_cast<char*>(&m_val2), sizeof(m_val2));
	_stream.write(reinterpret_cast<char*>(&m_val3), sizeof(m_val3));
}

void Resource::Deserialize(std::istream& _stream)
{
	_stream.read(reinterpret_cast<char*>(&m_val1), sizeof(m_val1));
	_stream.read(reinterpret_cast<char*>(&m_val2), sizeof(m_val2));
	_stream.read(reinterpret_cast<char*>(&m_val3), sizeof(m_val3));
}