#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "Common/Transform.h"
#include "Common/Capsule.h"
#include "Player.h"
#include "WarpStar.h"

WarpStar::WarpStar(Player& player, const Transform& transform) : player_(player) 
{

	transform_ = transform;

	state_ = STATE::NONE;

	// 状態管理
	stateChanges_.emplace(STATE::IDLE, std::bind(&WarpStar::ChangeStateIdle, this));
	stateChanges_.emplace(STATE::RESERVE, std::bind(&WarpStar::ChangeStateReserve, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&WarpStar::ChangeStateMove, this));

}

WarpStar::~WarpStar(void)
{
}

void WarpStar::Init(void)
{

	// モデルの基本情報
	transform_.SetModel(
		resMng_.LoadModelDuplicate(
			ResourceManager::SRC::WARP_STAR)
	);
	transform_.Update();

	// Zが無回転の状態を保持しておく
	VECTOR angles = transform_.quaRot.ToEuler();
	warpQua_ = Quaternion::Euler(angles.x, angles.y, 0.0f);
	// ワープ開始座標(ワールド座標)
	warpStartPos_ =
		VAdd(transform_.pos, warpQua_.PosAxis(WARP_RELATIVE_POS));

	// 回転エフェクト
	effectRotParticleResId_ = resMng_.Load(
		ResourceManager::SRC::WARP_STAR_ROT_EFF).handleId_;
	stepEffect_ = TERM_EFFECT;
	ChangeState(STATE::IDLE);


}

void WarpStar::Update(void)
{

	// 更新ステップ
	stateUpdate_();

}

void WarpStar::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}

void WarpStar::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();

}

void WarpStar::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateIdle, this);
}

void WarpStar::ChangeStateReserve(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateReserve, this);
	// プレイヤーの状態を変更
	player_.StartWarpReserve(TIME_WARP_RESERVE, warpQua_, warpStartPos_);
}

void WarpStar::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateMove, this);
}

void WarpStar::UpdateIdle(void)
{
	// 回転
	RotateZ(SPEED_ROT_IDLE);
	// エフェクト
	PlayEffectRotParticle();

	VECTOR diff = VSub(player_.GetCapsule().GetCenter(), transform_.pos);
	float dis = AsoUtility::SqrMagnitudeF(diff);
	if (dis < RADIUS * RADIUS)
	{
		ChangeState(STATE::RESERVE);
		return;
	}
}

void WarpStar::UpdateReserve(void)
{
	// 回転
	RotateZ(SPEED_ROT_RESERVE);
	// エフェクト
	PlayEffectRotParticle();

	if (player_.IsWarpMove())
	{
		ChangeState(STATE::MOVE);
	}
}

void WarpStar::UpdateMove(void)
{
	if (player_.IsPlay())
	{
		ChangeState(STATE::IDLE);
	}

}



void WarpStar::RotateZ(float speed)
{
	Quaternion zRot = Quaternion::AngleAxis(AsoUtility::Deg2RadD(speed), AsoUtility::AXIS_Z);

	transform_.quaRot = transform_.quaRot.Mult(zRot);
	transform_.Update();
} 

void WarpStar::PlayEffectRotParticle(void)
{
	stepEffect_ -= scnMng_.GetDeltaTime();
	if (stepEffect_ < 0.0f)
	{
		stepEffect_ = TERM_EFFECT;

		int playhandle = PlayEffekseer3DEffect(effectRotParticleResId_);

		//エフェクトの大きさ
		SetScalePlayingEffekseer3DEffect(playhandle, 5.0f, 5.0f, 5.0f);


		Quaternion rot = transform_.quaRot;
		VECTOR angles = rot.ToEuler();
		SetRotationPlayingEffekseer3DEffect(playhandle, angles.x, angles.y, angles.z);

		//エフェクトの位置
		VECTOR pos = VAdd(transform_.pos, rot.PosAxis(EFFECT_RELATIVE_POS));
		SetPosPlayingEffekseer3DEffect(playhandle, pos.x, pos.y, pos.z);
	}
}
