#pragma once
#include "..\..\Actor.h"

class CWaterFallTile;

/************************************************
*	��N���X.
**/
class CWaterFall final
	: public CActor
{
public:
	CWaterFall();
	~CWaterFall();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// ��̐ݒ�.
	void Setting();

private:
	// ������.
	void CreateWaterFall();

private:
	std::vector<std::unique_ptr<CWaterFallTile>>	m_WaterFallTileList;	// ��̃^�C�����X�g
	float											m_ImageSize;			// �摜�̃T�C�Y
};