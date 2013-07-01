#pragma once
#include <fstream>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <NuiApi.h>

using namespace std;
using namespace cv;

namespace StorageHandler
{
	// Save functions
	bool Save(FileStorage& fs, const float& data, const string& tag);
	bool Save(FileStorage& fs, const Mat& data, const string& tag);
	bool Save(FileStorage& fs, const unsigned long& data, const string& tag);
	bool Save(FileStorage& fs, const Vec3f& data, const string& tag);
	bool Save(FileStorage& fs, const Vector4& data, const string& tag);
	bool Save(FileStorage& fs, const vector<float>& data, const string& tag);
	bool Save(FileStorage& fs, const vector<Mat>& data, const string& tag);
	bool Save(FileStorage& fs, const vector<Vec3f>& data, const string& tag);
	bool Save(FileStorage& fs, const vector<Vector4>& data, const string& tag);
	
	// Load functions
	bool Load(FileStorage& fs, float& data, const string& tag);
	bool Load(FileStorage& fs, Mat& data, const string& tag);
	bool Load(FileStorage& fs, unsigned long& data, const string& tag);
	bool Load(FileStorage& fs, Vector4& data, const string& tag);
	bool Load(FileStorage& fs, vector<float>& data, const string& tag);
	bool Load(FileStorage& fs, vector<Mat>& data, const string& tag, const int size);
	bool Load(FileStorage& fs, vector<unsigned long>& data, const string& tag);
	bool Load(FileStorage& fs, vector<Vec3f>& data, const string& tag);
}