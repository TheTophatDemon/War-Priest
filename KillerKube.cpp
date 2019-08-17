#include "KillerKube.h"

#include "Gameplay.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>

#include "Settings.h"
#include "Missile.h"
#include "Actor.h"
#include "WeakChild.h"
#include "Zeus.h"
#include "Player.h"

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
	blackHoleNode = BlackHole::MakeBlackHole(scene, node_->GetWorldPosition(), node_);
	blackHoleNode->SetParent(node_);
	blackHoleNode->SetScale(0.1f);
	blackHoleNode->SetEnabled(false);

	blackHoleAnimation = SharedPtr<ValueAnimation>(new ValueAnimation(context_));
	blackHoleAnimation->SetKeyFrame(0.0f, Vector3(0.1f, 0.1f, 0.1f));
	blackHoleAnimation->SetKeyFrame(2.5f, Vector3(32.0f, 32.0f, 32.0f));
	blackHoleAnimation->SetKeyFrame(5.0f, Vector3(128.0f, 128.0f, 128.0f));
	blackHoleAnimation->SetKeyFrame(10.0f, Vector3(0.1f, 0.1f, 0.1f));
	blackHoleAnimation->SetInterpolationMethod(IM_LINEAR);
	blackHoleAnimation->SetSplineTension(0.25f);

	//Make the Pilot dance
	node_->GetChild("postalpilot", false)->GetOrCreateComponent<AnimationController>()->PlayExclusive("Models/enemy/postalpilot_idle.ani", 0, true, 0.0f);

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(KillerKube, OnSettingsChange));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(KillerKube, OnCollision));
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(KillerKube, OnCheatWindowEvent));

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

	if (game->player->cheating) 
	{
		if (GetSubsystem<Input>()->GetKeyPress(KEY_KP_7))
			ChangeState(STATE_BLACKHOLE);
		if (GetSubsystem<Input>()->GetKeyPress(KEY_KP_8))
			ChangeState(STATE_MISSILE);
	}

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
				if (Random(0.0f, 1.0f) > 0.5f)
					ChangeState(STATE_BLACKHOLE);
				else
					ChangeState(STATE_MISSILE);
			}
			else
			{
				ChangeState(STATE_MISSILE);
			}
		}

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
			const Vector3 shootDirection = Vector3(
				Random(-1.0f, 1.0f), 0.0f, Random(-1.0f, 1.0f)
			).Normalized();
			Quaternion shootRotation = Quaternion();
			shootRotation.FromLookRotation(shootDirection, Vector3::UP);
			Missile::MakeMissile(scene, node_->GetWorldPosition() + shootDirection * 8.0f, shootRotation, node_, game->playerNode);
			soundSource->Play(SOUND_MISSILE, true);
			Zeus::PuffOfSmoke(scene, node_->GetWorldPosition() + shootDirection * 8.0f, 0.5f)->SetScale(2.0f);
		}
		break;
	case STATE_BLACKHOLE:
		if (blackHoleNode->GetAttributeAnimationTime("Scale") < (blackHoleAnimation->GetEndTime() - blackHoleAnimation->GetBeginTime()) / 2.0f)
		{
			spinSpeed = 10.0f + blackHoleNode->GetAttributeAnimationTime("Scale") * 2.0f;
		}
		else
		{
			spinSpeed = 10.0f + (blackHoleAnimation->GetEndTime() - blackHoleNode->GetAttributeAnimationTime("Scale")) * 2.0f;
		}
		node_->Rotate(Quaternion(spinSpeed, Vector3::UP), TS_WORLD);
		body->SetLinearVelocity(Vector3::ZERO);
		
		if (blackHoleNode->GetAttributeAnimationTime("Scale") >= blackHoleAnimation->GetEndTime())
		{
			body->SetAngularVelocity(Vector3::ZERO);
			ChangeState(STATE_FLY);
		}
		break;
	}
	deltaTime = timeStep;
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

void KillerKube::OnCheatWindowEvent(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = dynamic_cast<UIElement*>(eventData["Element"].GetPtr());
	if (source)
	{
		if (source->HasTag("cheatWindowButton"))
		{
			if (source->GetName() == "kubeMissileButton")
			{
				ChangeState(STATE_MISSILE);
			}
			else if (source->GetName() == "kubeBlackHoleButton")
			{
				ChangeState(STATE_BLACKHOLE);
			}
		}
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
			blackHoleNode->SetAttributeAnimation("Scale", blackHoleAnimation, WM_CLAMP, 1.0f);
			blackHoleNode->SetAttributeAnimationTime("Scale", 0.0f);
			spinSpeed = 1.0f;
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