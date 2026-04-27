#pragma once
#include <memory>
#include <map>
#include <functional>
#include <DxLib.h>
#include "Stage.h"
#include "ActorBase.h"
class AnimationController;
class Collider;
class Capsule;
class Stage;

class Player : public ActorBase
{

public:

	// スピード
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 10.0f;

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;

	// ジャンプ力
	static constexpr float POW_JUMP = 35.0f;
	// ジャンプ受付時間
	static constexpr float TIME_JUMP_IN = 0.5f;

	// 煙エフェクト発生間隔
	static constexpr float TERM_FOOT_SMOKE = 0.3f;

	

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		WARP_RESERVE,
		WARP_MOVE,
		DEAD,
		VICTORY,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		WARP_PAUSE,
		FLY,
		FALLING,
		VICTORY
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 衝突用カプセルの取得
	const Capsule& GetCapsule(void) const;

	// ワープ準備開始
	void StartWarpReserve(
		float time, const Quaternion& goalRot, const VECTOR& goalPos);

	bool IsPlay(void)const;
	bool IsWarpMove(void)const;
private:

	// ジャンプ量
	VECTOR jumpPow_;
	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;
	// 衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;


	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// 状態管理
	STATE state_;
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;
	
	// 移動スピード
	float speed_;
	// 移動方向
	VECTOR moveDir_;
	// 移動量
	VECTOR movePow_;
	// 移動後の座標
	VECTOR movedPos_;

	// 足煙エフェクト
	int effectSmokeResId_;
	float stepFootSmoke_;

	// 丸影
	int imgShadow_;

	int effectWarpOrbitResId_;
	int effectWarpOrbitL_PlayId_;
	int effectWarpOrbitR_PlayId_;

	int frameLeftHand_;
	int frameRightHand_;

	// フレームごとの移動値
	VECTOR moveDiff_;

	// ワープ準備時間
	float timeWarp_;
	// ワープ準備経過時間
	float stepWarp_;
	// ワープ準備完了時の回転
	Quaternion warpQua_;
	// ワープ準備完了時の座標
	VECTOR warpReservePos_;
	// ワープ準備開始時のプレイヤー情報
	Quaternion reserveStartQua_;
	VECTOR reserveStartPos_;

	// ワープ前の惑星名
	Stage::NAME preWarpName_;

	//傾斜の方向
	VECTOR slopeDir_;

	//傾斜角
	float slopeAngleDeg_;

	//傾斜の力
	VECTOR slopePow_;

	//衝突している地面ポリゴンの法線
	VECTOR hitNormal_;

	//衝突している地面との座標
	VECTOR hitPos_;

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ判定
	bool isJump_;
	// ジャンプの入力受付時間
	float stepJump_;
	

	void InitAnimation(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateWarpReserve(void);
	void ChangeStateWarpMove(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateWarpReserve(void);
	void UpdateWarpMove(void);

	// 描画系
	void DrawDebug(void);
	void DrawShadow(void);

	// 操作 
	void ProcessMove(void);

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);
	// 移動量の計算
	void CalcGravityPow(void);

	void CalcSlope(void);

	std::unique_ptr<Capsule> capsule_;
	void CollisionCapsule(void);

	void ProcessJump(void);
	// 着地モーション終了
	bool IsEndLanding(void);

	// 足煙エフェクト
	void EffectFootSmoke(void);

	
	
	

	void EffectWarpOrbit(void);
	void SyncEffectWarpOrbitPos(void);
};
