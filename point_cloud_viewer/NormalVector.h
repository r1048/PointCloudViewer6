#pragma once

#include "Resolution.h"
#include "Player.h"
#include "Mapper.h"
#include "Segmentation.h"

class NormalVector : public Resolution
{
public:
	NormalVector(void);
	~NormalVector(void);

	void Initialize(void);
	bool IsValid(void) const;

	static Mat ComputeNormalVector(const Mat& pointMatrix);
};