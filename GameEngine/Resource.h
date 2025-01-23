#pragma once

#ifndef RESOURCE_H
#define RESOURCE_H

#include "Serializable.h"

class Resource :public Serializable
{
public:
	// Constructor/ Destructor
	Resource();
	virtual ~Resource();

	//Methods
	virtual void Serialize(std::ostream& _stream);
	virtual void Deserialize(std::istream& _streamm);
	void AssignNonDefaultValues();
	void ToString();

private:
	int m_val1;
	double m_val2;
	char m_val3;
};

#endif // RESOURCE_H
