#include "KillerKube.h"

#include "Gameplay.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/Technique.h>
#include "Settings.h"
#include "Blackstone.h"
#include "Missile.h"
#include "MissileFinder.h"
#include "Actor.h"
#include "WeakChild.h"

#define STATE_DORMANT 0
#define STATE_RISE 1
#define STATE_FLY 2
#define STATE_MISSILE 3
#define STATE_BLACKHOLE 4

#define HEIGHT_FROM_BOTTOM 5.25f
#define RISE_SPEED 750.0f
#define MAX_SPINSPEED 100.0f
#define SPIN_ACCEL 7.0f
#define SPIN_FRICTION 0.97f

#define SOUND_BLACKHOLE "Sounds/enm_blackhole.wav"
#define SOUND_FLY "Sounds/enm_fly.wav"
#define SOUND_BOUNCE "Sounds/enm_bounce.wav"
#define SOUND_MISSILE "Sounds/enm_missile.wav"
#define SOUND_INTRO "Sounds/enm_kubeintro.wav"

KillerKube::KillerKube(Context* context) : LogicComponent(context),
	distanceFromPlayer(0.0f),
	stateTimer(0.0f),
	state(-1),
	maxMoveSpeed(1.0f),
	moveSpeed(0.0f),
	spinSpeed(0.0f),
	direction(1.0f, 0.0f, 0.0f),
	targetHeight(0.0f),
	lastState(-1),
	attackTimer(0.0f),
	shootTimer(0.0f),
	deltaTime(0.0f),
	collideTime(0.0f)
{
}

void KillerKube::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	maxMoveSpeed = 50.0f * Settings::GetDifficulty();
}

void KillerKube::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	
	body = node_->GetComponent<RigidBody>();
	body->SetLinearVelocity(Vector3::ZERO);
	body->SetAngularFactor(Vector3::ZERO);

	targetHeight = node_->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;

	soundNode = scene->CreateChild("sound");
	soundSource = soundNode->CreateComponent<SoundSounder>();
	soundSource->SetDistanceAttenuation(10.0f, 700.0f, 3.0f);
	soundSource2D = soundNode->CreateComponent<SoundSource>();
	soundSource2D->SetSoundType("GAMEPLAY");
	WeakChild::MakeWeakChild(soundNode, node_);

	//Glowy red aura
	glowNode = node_->CreateChild();
	StaticModel* sm = glowNode->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/fireglow.xml"));

	//Black hole
	blackHoleNode = node_->CreateChild();
	blackHoleNode->AddTag("blackhole");
	blackHoleModel = blackHoleNode->CreateComponent<StaticModel>();
	blackHoleModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	blackHoleModel->SetMaterial(cache->GetResource<Material>("Materials/blackhole.xml"));
	RigidBody* bhrb = blackHoleNode->CreateComponent<RigidBody>();
	bhrb->SetCollisionLayer(17); //1+16
	bhrb->SetTrigger(true);
	bhrb->SetLinearFactor(Vector3::ZERO);
	bhrb->SetAngularFactor(Vector3::ZERO);
	CollisionShape* bhcol = blackHoleNode->CreateComponent<CollisionShape>();
	bhcol->SetSphere(1.0f, Vector3::ZERO, Quaternion::IDENTITY);
	blackHoleNode->SetScale(0.1f);
	blackHoleNode->SetEnabled(false);

	//Find the boundaries of its flying space
	areas = Vector<SharedPtr<Node>>();
	PODVector<Node*> areas_ptrs = scene->GetChildrenWithTag("kube_area", true);
	for (Node* n : areas_ptrs)
	{
		n->RemoveComponent<StaticModel>();
		areas.Push(SharedPtr<Node>(n));
		SubscribeToEvent(n, E_NODECOLLISION, URHO3D_HANDLER(KillerKube, OnAreaCollision));
	}

	//The MissileFinder is only neccessary when a Kube is in the level, so their presences are tied.
	if (game->playerNode->GetScene() == scene && !game->playerNode->HasComponent<MissileFinder>())
		game->playerNode->CreateComponent<MissileFinder>();

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(KillerKube, OnSettingsChange));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(KillerKube, OnCollision));
	SubscribeToEvent(blackHoleNode, E_NODECOLLISION, URHO3D_HANDLER(KillerKube, OnBlackHoleCollision));

	SendEvent(Settings::E_SETTINGSCHANGED); //To initialize

	ChangeState(STATE_DORMANT);
}

void KillerKube::FixedUpdate(float timeStep)
{
	soundNode->SetWorldPosition(node_->GetWorldPosition());
	Vector3 plyPos = game->playerNode->GetWorldPosition(); 
	plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); 
	ourPos.y_ = 0.0f;
	const Vector3 targetDiff = (ourPos - plyPos);
	distanceFromPlayer = targetDiff.Length();

	const float hDiff = targetHeight - node_->GetWorldPosition().y_;

	stateTimer += timeStep;
	switch (state)
	{
	case STATE_DORMANT:
	{
		body->SetLinearVelocity(Vector3::ZERO);

		//Instead of a direct distance, we measure the distance from the player to an infinite plane spanning the Kube's front surface
		//so that you can't walk past it without activating it.
		const Vector3 actualForward = node_->GetWorldDirection().CrossProduct(Vector3::UP);
		const float distanceToFrontPlane = (-targetDiff).DotProduct(actualForward);
		if (fabs(distanceToFrontPlane) < 40.0f)
		{
			soundSource2D->Play(cache->GetResource<Sound>(SOUND_INTRO));
			soundSource->Play(SOUND_FLY, true);
			ChangeState(STATE_RISE);
		}
		break;
	}
	case STATE_RISE:
		node_->Rotate(Quaternion(spinSpeed, Vector3::UP), TS_WORLD);
		if (Abs(hDiff) < 1.0f) //Decelerate the spinning before resuming activity
		{
			body->SetLinearVelocity(Vector3::ZERO);
			spinSpeed *= SPIN_FRICTION;
			if (fabs(spinSpeed) < 1.0f)
			{
				ChangeState(STATE_FLY);
			}
		}
		else
		{
			if (lastState == STATE_DORMANT)
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff) * 0.1f, 0.0f));
			else
				body->SetLinearVelocity(Vector3(0.0f, timeStep * RISE_SPEED * Sign(hDiff), 0.0f));
			spinSpeed = Min(MAX_SPINSPEED, spinSpeed + (timeStep * SPIN_ACCEL));
		}
		break;
	case STATE_FLY:
	{
		node_->Rotate(Quaternion(timeStep * 100.0f, Vector3::UP), TS_LOCAL);
		body->SetLinearVelocity(direction * moveSpeed);

		moveSpeed = Clamp(moveSpeed + 0.5f, -maxMoveSpeed, maxMoveSpeed);

		if (GetSubsystem<Input>()->GetKeyDown(KEY_KP_6))
		{
			direction.y_ = 1.0f;
		}
		else if (GetSubsystem<Input>()->GetKeyDown(KEY_KP_5))
		{
			direction.y_ = -1.0f;
		}

		//Running into walls shifts it up, but it must return to ground level
		if (fabs(hDiff) > 0.1f)
		{
			//std::cout << direction.y_ << std::endl;
			direction.y_ = Clamp(direction.y_ + timeStep * Sign(hDiff), -1.0f, 1.0f);
		}
		else
		{
			direction.y_ = 0.0f;
		}

		attackTimer -= timeStep;
		if (attackTimer < 0.0f)
		{
			if (distanceFromPlayer < 70.0f)
			{
				ChangeState(STATE_BLACKHOLE);
			}
			else
			{
				ChangeState(STATE_MISSILE);
			}
		}

		if (GetSubsystem<Input>()->GetKeyPress(KEY_KP_7))
			ChangeState(STATE_BLACKHOLE);

		//Check for collisions 
		PhysicsRaycastResult result;
		physworld->SphereCast(result, Ray(node_->GetWorldPosition(), direction), 8.0f, 16.0f, 10U); //2 + 8
		if (result.body_)
		{
			if (collideTime == 0.0f)
			{
				soundSource->Play(SOUND_BOUNCE, true);
			}
			collideTime += deltaTime;
			if (fabs(result.normal_.x_) + fabs(result.normal_.z_) > 0.0f) direction = Vector3(result.normal_.x_, 0.0f, result.normal_.z_).Normalized() + Vector3(0.0f, collideTime * 0.25f, 0.0f);
		}
		else
		{
			collideTime = 0.0f;
		}
		
		break;
	}
	case STATE_MISSILE:
		node_->Rotate(Quaternion(spinSpeed, Vector3::UP), TS_WORLD);
		body->SetLinearVelocity(Vector3::ZERO);
		if (stateTimer < 4.0f) 
		{
			spinSpeed = Min(MAX_SPINSPEED, spinSpeed + (timeStep * SPIN_ACCEL));
		}
		else //Decelerate spinning before going back to normal
		{
			spinSpeed *= SPIN_FRICTION;
			if (fabs(spinSpeed) < 1.0f)
			{
				body->SetAngularVelocity(Vector3::ZERO);
				ChangeState(STATE_FLY);
			}
		}
		shootTimer += timeStep;
		if (shootTimer > Settings::ScaleWithDifficulty(1.0f, 0.5f, 0.25f))
		{
			shootTimer = 0.0f;
			Quaternion shootDirection = Quaternion();
			shootDirection.FromLookRotation(Vector3(
				Random(-1.0f, 1.0f), 0.0f, Random(-1.0f, 1.0f)
			).Normalized(), Vector3::UP);
			Missile::MakeMissile(scene, node_->GetWorldPosition(), shootDirection, node_, game->playerNode);
			soundSource->Play(SOUND_MISSILE, true);
		}
		break;
	case STATE_BLACKHOLE:
		node_->Rotate(Quaternion(spinSpeed, Vector3::UP), TS_WORLD);
		body->SetLinearVelocity(Vector3::ZERO);
		if (stateTimer < 16.0f)
		{
			spinSpeed = Min(MAX_SPINSPEED, spinSpeed + (timeStep * SPIN_ACCEL));
		}
		else if (stateTimer > 24.0f)
		{
			spinSpeed *= 0.25f;
		}
		if (stateTimer < 8.0f) 
		{
			blackHoleNode->SetScale(blackHoleNode->GetScale().x_ + timeStep * 32.0f);
		}
		else
		{
			blackHoleNode->SetScale(blackHoleNode->GetScale().x_ - timeStep * 32.0f / Settings::GetDifficulty());
			if (blackHoleNode->GetScale().x_ < 1.0f)
			{
				body->SetAngularVelocity(Vector3::ZERO);
				ChangeState(STATE_FLY);
			}
		}
		//Hack to make it so that it still looks black-holey when the camera is inside of the black hole model
		const float camDist = (blackHoleNode->GetWorldPosition() - game->cameraNode->GetWorldPosition()).Length();
		if (camDist < blackHoleNode->GetScale().x_ * 0.75f)
		{
			blackHoleModel->GetMaterial(0)->GetTechnique(0)->GetPass("postalpha")->SetDepthTestMode(CompareMode::CMP_ALWAYS);
		}
		else
		{
			blackHoleModel->GetMaterial(0)->GetTechnique(0)->GetPass("postalpha")->SetDepthTestMode(CompareMode::CMP_LESSEQUAL);
		}
		break;
	}
	deltaTime = timeStep;
}

void KillerKube::OnAreaCollision(StringHash eventType, VariantMap& eventData)
{
	RigidBody* body = (RigidBody*)eventData["Body"].GetPtr();
	Node* area = body->GetNode();
	Node* otherNode = (Node*)eventData["OtherNode"].GetPtr();
	if (otherNode->GetName() == "player")
	{
		for (SharedPtr<Node> a : areas)
		{
			if (a.Get() == area)
			{
				targetHeight = a->GetWorldPosition().y_ + HEIGHT_FROM_BOTTOM;
				break;
			}
		}
	}
}

void KillerKube::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 128 && other->GetName() == "player")
	{
		VariantMap map = VariantMap();
		map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, node_));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, other));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, Settings::ScaleWithDifficulty(10.0f, 12.0f, 15.0f)));
		SendEvent(Projectile::E_PROJECTILEHIT, map);
	}
	else if (otherBody->GetCollisionLayer() & 64 && state != STATE_BLACKHOLE) //Shove enemies aside
	{
		if (other->HasComponent<Actor>())
		{
			Actor* actor = other->GetComponent<Actor>();
			Quaternion direction = Quaternion();
			VectorBuffer contacts = eventData["Contacts"].GetBuffer();
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			normal.y_ = 0.0f;
			direction.FromLookRotation(-normal, Vector3::UP);
			actor->KnockBack(100.0f, direction);
		}
	}
}

void KillerKube::OnBlackHoleCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != node_ && otherBody->GetCollisionLayer() <= 1 && otherBody->GetMass() > 0)
	{ 
		//All rigid bodies that aren't likely to have code for this interaction are pushed towards the center
		const Vector3 diff = node_->GetWorldPosition() - other->GetWorldPosition();
		otherBody->ApplyImpulse(diff.Normalized() * 2.0f * otherBody->GetMass());
	}
}

void KillerKube::ChangeState(const int newState)
{
	if (newState != state)
	{
		LeaveState(state);
		EnterState(newState);
		lastState = state;
		stateTimer = 0.0f;
	}
	state = newState;
}

void KillerKube::EnterState(const int newState)
{
	switch (newState)
	{
		case STATE_DORMANT:
			glowNode->SetEnabled(false);
			glowNode->SetScale(0.0f);
			break;
		case STATE_FLY:
			moveSpeed = 0.0f;
			direction = Vector3(Random(-1.0f, 1.0f), 0.0f, Random(-1.0f, 1.0f));
			direction.Normalize();
			attackTimer = Random(Settings::ScaleWithDifficulty(8.0f, 5.0f, 3.0f), 10.0f);
			break;
		case STATE_RISE:
			spinSpeed = 10.0f;
			body->SetLinearFactor(Vector3::ONE);
			break;
		case STATE_MISSILE:
			spinSpeed = 0.0f;
			body->SetLinearFactor(Vector3::ONE);
			shootTimer = 0.0f;
			soundSource->Play(SOUND_FLY, true);
			break;
		case STATE_BLACKHOLE:
			blackHoleNode->SetScale(0.1f);
			blackHoleNode->SetEnabled(true);
			body->SetLinearFactor(Vector3::ZERO);
			soundSource->Play(SOUND_BLACKHOLE);
			break;
	}
}

void KillerKube::LeaveState(const int oldState)
{
	switch(oldState) 
	{
	case STATE_DORMANT:
	{
		glowNode->SetEnabled(true);
		glowNode->SetScale(8.25f);

		SharedPtr<ValueAnimation> valAnim(new ValueAnimation(context_));
		valAnim->SetKeyFrame(0.0f, Vector3(8.5f, 8.5f, 8.5f));
		valAnim->SetKeyFrame(0.5f, Vector3(9.0f, 9.0f, 9.0f));
		valAnim->SetKeyFrame(1.0f, Vector3(8.5f, 8.5f, 8.5f));

		glowNode->SetAttributeAnimation("Scale", valAnim, WM_LOOP, 1.0f);

		game->FlashScreen(Color::WHITE, 0.01f);
		break;
	}
	case STATE_RISE:
		body->SetLinearFactor(Vector3(1.0f, 1.0f, 1.0f));
		break;
	case STATE_MISSILE:
		body->SetLinearFactor(Vector3(1.0f, 1.0f, 1.0f));
		soundSource->StopPlaying();
		break;
	case STATE_BLACKHOLE:
		blackHoleNode->SetScale(1.0f);
		blackHoleNode->SetEnabled(false);
		body->SetLinearFactor(Vector3::ONE);
		soundSource->StopPlaying();
		break;
	}
}

KillerKube::~KillerKube()
{
}

void KillerKube::RegisterObject(Context* context)
{
	context->RegisterFactory<KillerKube>();
}