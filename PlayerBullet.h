#pragma once
#include "GameObject.h"
class PlayerBullet :
	public GameObject
{
public:
	using GameObject::GameObject;

	void Update() override;

	inline const DirectX::XMFLOAT3& GetVel() { return vel; }
	inline void SetVel(const DirectX::XMFLOAT3& vel) { this->vel = vel; }

	// 弾の消える時間
	UINT life = 60 * 2;

	UINT numFrame = 0;

private:
	DirectX::XMFLOAT3 vel;
};

