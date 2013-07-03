#include "StorageHandler.h"

// save: int
bool StorageHandler::Save(FileStorage& fs, const int& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	fs << data;
	return true;
}

// save: bool
bool StorageHandler::Save(FileStorage& fs, const bool& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	if(!tag.empty()) fs << tag;
	fs << data;
	return true;
}

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
	if(!tag.empty()) {
		fs << tag;
		fs << "[:";
	}
	for(int ii = 0; ii < 3; ii++)
		fs << data[ii];
	if(!tag.empty()) fs << "]";
	return true;
}
//
//// save: Vector4
//bool StorageHandler::Save(FileStorage& fs, const Vector4& data, const string& tag)
//{
//	if(!fs.isOpened()) return false;
//	if(!tag.empty()) {
//		fs << tag;
//		fs << "[:";
//	}
//
//	fs << data.x;
//	fs << data.y;
//	fs << data.z;
//
//	if(!tag.empty())
//		fs << "]";
//	return true;
//}

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

//// save: vector<Vector4>
//bool StorageHandler::Save(FileStorage& fs, const vector<Vector4>& data, const string& tag)
//{
//	if(!fs.isOpened() || data.size() == 0) return false;
//	try
//	{
//		fs << tag << "[:";
//		for(int ii = 0; ii < data.size(); ii++)
//		{
//			Save(fs, data[ii], "");
//		}
//		fs << "]";
//	}
//	catch(exception e)
//	{
//		cerr << e.what() << endl;
//		return false;
//	}
//	return true;
//}


// load: int
bool StorageHandler::Load(FileStorage& fs, int& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	fs[tag] >> data;
	return true;
}

// load: bool
bool StorageHandler::Load(FileStorage& fs, bool& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	unsigned char _data;
	fs[tag] >> _data;
	data = static_cast<bool>(_data);
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

// load: vec3f
bool StorageHandler::Load(FileStorage& fs, Vec3f& data, const string& tag)
{
	if(!fs.isOpened()) return false;
	vector<float> _data;
	fs[tag] >> _data;
	for(int ii = 0; ii < 3; ii++)
		data[ii] = _data[ii];
	return true;
}
//
//// load: Vector4
//bool StorageHandler::Load(FileStorage& fs, Vector4& data, const string& tag)
//{
//	if(!fs.isOpened()) return false;
//	vector<float> _data;
//	fs[tag] >> _data;
//	data.x = _data[0];
//	data.y = _data[1];
//	data.z = _data[2];
//	return true;
//}

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

Vec3f StorageHandler::Vector4_to_Vec3f(const Vector4& data)
{
	Vec3f _data(0, 0, 0);
	_data[0] = data.x;
	_data[1] = data.y;
	_data[2] = data.z;
	return _data;
}

Vector4 StorageHandler::Vec3f_to_Vector4(const Vec3f& data)
{
	Vector4 _data;
	_data.x = data[0];
	_data.y = data[1];
	_data.z = data[2];
	_data.w = 1.0f;
	return _data;
}

Mat StorageHandler::Matrix4_to_Mat3x3(const Matrix4& rotation)
{
	Mat rotationMat = Mat();
	if(rotation.M14 == 0 && rotation.M24 == 0 && rotation.M34 == 0 &&
		rotation.M41 == 0 && rotation.M42 == 0 && rotation.M43 == 0 &&
		rotation.M44 == 1)
	{
		rotationMat = Mat::zeros(3, 3, CV_32FC1);
		rotationMat.at<float>(0, 0) = rotation.M11;
		rotationMat.at<float>(0, 1) = rotation.M12;
		rotationMat.at<float>(0, 2) = rotation.M13;
		
		rotationMat.at<float>(1, 0) = rotation.M21;
		rotationMat.at<float>(1, 1) = rotation.M22;
		rotationMat.at<float>(1, 2) = rotation.M23;

		rotationMat.at<float>(2, 0) = rotation.M31;
		rotationMat.at<float>(2, 1) = rotation.M32;
		rotationMat.at<float>(2, 2) = rotation.M33;
	}
	return rotationMat;
}

Mat StorageHandler::Multiply_Mat_from_Mat3x3_by_Vec3f(const Mat& Mat3x3, const Vec3f& vec)
{
	Mat result = Mat::zeros(3, 1, CV_32FC1);
	if(vec == Vec3f(0, 0, 0)) return result;
	Mat mvec = Mat::zeros(3, 1, CV_32FC1);
	for(int rr = 0; rr < 3; rr++)
		mvec.at<float>(rr, 0) = vec[rr];
	result = Mat3x3 * mvec;
	return result;
}

Vec3f StorageHandler::Multiply_Vec3f_from_Mat3x3_by_Vec3f(const Mat& Mat3x3, const Vec3f& vec)
{
	Mat result = Multiply_Mat_from_Mat3x3_by_Vec3f(Mat3x3, vec);
	Vec3f vresult;
	for(int rr = 0; rr < 3; rr++)
		vresult[rr] = result.at<float>(rr, 0);
	return vresult;
}

Mat StorageHandler::Multiply_Mat_from_Mat4x4_by_Vec3f(const Mat& Mat4x4, const Vec3f& vec)
{
	Mat result = Mat::zeros(4, 1, CV_32FC1);
	if(vec == Vec3f(0, 0, 0)) return result;
	Mat mvec = Mat::zeros(4, 1, CV_32FC1);
	for(int rr = 0; rr < 3; rr++)
		mvec.at<float>(rr, 0) = vec[rr];
	mvec.at<float>(3, 0) = 1.0f;
	result = Mat4x4 * mvec;
	return result;
}

Vec3f StorageHandler::Multiply_Vec3f_from_Mat4x4_by_Vec3f(const Mat& Mat4x4, const Vec3f& vec)
{
	Mat result = Multiply_Mat_from_Mat4x4_by_Vec3f(Mat4x4, vec);
	Vec3f vresult;
	for(int rr = 0; rr < 3; rr++)
		vresult[rr] = result.at<float>(rr, 0);
	return vresult;
}