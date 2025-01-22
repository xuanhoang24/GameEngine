#include "FileController.h"

int main()
{
	FileController* fc = &FileController::Instance();
	cout << fc->GetCurDirectory() << endl;
	int fs = fc->GetFileSize("FileController.cpp");
	unsigned char* buffer = new unsigned char[fs];
	fc->ReadFileAsync("FileController.cpp", buffer, fs);
	while (!fc->GetFileReadDone())
	{
		cout << "Thread Running..." << std::endl;
	}
	if (fc->GetFileReadSuccess())
	{
		cout << "File size: " << fs << endl;
	}
	else 
	{
		cout << "File read operation unsuccessful." << endl;
	}

	delete[] buffer;
}