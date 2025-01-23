#include "ObjectPool.h"
#include "Resource.h"

int main()
{
	ObjectPool<Resource>* objP = new ObjectPool<Resource>();
	Resource* r1 = objP->GetResource();
	r1->AssignNonDefaultValues();

	ofstream writeStream("resource.bin", ios::out | ios::binary);
	r1->Serialize(writeStream);
	writeStream.close();
	cout << "r1 values: ";
	r1->ToString();

	Resource* r2 = objP->GetResource();
	ifstream readStream("resource.bin", ios::in | ios::binary);
	r2->Deserialize(readStream);
	readStream.close();
	cout << "r2 values: ";
	r1->ToString();

	delete objP;
}