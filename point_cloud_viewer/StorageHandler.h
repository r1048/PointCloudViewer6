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
	bool Save(FileStorage& fs, const bool& data, const string& tag);
	bool Save(FileStorage& fs, const int& data, const string& tag);
	bool Save(FileStorage& fs, const float& data, const string& tag);
	bool Save(FileStorage& fs, const Mat& data, const string& tag);
	bool Save(FileStorage& fs, const unsigned long& data, const string& tag);
	bool Save(FileStorage& fs, const Vec3f& data, const string& tag);
	bool Save(FileStorage& fs, const vector<float>& data, const string& tag);
	bool Save(FileStorage& fs, const vector<Mat>& data, const string& tag);
	bool Save(FileStorage& fs, const vector<Vec3f>& data, const string& tag);
	
	// Load functions
	bool Load(FileStorage& fs, int& data, const string& tag);
	bool Load(FileStorage& fs, bool& data, const string& tag);
	bool Load(FileStorage& fs, float& data, const string& tag);
	bool Load(FileStorage& fs, Mat& data, const string& tag);
	bool Load(FileStorage& fs, unsigned long& data, const string& tag);
	bool Load(FileStorage& fs, Vec3f& data, const string& tag);
	bool Load(FileStorage& fs, vector<float>& data, const string& tag);
	bool Load(FileStorage& fs, vector<Mat>& data, const string& tag, const int size);
	bool Load(FileStorage& fs, vector<unsigned long>& data, const string& tag);
	bool Load(FileStorage& fs, vector<Vec3f>& data, const string& tag);

	// Convert functions
	Vec3f Vector4_to_Vec3f(const Vector4& data);
	Vector4 Vec3f_to_Vector4(const Vec3f& data);
	Mat Matrix4_to_Mat3x3(const Matrix4& rotation);
	
	Mat Multiply_Mat_from_Mat3x3_by_Vec3f(const Mat& Mat3x3, const Vec3f& vec);
	Vec3f Multiply_Vec3f_from_Mat3x3_by_Vec3f(const Mat& Mat3x3, const Vec3f& vec);
	Mat Multiply_Mat_from_Mat4x4_by_Vec3f(const Mat& Mat4x4, const Vec3f& vec);
	Vec3f Multiply_Vec3f_from_Mat4x4_by_Vec3f(const Mat& Mat4x4, const Vec3f& vec);
}