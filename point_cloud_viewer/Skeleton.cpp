#include "Skeleton.h"

const int Skeleton::parent_indices[N_JOINT] =
	{0, 0, 1, 2, 2, 4, 5, 6, 2, 8, 9, 10, 0, 12, 13, 14, 0, 16, 17, 18};

const string Skeleton::TAG_SKELETON_JOINTS = "SKELETON_JOINTS";
const string Skeleton::TAG_POINT_JOINTS = "POINT_JOINTS";
const string Skeleton::TAG_ABSOLUTE_MATRICES = "ABSOLUTE_MATRICES";
const string Skeleton::TAG_HIERARCHICAL_MATRICES = "HIERARCHICAL_MATRICES";
const string Skeleton::TAG_JOINT_INDICES = "JOINT_INDICES";
const string Skeleton::TAG_SKELETON_DATA = "SKELETON_DATA";

const string Skeleton::TAG_USER_INDEX = "USER_INDEX";
const string Skeleton::TAG_TRACKING_ID = "TRACKING_ID";
const string Skeleton::TAG_QUALITY_FLAGS = "QUALITY_FLAGS";
const string Skeleton::TAG_POSITION = "POSITION";
const string Skeleton::TAG_STATE = "STATE";
const string Skeleton::TAG_SKELETON_POSITION = "SKELETON_POSITION";
const string Skeleton::TAG_SKELETON_STATE = "SKELETON_STATE";
const string Skeleton::TAG_SKELETON_POINTS = "SKELETON_POINTS";

//Part::Part(const Skeleton& skeleton,
//		   const int partIndex,
//		   const Mat& pointMatrix,
//		   const Mat& labelMatrix)
//{
//	// TODO:
//	// set indices
//	const int jointIndex = partIndex + 1;
//	const int parentIndex = Skeleton::GetParentIndex(jointIndex);
//
//	// set validity, matrix and joints from skeleton
//	bool valid1 = skeleton.m_skeletonData.eSkeletonPositionTrackingState[parentIndex];
//	bool valid2 = skeleton.m_skeletonData.eSkeletonPositionTrackingState[jointIndex];
//	m_valid = valid1 && valid2;
//
//	if(m_valid == false) return ;
//	m_rotation = skeleton.m_absoluteMatrices[jointIndex];
//	m_startJoint = skeleton.m_skeletonJoints[parentIndex];
//	m_endJoint = skeleton.m_skeletonJoints[jointIndex];
//
//	// set points from point and label matrices
//	const Mat mask = labelMatrix == partIndex;
//
//}

void Skeleton::Initialize(void)
{
	m_skeletonJoints.clear();
	m_pointJoints.clear();
	m_absoluteMatrices.clear();
	m_hierarchicalMatrices.clear();
	m_skeletonData = NUI_SKELETON_DATA();
}

bool Skeleton::IsValid(void) const
{
	if(m_skeletonData.eTrackingState == NUI_SKELETON_NOT_TRACKED) return false;

	const int size = NUI_SKELETON_POSITION_COUNT;
	const int size1 = m_skeletonJoints.size();
	const int size2 = m_pointJoints.size();
	const int size3 = m_absoluteMatrices.size();
	const int size4 = m_hierarchicalMatrices.size();
	if(size == size1 && size1 == size2 &&
		size2 == size3 && size3 == size4) return true;
	else return false;
}

void Skeleton::Initialize(
	const int index,
	const NUI_SKELETON_DATA& skeletonData,
	INuiCoordinateMapper*& pMapper)
{
	m_skeletonData = skeletonData;
	m_skeletonData.dwUserIndex = index;
	this->SetFromData(pMapper);
}

void Skeleton::SetFromData(INuiCoordinateMapper*& pMapper)
{
	if(m_skeletonData.eTrackingState == NUI_SKELETON_NOT_TRACKED) return ;

	const int nJoint = NUI_SKELETON_POSITION_COUNT;
	m_skeletonJoints.clear();
	m_pointJoints.clear();
	m_absoluteMatrices.clear();
	m_hierarchicalMatrices.clear();

	// copy skeletonJoints
	for(int ii = 0; ii < nJoint; ii++) {
		Vec3f skeletonJoint(0, 0, 0);
		skeletonJoint[0] = m_skeletonData.SkeletonPositions[ii].x;
		skeletonJoint[1] = m_skeletonData.SkeletonPositions[ii].y;
		skeletonJoint[2] = m_skeletonData.SkeletonPositions[ii].z;
		m_skeletonJoints.push_back(skeletonJoint);
	}
	
	// copy pointJoints
	for(int ii = 0; ii < nJoint; ii++)
	{
		Vec3f point = ConvertSkeletonToPoint(m_skeletonData.SkeletonPositions[ii], pMapper);
		m_pointJoints.push_back(point);
	}

	// copy absoluteMatrices and jointIndices
	NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
	NuiSkeletonCalculateBoneOrientations(&m_skeletonData, boneOrientations);
	m_absoluteMatrices.resize(20);
	m_hierarchicalMatrices.resize(20);
	for(int ii = 0; ii < nJoint; ii++)
	{
		const NUI_SKELETON_BONE_ORIENTATION& orientation = boneOrientations[ii];
		const int index = orientation.endJoint;

		m_absoluteMatrices[index] =
			ConvertMatrix4ToMat3x3(orientation.absoluteRotation.rotationMatrix);
		m_hierarchicalMatrices[index] = 
			ConvertMatrix4ToMat3x3(orientation.hierarchicalRotation.rotationMatrix);
	}
}

Mat Skeleton::ConvertMatrix4ToMat3x3(const Matrix4& rotation)
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

Vec3f Skeleton::ConvertSkeletonToPoint(const Vector4& point4, INuiCoordinateMapper*& pMapper)
{
	Vec3f fpoint;
	NUI_DEPTH_IMAGE_POINT dpoint;
	NUI_COLOR_IMAGE_POINT cpoint;
	USHORT dd;

	// convert skeleton to depth
	NuiTransformSkeletonToDepthImage(
		point4,
		&dpoint.x, &dpoint.y, &dd,
		m_depthResolution);
	dd = (dd & 0xfff8) >> 3;
	dpoint.depth = static_cast<LONG>(dd);

	// convert depth to color
	pMapper->MapDepthPointToColorPoint(
		m_depthResolution,
		&dpoint,
		NUI_IMAGE_TYPE_COLOR,
		m_depthResolution,
		&cpoint);
	dpoint.x = cpoint.x;
	dpoint.y = cpoint.y;

	// convert color to point
	fpoint[0] = -(cpoint.x - CX) * dpoint.depth / FX;
	fpoint[1] = -(cpoint.y - CY) * dpoint.depth / FY;
	fpoint[2] = dpoint.depth;

	return fpoint;
}

int Skeleton::GetParentIndex(const int& index)
{
	if(index < 0 || index >= N_JOINT) return -1;
	else return parent_indices[index];
}

bool Skeleton::Save(FileStorage& fs) const
{
	bool valid = true;
	if(valid) valid &= SaveSkeletonJoints(fs);
	if(valid) valid &= SavePointJoints(fs);
	if(valid) valid &= SaveAbsoluteMatrices(fs);
	if(valid) valid &= SaveSkeletonData(fs);
	return valid;
}

bool Skeleton::Load(FileStorage& fs)
{
	bool valid = true;
	if(valid) valid &= LoadSkeletonJoints(fs);
	if(valid) valid &= LoadPointJoints(fs);
	if(valid) valid &= LoadAbsoluteMatrices(fs);
	if(valid) valid &= LoadSkeletonData(fs);
	return valid;
}


bool Skeleton::SaveSkeletonJoints(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		fs << TAG_SKELETON_JOINTS << "[:";
		for(int ii = 0; ii < m_skeletonJoints.size(); ii++)
		{
			const Vec3f& point = m_skeletonJoints[ii];
			fs << static_cast<float>(point[0]);
			fs << static_cast<float>(point[1]);
			fs << static_cast<float>(point[2]);
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

bool Skeleton::LoadSkeletonJoints(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	try
	{
		vector<float> values;
		fs[TAG_SKELETON_JOINTS] >> values;
		if(values.size() % 3 != 0) return false;
		m_skeletonJoints.clear();
		const int count = values.size() / 3;
		for(int ii = 0; ii < count; ii++)
		{
			Vec3f point;
			point[0] = values[3 * ii + 0];
			point[1] = values[3 * ii + 1];
			point[2] = values[3 * ii + 2];
			m_skeletonJoints.push_back(point);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::SavePointJoints(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		fs << TAG_POINT_JOINTS << "[:";
		for(int ii = 0; ii < m_pointJoints.size(); ii++)
		{
			const Vec3f& point = m_pointJoints[ii];
			for(int jj = 0; jj < 3; jj++)
				fs << point[jj];
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


bool Skeleton::LoadPointJoints(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	try
	{
		vector<float> values;
		fs[TAG_POINT_JOINTS] >> values;
		if(values.size() % 3 != 0) return false;
		m_pointJoints.clear();
		const int count = values.size() / 3;
		for(int ii = 0; ii < count; ii++)
		{
			Vec3f point;
			for(int jj = 0; jj < 3; jj++)
				point[jj] = values[3 * ii + jj];
			m_pointJoints.push_back(point);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::SaveAbsoluteMatrices(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		char buffer[20];
		for(int ii = 0; ii < m_absoluteMatrices.size(); ii++)
		{
			sprintf(buffer, "%02d", ii);
			string tag = TAG_ABSOLUTE_MATRICES + buffer;
			fs << tag << m_absoluteMatrices[ii];
		}

		for(int ii = 0; ii < m_hierarchicalMatrices.size(); ii++)
		{
			sprintf(buffer, "%02d", ii);
			string tag = TAG_HIERARCHICAL_MATRICES + buffer;
			fs << tag << m_hierarchicalMatrices[ii];
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::LoadAbsoluteMatrices(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	try
	{
		m_absoluteMatrices.clear();
		char buffer[20];
		for(int ii = 0; ii < NUI_SKELETON_POSITION_COUNT; ii++)
		{
			sprintf(buffer, "%02d", ii);
			string tag = TAG_ABSOLUTE_MATRICES + buffer;
			Mat matrix;
			fs[tag] >> matrix;
			m_absoluteMatrices.push_back(matrix);
		}

		m_hierarchicalMatrices.clear();
		for(int ii = 0; ii < NUI_SKELETON_POSITION_COUNT; ii++)
		{
			sprintf(buffer, "%02d", ii);
			string tag = TAG_HIERARCHICAL_MATRICES + buffer;
			Mat matrix;
			fs[tag] >> matrix;
			m_hierarchicalMatrices.push_back(matrix);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::SaveSkeletonData(FileStorage& fs) const
{
	if(!fs.isOpened()) return false;
	try
	{
		// save meta info
		fs << TAG_USER_INDEX << static_cast<unsigned char>(m_skeletonData.dwUserIndex);
		fs << TAG_TRACKING_ID << static_cast<unsigned char>(m_skeletonData.dwTrackingID);
		fs << TAG_QUALITY_FLAGS << static_cast<unsigned char>(m_skeletonData.dwQualityFlags);
	
		// save position and state
		fs << TAG_POSITION << "[:";
		fs << static_cast<float>(m_skeletonData.Position.x);
		fs << static_cast<float>(m_skeletonData.Position.y);
		fs << static_cast<float>(m_skeletonData.Position.z);
		fs << "]";

		fs << TAG_STATE << static_cast<unsigned char>(m_skeletonData.eTrackingState);
	
		// save skeleton positions and states
		fs << TAG_SKELETON_POSITION << "[:";
		for(int ii = 0; ii < NUI_SKELETON_POSITION_COUNT; ii++)
		{
			Vector4 skeletonPosition = m_skeletonData.SkeletonPositions[ii];
			fs << static_cast<float>(skeletonPosition.x);
			fs << static_cast<float>(skeletonPosition.y);
			fs << static_cast<float>(skeletonPosition.z);
		}
		fs << "]";

		fs << TAG_SKELETON_STATE << "[:";
		for(int ii = 0; ii < NUI_SKELETON_POSITION_COUNT; ii++)
			fs << static_cast<unsigned char>(m_skeletonData.eSkeletonPositionTrackingState[ii]);
		fs << "]";
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool Skeleton::LoadSkeletonData(FileStorage& fs)
{
	if(!fs.isOpened()) return false;
	try
	{
		m_skeletonData = NUI_SKELETON_DATA();

		// load meta info
		unsigned char _userIndex;
		unsigned char _trackingID;
		unsigned char _qualityFlags;
		fs[TAG_USER_INDEX] >> _userIndex;
		fs[TAG_TRACKING_ID] >> _trackingID;
		fs[TAG_QUALITY_FLAGS] >> _qualityFlags;
		m_skeletonData.dwUserIndex = static_cast<DWORD>(_userIndex);
		m_skeletonData.dwTrackingID = static_cast<DWORD>(_trackingID);
		m_skeletonData.dwQualityFlags = static_cast<DWORD>(_qualityFlags);

		// load position
		vector<float> _position;
		fs[TAG_POSITION] >> _position;
		if(_position.size() != 3) throw exception("invalid position");
		m_skeletonData.Position.x = _position[0];
		m_skeletonData.Position.y = _position[1];
		m_skeletonData.Position.z = _position[2];

		// load state
		unsigned char _state;
		fs[TAG_STATE] >> _state;
		m_skeletonData.eTrackingState = static_cast<NUI_SKELETON_TRACKING_STATE>(_state);

		// load skeleton positions and states
		vector<float> _positions;
		vector<int> _states;
		fs[TAG_SKELETON_POSITION] >> _positions;
		fs[TAG_SKELETON_STATE] >> _states;
		if(_positions.size() != NUI_SKELETON_POSITION_COUNT * 3) throw exception("invalid skeleton positions");
		if(_states.size() != NUI_SKELETON_POSITION_COUNT) throw exception("invalid skeleton states");
		for(int ii = 0; ii < NUI_SKELETON_POSITION_COUNT; ii++)
		{
			m_skeletonData.SkeletonPositions[ii].x = _positions[ii * 3 + 0];
			m_skeletonData.SkeletonPositions[ii].y = _positions[ii * 3 + 1];
			m_skeletonData.SkeletonPositions[ii].z = _positions[ii * 3 + 2];
			m_skeletonData.eSkeletonPositionTrackingState[ii] =
				static_cast<NUI_SKELETON_POSITION_TRACKING_STATE>(_states[ii]);
		}
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		return false;
	}
	return true;
}