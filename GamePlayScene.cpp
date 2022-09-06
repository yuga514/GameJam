#include "GamePlayScene.h"
#include "SceneManager.h"
#include "Audio.h"
#include "Input.h"
#include "DebugText.h"

#include "FbxObject3d.h"
#include "FbxLoader.h"
#include "DirectXCommon.h"
#include "Collision.h"
#include "ParticleManager.h"

#include "PostEffect.h"

using namespace DirectX;

void GamePlayScene::Initialize(DirectXCommon* dxcommon)
{
	// �X�v���C�g���ʃe�N�X�`���ǂݍ���
	SpriteCommon::GetInstance()->LoadTexture(1, L"Resources/gameplay.png");
	//SpriteCommon::GetInstance()->LoadTexture(2, L"Resources/aim.png");

	// �X�v���C�g�̐���
	sprite.reset(Sprite::Create(1, { 0,0 }, false, false));
	//aim.reset(Sprite::Create(2));

	// �J�����̏�����
	//camera.reset(new TrackingCamera());
	camera.reset(new Camera(WinApp::window_width,  WinApp::window_height));
	
	camera->SetEye({ 0, 5, -20 });
	 camera->SetTarget({ 0, 0, 50 });

	ObjObject3d::SetCamera(camera.get());

	// OBJ���烂�f���f�[�^��ǂݍ���

	// �X�J�C�h�[��
	skyDomeModel.reset(ObjModel::LoadFromObj("skydome"));
	skyDomeObj = ObjObject3d::Create();
	skyDomeObj->SetModel(skyDomeModel.get());
	skyDomeObj->SetScale({ 5,5,5 });

	// ���@�̓ǂݍ���
	pBulletModel.reset(ObjModel::LoadFromObj("playerBullet"));
	// �G�̓ǂݍ���
	enemyModel.reset(ObjModel::LoadFromObj("enemy"));

	//�f�o�C�X���Z�b�g
	FbxObject3d::SetDevice(dxcommon->GetDev());
	//�J�������Z�b�g
	FbxObject3d::SetCamera(camera.get());
	//�O���t�B�b�N�X�p�C�v���C������
	FbxObject3d::CreateGraphicsPipeline();
	// �v���C���[������
	player = std::make_unique<Player>();

	// �J�������v���C���[�̈ʒu�ɃZ�b�g
	// camera->SetTrackingTarget(player.get());
	// camera->SetTarget(player->GetPos());
	/*XMFLOAT3 eye = player->GetPos();
	eye.z -= 50;
	eye.y += 10;
	camera->SetEye(eye);*/

	// �G
	enemy.resize(0);

	// �p�[�e�B�N��������
	ParticleManager::GetInstance()->SetCamera(camera.get());

	// �X�V�����̊֐�������
	updateProcess = std::bind(&GamePlayScene::start, this);

	// �����ǂݍ���
	Audio::GetInstance()->LoadWave("Alarm01.wav");

	// �����Đ�
	// audio->PlayWave("Alarm01.wav");

	// �X�v���C���Ȑ�
	// posints = { start, start, p2, p3, end, end }
	points.emplace_back(XMVectorSet(0, 0, 0, 0));
	points.emplace_back(XMVectorSet(0, 0, 0, 0));
	points.emplace_back(XMVectorSet(0, 0, 20, 0));
	points.emplace_back(XMVectorSet(0, 20, 40, 0));
	points.emplace_back(XMVectorSet(0, 0, 0, 0));
	points.emplace_back(XMVectorSet(0, 0, 0, 0));

	splineStartIndex = 1;

	// CSV�ǂݍ���
	csv = Enemy::LoadCsv("Resources/enemy.csv");

}

void GamePlayScene::Finalize()
{

}

void GamePlayScene::Update()
{
	// �V�[���J��
	updateProcess();

	// �p�[�e�B�N���X�V
	ParticleManager::GetInstance()->Update();

	camera->Update();
	//fbxObj->Update();
	player->Update();
	//}
	for (auto& i : enemy) {
		i->Update();
	}

	skyDomeObj->Update();

	// �X�v���C�g�X�V
	sprite->Update();


	// aim->SetPosition({player->GetScreenAimPos().x,player->GetScreenAimPos().y,0});
	/*aim->SetPosition({ (float)Input::GetInstance()->GetMousePos().x,(float)Input::GetInstance()->GetMousePos().y,0 });
	aim->Update();*/
}

// �V�[���J��
void GamePlayScene::start()
{
	Input* input = Input::GetInstance();
	// ���U�C�N�������鎞��
	constexpr UINT mosaicFrameMax = 40;

	// ���U�C�N�̎��Ԃ��ő�܂ł�������play�֐��ɕς���
	if (++mosaicFrame > mosaicFrameMax) {
		PostEffect::GetInstance()->SetMosaicNum({ WinApp::window_width ,WinApp::window_height });
		// updateProcess��play�֐����Z�b�g
		updateProcess = std::bind(&GamePlayScene::play, this);
	}
	else {
		XMFLOAT2 mosaicLevel = {};
		float rate = (float)mosaicFrame / mosaicFrameMax;
		mosaicLevel.x = WinApp::window_width * rate;
		mosaicLevel.y = WinApp::window_height * rate;
		PostEffect::GetInstance()->SetMosaicNum(mosaicLevel);
	}
}

void GamePlayScene::play()
{
	Input* input = Input::GetInstance();

	{
		char tmp[32]{};
		sprintf_s(tmp, 32, "%u", frame);
		DebugText::GetInstance()->Print(tmp, 0, 50);
	}

	// ���W����
	if (input->PushKey(DIK_UP) || input->PushKey(DIK_DOWN) || input->PushKey(DIK_RIGHT) || input->PushKey(DIK_LEFT))
	{
		// �v���C���[�̉�]
		XMFLOAT3 playerRot = player->GetRotation();
		constexpr float rotSpeed = 1.f;

		if (input->PushKey(DIK_RIGHT)) {
			playerRot.y += rotSpeed;
		}
		if (input->PushKey(DIK_LEFT)) {
			playerRot.y -= rotSpeed;
		}
		if (input->PushKey(DIK_UP)) {
			playerRot.x -= rotSpeed;
		}
		if (input->PushKey(DIK_DOWN)) {
			playerRot.x += rotSpeed;
		}
		player->SetRotation(playerRot);
	}

	// �V�[���؂�ւ�
	if (input->TriggerKey(DIK_SPACE))
	{
		SceneManager::GetInstance()->ChangeScene("END");
	}

	// ���U�C�N�؂�ւ�
	{
		const bool TriggerP = input->TriggerKey(DIK_P);
		if (TriggerP) {
			// ���U�C�N�ׂ̍���
			// �E�C���h�E�T�C�Y�Ɠ����ׂ����̃��U�C�N = ���U�C�N�Ȃ�
			DirectX::XMFLOAT2 mosaicNum{ WinApp::window_width, WinApp::window_height };

			// ���U�C�N��������ꍇ�́A�ׂ�����ύX
			if (mosaicFlag) {
				mosaicNum = DirectX::XMFLOAT2(100, 100);
			}

			// ���U�C�N��������
			PostEffect::GetInstance()->SetMosaicNum(mosaicNum);

			// �t���O�𔽓]
			mosaicFlag = !mosaicFlag;
		}
	}

	frame++;
}

void GamePlayScene::Draw(DirectXCommon* dxcommon)
{
	// �X�v���C�g���ʃR�}���h
	SpriteCommon::GetInstance()->PreDraw();
	// �X�v���C�g�`��
	sprite->Draw();

	// 3D�I�u�W�F�N�g�`��O����
	ObjObject3d::PreDraw();

	//�X�J�C�h�[���̕`��
	skyDomeObj->Draw();

	// �v���C���[�̕`��
	player->Draw();
	// �G�̕����`��
	for (auto& i : enemy) {
		i->Draw();
	}
	/// <summary>
	/// ������3D�I�u�W�F�N�g�̕`�揈����ǉ��ł���
	/// </summary>

	// 3D�I�u�W�F�N�g�`��㏈��
	ObjObject3d::PostDraw();
	// �p�[�e�B�N���`��
	ParticleManager::GetInstance()->Draw(dxcommon->GetCmdList());
	// �X�v���C�g���ʃR�}���h
	SpriteCommon::GetInstance()->PreDraw();
}

void GamePlayScene::DrawFrontSprite(DirectXCommon* dxcommon) {
	SpriteCommon::GetInstance()->PreDraw();
	// aim->Draw();
}

XMVECTOR GamePlayScene::splinePosition(const std::vector<XMVECTOR>& posints, size_t startIndex, float t)
{
	size_t n = posints.size() - 2;

	if (startIndex > n)return posints[n];
	if (startIndex < 1)return posints[1];

	XMVECTOR p0 = posints[startIndex - 1];
	XMVECTOR p1 = posints[startIndex];
	XMVECTOR p2 = posints[startIndex + 1];
	XMVECTOR p3 = posints[startIndex + 2];

	XMVECTOR position = 0.5 * ((2 * p1 + (-p0 + p2) * t) +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
		(-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t);

	return position;
}

std::unique_ptr<Enemy>& GamePlayScene::enemyAdd(XMFLOAT3 pos, XMFLOAT3 vel)
{
	enemy.emplace_back();
	auto& e = enemy.back();

	// �G�̏������W
	e = std::make_unique<Enemy>(enemyModel.get(), pos);
	// �G�̑傫��
	e->SetScale(XMFLOAT3(enemyScale, enemyScale, enemyScale));
	// �G�̑��x
	e->SetVel(vel);
	// �G�̕W�I
	e->SetShotTarget(player.get());

	return e;
}