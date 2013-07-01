#include "StorageHandler.h"


// save: float
bool StorageHandler::Save(FileStorage& fs, const float& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	fs << data;
	return true;
}

// save: Mat
bool StorageHandler::Save(FileStorage& fs, const Mat& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	fs << data;
	return true;
}

// save: DWORD
bool StorageHandler::Save(FileStorage& fs, const unsigned long& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	fs << static_cast<unsigned char>(data);
	return true;
}

// save: Vec3f
bool StorageHandler::Save(FileStorage& fs, const Vec3f& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	for(int ii = 0; ii < 3; ii++)
		fs << data[ii];
	return true;
}

// save: Vector4
bool StorageHandler::Save(FileStorage& fs, const Vector4& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) {
		fs << tag;
		fs << "[:";
	}

	fs << data.x;
	fs << data.y;
	fs << data.z;

	if(!tag.empty())
		fs << "]";
	return true;
}

// save: vector<float>
bool StorageHandler::Save(FileStorage& fs, const vector<float>& data, const string& tag)
{
	if(!fs.isOpened() || data.size() == 0) return false;
	try
	{
		fs << tag << "[:";
		for(int ii = 0; ii < data.size(); ii++)
		{
			Save(fs, data[ii], "");
		}
		fs << "]";
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}


// save: vector<Mat>
bool StorageHandler::Save(FileStorage& fs, const vector<Mat>& data, const string& tag)
{
	if(!fs.isOpened() || data.size() == 0) return false;
	try
	{
		char buffer[MAX_PATH];
		for(int ii = 0; ii < data.size(); ii++)
		{
			sprintf(buffer, "%02d", ii);
			string _tag = tag + buffer;
			Save(fs, data[ii], _tag);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// save: vector<Vec3f>
bool StorageHandler::Save(FileStorage& fs, const vector<Vec3f>& data, const string& tag)
{
	if(!fs.isOpened() || data.size() == 0) return false;
	try
	{
		fs << tag << "[:";
		for(int ii = 0; ii < data.size(); ii++)
		{
			Save(fs, data[ii], "");
		}
		fs << "]";
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// save: vector<Vector4>
bool StorageHandler::Save(FileStorage& fs, const vector<Vector4>& data, const string& tag)
{
	if(!fs.isOpened() || data.size() == 0) return false;
	try
	{
		fs << tag << "[:";
		for(int ii = 0; ii < data.size(); ii++)
		{
			Save(fs, data[ii], "");
		}
		fs << "]";
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// load: float
bool StorageHandler::Load(FileStorage& fs, float& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	fs[tag] >> data;
	return true;
}

// load: Mat
bool StorageHandler::Load(FileStorage& fs, Mat& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	fs[tag] >> data;
	return true;
}
	
// load: DWORD
bool StorageHandler::Load(FileStorage& fs, unsigned long& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	unsigned char _data;
	fs[tag] >> _data;
	data = static_cast<unsigned long>(_data);
	return true;
}

// load: Vector4
bool StorageHandler::Load(FileStorage& fs, Vector4& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	vector<float> _data;
	fs[tag] >> _data;
	data.x = _data[0];
	data.y = _data[1];
	data.z = _data[2];
	return true;
}

// load: vector<float>
bool StorageHandler::Load(FileStorage& fs, vector<float>& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	try
	{
		fs[tag] >> data;
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// load: vector<Mat>
bool StorageHandler::Load(FileStorage& fs, vector<Mat>& data, const string& tag, const int size)
{
	if(!fs.isOpened()) return false;
	try
	{
		data.clear();
		data.resize(size);
		char buffer[MAX_PATH];
		for(int ii = 0; ii < size; ii++)
		{
			sprintf(buffer, "%02d", ii);
			string _tag = tag + buffer;
			Load(fs, data[ii], _tag);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// load: vector<DWORD>
bool StorageHandler::Load(FileStorage& fs, vector<unsigned long>& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	try
	{
		vector<unsigned char> _data;
		fs[tag] >> _data;
		for(int ii = 0; ii < _data.size(); ii++)
			data.push_back(static_cast<unsigned long>(_data[ii]));
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

// load: vector<Vec3f>
bool StorageHandler::Load(FileStorage& fs, vector<Vec3f>& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	try
	{
		vector<float> values;
		fs[tag] >> values;
		if(values.size() % 3 != 0) return false;
		const int count = values.size() / 3;
		for(int ii = 0; ii < count; ii++)
		{
			Vec3f point;
			point[0] = values[3 * ii + 0];
			point[1] = values[3 * ii + 1];
			point[2] = values[3 * ii + 2];
			data.push_back(point);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}