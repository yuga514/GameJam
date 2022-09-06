#include "Player.h"
#include "FbxLoader.h"
#include "DirectXCommon.h"
#include "Input.h"

using namespace DirectX;

Player::Player()
	:Player(ObjModel::LoadFromObj("rat"))
{
	obj->SetPosition({ 0,0,0 });
	obj->SetScale({ 1,1,1 });

	aim = ObjObject3d::Create();
	aim->Initialize();
	aim->SetModel(ObjModel::LoadFromObj("sphere"));
	constexpr float aimLength = 10;
	aim->SetPosition({ 0,0,aimLength });
	aim->SetScale({ 0.5,0.5,1 });
	aim->SetParent(obj.get());
}

void Player::Update()
{
	// �ړ�����
	Move();

	{
		XMMATRIX mat =
			obj->GetCamera()->GetViewMatrix() *
			obj->GetCamera()->GetProjectionMatrix() *
			obj->GetCamera()->GetViewPortMatrix();

		XMVECTOR screenAimPos = XMVector3Transform(aim->GetMatWorld().r[3], mat);
		screenAimPos /= XMVectorGetW(screenAimPos);

		// �ϐ��Ɋi�[
		float2ScreenAimPos = XMFLOAT2(XMVectorGetX(screenAimPos), XMVectorGetY(screenAimPos));
	}

	for (auto& i : bullet)
	{
		i.Update();
	}

	bullet.erase(std::remove_if(bullet.begin(), bullet.end(), [](PlayerBullet& i) {return !i.GetAlive(); }), bullet.end());

	obj->Update();
	//aim->Update();
}

void Player::Draw()
{
	for (auto& i : bullet)
	{
		i.Draw();
	}

	if (alive) {
		obj->Draw();
		// aim->Draw();
	}

}

void Player::Shot(ObjModel* model, float scale)
{
	bullet.emplace_back(model, obj->GetPosition());
	bullet.back().SetScale({ scale,scale,scale });
	XMFLOAT3 vel;
	XMStoreFloat3(&vel, XMVector3Transform(XMVectorSet(0, 0, 1, 1), obj->GetMatRot()));


	if (shotTarget != nullptr) {
		// ���x���v�Z
		// ��������W�I�܂ł̃x�N�g��
		vel = {
			shotTarget->GetPos().x - obj->GetPosition().x,
			shotTarget->GetPos().y - obj->GetPosition().y,
			shotTarget->GetPos().z - obj->GetPosition().z
		};
		// XMVECTOR�ɕϊ�
		XMVECTOR vectorVel = XMLoadFloat3(&vel);
		// �傫����1�ɂ���
		vectorVel = XMVector3Normalize(vectorVel);
		// �傫����C�ӂ̒l�ɂ���
		vectorVel = XMVectorScale(vectorVel, 0.3);
		// FLOAT3�ɕϊ�
		XMStoreFloat3(&vel, vectorVel);

		// �W�I�Ɍ�����
		float rotx = atan2f(vel.y, vel.z);
		float roty = atan2f(vel.x, vel.z);
		bullet.back().SetRotation(XMFLOAT3(XMConvertToDegrees(rotx), XMConvertToDegrees(roty), 0));
	}

	bullet.back().SetVel(vel);
}

void Player::Move(float speed) {
	const bool hitW = Input::GetInstance()->PushKey(DIK_W);
	const bool hitS = Input::GetInstance()->PushKey(DIK_S);
	const bool hitA = Input::GetInstance()->PushKey(DIK_A);
	const bool hitD = Input::GetInstance()->PushKey(DIK_D);
	const bool hitZ = Input::GetInstance()->PushKey(DIK_Z);
	const bool hitX = Input::GetInstance()->PushKey(DIK_X);

	if (hitW || hitS || hitA || hitD || hitZ || hitX) {
		auto pos = obj->GetPosition();

		if (hitW) {
			pos.y += speed;
		}
		else if (hitS) {
			pos.y -= speed;
		}

		if (hitD) {
			pos.x += speed;
		}
		else if (hitA) {
			pos.x -= speed;
		}

		if (hitZ) {
			pos.z += speed;
		}
		else if (hitX) {
			pos.z -= speed;
		}

		obj->SetPosition(pos);
	}
}

//void Player::Bullet()
//{
//	const bool hitSpace = Input::GetInstance()->PushKey(DIK_SPACE);
//
//	if (hitSpace) {
//
//	}
//}