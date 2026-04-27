#include <vector>
#include <map>
#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/GravityManager.h"
#include "WarpStar.h"
#include "Player.h"
#include "Planet.h"
#include "Common/Collider.h"
#include "Common/Transform.h"
#include "Stage.h"

Stage::Stage(Player& player) 
	: resMng_(ResourceManager::GetInstance()), player_(player)
{
	activeName_ = NAME::MAIN_PLANET;
	step_ = 0.0f;
}

Stage::~Stage(void)
{
	
	// ワープスター
	warpStars_.clear();
	
	// 惑星
	planets_.clear();

}

void Stage::Init(void)
{
	MakeMainStage();
	MakeFallPlanet();
	MakeFlatPlanet();
	MakeLastPlanet();
	MakeSpecialPlanet();
	MakeWarpStar();
	MakeGoalStar();

	step_ = -1.0f;
}

void Stage::Update(void)
{

	// ワープスター
	for (const auto& s : warpStars_)
	{
		s->Update();
	}

	// 重力範囲が重なっていた場合、惑星がコロコロ切り替わらないように
// 一定時間ステージが変わらないようにする
	bool isPossibleChange = true;
	if (step_ > 0.0f)
	{
		step_ -= SceneManager::GetInstance().GetDeltaTime();
		isPossibleChange = false;
	}

	// 惑星
	for (const auto& s : planets_)
	{
		s.second->Update();
		// 演習の実装箇所


		if (isPossibleChange && s.second->InRangeGravity(player_.GetTransform().pos) && activeName_ != s.second->GetName())
		{

			ChangeStage(s.second->GetName());
			isPossibleChange = false;
		}
	}

}

void Stage::Draw(void)
{

	// ワープスター
	for (const auto& s : warpStars_)
	{
		s->Draw();
	}

	// 惑星
	for (const auto& s : planets_)
	{
		s.second->Draw();
	}

}

void Stage::ChangeStage(NAME type)
{

	activeName_ = type;

	// 対象のステージを取得する
	activePlanet_ = GetPlanet(activeName_);

	// ステージの当たり判定をプレイヤーに設定
	player_.ClearCollider();
	player_.AddCollider(activePlanet_.lock()->GetTransform().collider);

	// 重力制御に惑星を渡す
	GravityManager::GetInstance().ChangeActivePlanet(activePlanet_);

	step_ = TIME_STAGE_CHANGE;

}

std::weak_ptr<Planet> Stage::GetPlanet(NAME type)
{
	if (planets_.count(type) == 0)
	{
		return nullPlanet;
	}

	return planets_[type];
}

void Stage::MakeMainStage(void)
{

	// 最初の惑星
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_PLANET));
	planetTrans.scl = AsoUtility::VECTOR_ONE;
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { 0.0f, -100.0f, 0.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	NAME name = NAME::MAIN_PLANET;
	std::shared_ptr<Planet> planet =
		std::make_shared<Planet>(
			name, Planet::TYPE::GROUND, planetTrans);
	planet->Init();
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

}

void Stage::MakeFallPlanet(void)
{

	// 落とし穴の惑星
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FALL_PLANET));
	float scale = 10.0f;
	planetTrans.scl = { scale, scale, scale };
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { -12000.0f, 4100.0f, 10200.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	NAME name = NAME::FALL_PLANET;
	std::shared_ptr<Planet> planet = 
		std::make_shared<Planet>(
			name, Planet::TYPE::SPHERE, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(3000.0f);
	planet->SetDeadLength(500.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

}

void Stage::MakeFlatPlanet(void)
{

	Transform planetTrans;
	std::shared_ptr<Planet> planet;
	NAME name;
	float size;
	VECTOR scale;

	// ① GROUNDタイプ(重力がY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	size = 10.0f;
	planetTrans.scl = { size, size, size };
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { -18000.0f, 7000.0f, 15000.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_BASE;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::GROUND, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(1300.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

	// 以後の惑星のスケール
	//------------------------------------------------------------------------------
	size = 3.0f;
	scale = { size, size, size };
	//------------------------------------------------------------------------------

	// ② TRANS_ROTタイプ(重力が回転方向のY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-10.0f));
	planetTrans.pos = { -18000.0f, 7000.0f, 16800.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_ROT01;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::TRANS_ROT, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(420.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------


	// ③ TRANS_ROTタイプ(重力が回転方向のY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-70.0f));
	planetTrans.pos = { -18000.0f, 7000.0f, 17900.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_ROT02;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::TRANS_ROT, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(420.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------


	// ④ TRANS_ROTタイプ(重力が回転方向のY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-45.0f));
	planetTrans.pos = { -18000.0f, 7000.0f, 19000.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_ROT03;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::TRANS_ROT, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(420.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------


	// ⑤ TRANS_ROTタイプ(重力が回転方向のY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(0.0f));
	planetTrans.pos = { -18000.0f, 7000.0f, 20100.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_ROT04;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::TRANS_ROT, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(420.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------


	// ⑥ TRANS_CAMERA_FIXEDタイプ(重力が回転方向のY+だがカメラ固定)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_02));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(60.0f));
	planetTrans.pos = { -18000.0f, 7200.0f, 21200.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_FIXED01;
	planet = std::make_shared<Planet>(
		name, Planet::TYPE::TRANS_CAMERA_FIXED, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(420.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------


	// ⑦ TTRANS_ROTタイプ(重力が回転方向のY+)
	//------------------------------------------------------------------------------
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::FLAT_PLANET_01));
	planetTrans.scl = scale;
	planetTrans.quaRot = Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(0.0f));
	planetTrans.pos = { -18000.0f, 7300.0f, 22400.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	name = NAME::FLAT_PLANET_FIXED02;
	planet = std::make_shared<Planet>(name, Planet::TYPE::TRANS_ROT, planetTrans);
	planet->Init();
	planet->SetGravityPow(12.0f);
	planet->SetGravityRadius(520.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

}

void Stage::MakeLastPlanet(void)
{

	// 最後の惑星
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::LAST_PLANET));
	float scale = 8.0f;
	planetTrans.scl = { scale, scale, scale };
	planetTrans.quaRot = Quaternion::Euler(AsoUtility::Deg2RadF(-15.0f), 0.0f, 0.0f);
	planetTrans.pos = { -18000.0f, 8400.0f, 25200.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	NAME name = NAME::LAST_STAGE;
	std::shared_ptr<Planet> planet = 
		std::make_shared<Planet>(name, Planet::TYPE::SPHERE, planetTrans);
	planet->Init();
	planet->SetGravityPow(20.0f);
	planet->SetGravityRadius(2300.0f);
	planet->SetDeadLength(0.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

}

void Stage::MakeSpecialPlanet(void)
{

	// 特別な惑星惑星
	//------------------------------------------------------------------------------
	Transform planetTrans;
	planetTrans.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::SPECIAL_PLANET));
	float scale = 8.0f;
	planetTrans.scl = { scale, scale, scale };
	planetTrans.quaRot = Quaternion();
	planetTrans.pos = { -6000.0f, -150.0f, -1500.0f };

	// 当たり判定(コライダ)作成
	planetTrans.MakeCollider(Collider::TYPE::STAGE);

	planetTrans.Update();

	NAME name = NAME::SPECIAL_STAGE;
	std::shared_ptr<Planet> planet = std::make_shared<Planet>(
		name, Planet::TYPE::ROAD, planetTrans);
	planet->Init();
	planet->SetGravityRadius(700.0f);
	planet->SetDeadLength(1000.0f);
	planets_.emplace(name, std::move(planet));
	//------------------------------------------------------------------------------

}

void Stage::MakeWarpStar(void)
{

	Transform trans;
	std::unique_ptr<WarpStar> star;

	// 落とし穴惑星へのワープスター
	//------------------------------------------------------------------------------
	trans.pos = { -910.0f, 200.0f, 894.0f };
	trans.scl = { 0.6f, 0.6f, 0.6f };
	trans.quaRot = Quaternion::Euler(
		AsoUtility::Deg2RadF(-25.0f),
		AsoUtility::Deg2RadF(-50.0f),
		AsoUtility::Deg2RadF(0.0f)
	);

	star = std::make_unique<WarpStar>(player_, trans);
	star->Init();
	warpStars_.push_back(std::move(star));
	//------------------------------------------------------------------------------


	// 平坦な惑星へのワープスター
	//------------------------------------------------------------------------------
	trans.pos = { -11360.0f, 6200.0f, 9600.0f };
	trans.scl = { 0.6f, 0.6f, 0.6f };
	trans.quaRot = Quaternion::Euler(
		AsoUtility::Deg2RadF(-12.0f),
		AsoUtility::Deg2RadF(-50.0f),
		AsoUtility::Deg2RadF(0.0f)
	);

	star = std::make_unique<WarpStar>(player_, trans);
	star->Init();
	warpStars_.push_back(std::move(star));
	//------------------------------------------------------------------------------

}

void Stage::MakeGoalStar(void)
{
}
