#include "Player.h"

const string Player::TAG_PLAYER_INDEX = "PLAYER_INDEX";
const string Player::TAG_LABEL_MATRIX = "LABEL_MATRIX";
const string Player::TAG_NORMAL_MATRIX = "NORMAL_MATRIX";

void Player::Initialize(void)
{
	m_index = 0;
	m_storage = Storage();
	m_skeleton = Skeleton();
	m_labelMatrix = Mat();
	m_normalMatrix = Mat();
}

bool Player::IsValid(void) const
{
	return m_storage.IsValid();
}

bool Player::Initialize(
	const Storage& storage,
	const Mat& indexFrame,
	const int index,
	const NUI_SKELETON_DATA& skeletonData)
{
	if(!storage.IsValid() || indexFrame.empty()) return false;
	if(index <= 0 || index > NUI_SKELETON_COUNT) return false;

	m_index = index;
	m_storage.Update(storage, indexFrame == index);
	m_skeleton.Initialize(index, skeletonData);
	return true;
}

bool Player::Segment(void)
{
	if(m_labelMatrix.empty())
	{
		Segmentation::ComputeLabel(
			m_skeleton,
			m_storage.GetSkeleton(),
			m_labelMatrix);
		return !m_labelMatrix.empty();
	}
	else return true;
}

bool Player::Normal(void)
{
	if(m_normalMatrix.empty())
	{
		m_normalMatrix = NormalVector::ComputeNormalVector(m_storage.GetSkeleton());
		return !m_normalMatrix.empty();
	}
	else return true;
}

bool Player::GraphCut(void)
{
	m_labelMatrix = GraphCutter::Run(
		m_skeleton,
		m_labelMatrix,
		m_storage.GetSkeleton(),
		m_normalMatrix);
	return true;
}

bool Player::Transform(const Player& refPlayer, Mapper& mapper)
{
	const Skeleton& newSkeleton = GetSkeleton();
	const Skeleton& refSkeleton = refPlayer.GetSkeleton();
	Transformation transformation(refSkeleton, newSkeleton);
	
	const Mat transformed = transformation.TransformSkeletonFrame(
		GetStorage().GetSkeleton(), GetLabel());
	if(transformed.empty()) return false;
	m_storage.UpdateSkeleton(transformed);
	cout << "transformed" << endl;
	return true;
}

void Player::UpdateColorFrame(const Mat& colorFrame)
{
	m_storage.UpdateColor(colorFrame);
}

bool Player::Save(const string& path) const
{
	FileStorage fs;
	bool isSaved = true;
	isSaved &= fs.open(path, FileStorage::WRITE);
	if(isSaved) fs << TAG_PLAYER_INDEX << m_index;
	if(isSaved) isSaved &= m_storage.SaveStorage(fs);
	if(isSaved) isSaved &= m_skeleton.Save(fs);
	if(isSaved && !m_labelMatrix.empty()) isSaved &= StorageHandler::Save(fs, m_labelMatrix, TAG_LABEL_MATRIX);
	if(isSaved && !m_normalMatrix.empty()) isSaved &= StorageHandler::Save(fs, m_normalMatrix, TAG_NORMAL_MATRIX);
	if(fs.isOpened()) fs.release();
	return isSaved;
}

bool Player::Load(const string& path)
{
	FileStorage fs;
	bool isLoaded = true;
	if(path.find(".yml") == string::npos) return false;
	if(path.find("player") == string::npos) return false;
	isLoaded &= fs.open(path, FileStorage::READ);
	if(isLoaded) fs[TAG_PLAYER_INDEX] >> m_index;
	if(isLoaded) isLoaded &= m_storage.LoadStorage(fs);
	if(isLoaded) isLoaded &= m_skeleton.Load(fs);
	if(isLoaded) StorageHandler::Load(fs, m_labelMatrix, TAG_LABEL_MATRIX);
	if(isLoaded) StorageHandler::Load(fs, m_normalMatrix, TAG_NORMAL_MATRIX);
	if(fs.isOpened()) fs.release();
	return isLoaded;
}