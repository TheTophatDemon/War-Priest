#include "KilledKaaba.h"

#include "Gameplay.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/Technique.h>
#include "Settings.h"
#include "Blackstone.h"
#include "Missile.h"

#define STATE_DORMANT 0
#define STATE_RISE 1
#define STATE_FLY 2
#define STATE_MISSILE 3
#define STATE_BLACKHOLE 4

#define HEIGHT_FROM_BOTTOM 5.0f
#define RISE_SPEED 750.0f
#define MAX_SPINSPEED 100.0f
#define SPIN_ACCEL 7.0f
#define SPIN_FRICTION 0.97f

KilledKaaba::KilledKaaba(Context* context) : LogicComponent(context),
	distanceFromPlayer(0.0f),
	stateTimer(0.0f),
	state(-1),
	moveSpeed(1.0f),
	spinSpeed(0.0f),
	direction(1.0f, 0.0f, 0.0f),
	targetHeight(0.0f),
	lastState(-1),
	attackTimer(0.0f),
	shootTimer(0.0f)
{
}

void KilledKaaba::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	moveSpeed = 50.0f * Settings::GetDifficulty();
}

void KilledKaaba::Start()
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
	PODVector<Node*> areas_ptrs = scene->GetChildrenWithTag("kaaba_area", true);
	for (Node* n : areas_ptrs)
	{
		n->RemoveComponent<StaticModel>();
		areas.Push(SharedPtr<Node>(n));
		SubscribeToEvent(n, E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnAreaCollision));
	}

	//The MissileFinder is only neccessary when a Kaaba is in the level, so their presences are tied.
	if (game->playerNode->GetScene() == scene && !game->playerNode->HasComponent<MissileFinder>())
		game->playerNode->CreateComponent<MissileFinder>();

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(KilledKaaba, OnSettingsChange));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnCollision));
	SubscribeToEvent(blackHoleNode, E_NODECOLLISION, URHO3D_HANDLER(KilledKaaba, OnBlackHoleCollision));

	SendEvent(Settings::E_SETTINGSCHANGED); //To initialize

	ChangeState(STATE_DORMANT);
}

void KilledKaaba::FixedUpdate(float timeStep)
{
	Vector3 plyPos = game->playerNode->GetWorldPosition(); 
	plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); 
	ourPos.y_ = 0.0f;
	const Vector3 targetDiff = (ourPos - plyPos);
	distanceFromPlayer = targetDiff.Length();

	const float hDiff = targetHeight - node_->GetWorldPosition().y_;
	const float currentSpeed = body->GetLinearVelocity().Length();
	const int spinDir = Sign(body->GetAngularVelocity().y_);

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
		node_->Rotate(Quaternion(timeStep * 100.0f, Vector3::UP), TS_LOCAL);
		if (currentSpeed < moveSpeed) 
		{
			body->ApplyForce(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
		}
		else if (currentSpeed > moveSpeed)
		{
			body->SetLinearVelocity(body->GetLinearVelocity().Normalized() * moveSpeed);
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
		
		if (hDiff < -1.0f) 
		{
			const int buffer = body->GetCollisionLayer();
			body->SetCollisionLayer(0);

			PhysicsRaycastResult result;
			physworld->SphereCast(result, Ray(node_->GetWorldPosition(), Vector3::DOWN), 8.0f, 10.0f, 2U);
			if (result.body_ == nullptr || result.body_ == body) ChangeState(STATE_RISE);

			body->SetCollisionLayer(buffer);
		}
		else if (hDiff > 1.0f)
		{
			ChangeState(STATE_RISE);
		}
		break;
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
			blackHoleModel->GetMaterial(0)->GetTechnique(0)->GetPass("muzzleflash")->SetDepthTestMode(CompareMode::CMP_ALWAYS);
		}
		else
		{
			blackHoleModel->GetMaterial(0)->GetTechnique(0)->GetPass("muzzleflash")->SetDepthTestMode(CompareMode::CMP_LESSEQUAL);
		}
		break;
	}
}

void KilledKaaba::OnAreaCollision(StringHash eventType, VariantMap& eventData)
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

void KilledKaaba::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 2 || 
		(otherBody->IsTrigger() && other->HasTag("enemy_barrier")) )
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof()) 
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			direction += normal * -Sign(distance);
		}
		direction.y_ = 0.0f;
		direction.Normalize();
		body->ApplyImpulse(Vector3(direction.x_ * moveSpeed, 0.0f, direction.z_ * moveSpeed));
	}
	else if (otherBody->GetCollisionLayer() & 128 && other->GetName() == "player")
	{
		VariantMap map = VariantMap();
		map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, node_));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, other));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, Settings::ScaleWithDifficulty(10.0f, 12.0f, 15.0f)));
		SendEvent(Projectile::E_PROJECTILEHIT, map);
	}
	else if (otherBody->GetCollisionLayer() & 64) //Shove enemies aside
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

void KilledKaaba::OnBlackHoleCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != node_ && otherBody->GetCollisionLayer() <= 1 && otherBody->GetMass() > 0)
	{ 
		//All rigid bodies that aren't likely to have code for this interaction are pushed towards the center
		const Vector3 diff = node_->GetWorldPosition() - other->GetWorldPosition();
		const float push = 12.0f + (4.0f * diff.LengthSquared());
		otherBody->ApplyImpulse(diff.Normalized() * Vector3(push, push, push) * otherBody->GetMass());
	}
}

void KilledKaaba::ChangeState(const int newState)
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

void KilledKaaba::EnterState(const int newState)
{
	switch (newState)
	{
		case STATE_DORMANT:
			glowNode->SetEnabled(false);
			glowNode->SetScale(0.0f);
			break;
		case STATE_FLY:
			direction = Vector3(Random(-1.0f, 1.0f), Random(-1.0f, 1.0f), Random(-1.0f, 1.0f));
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
			break;
		case STATE_BLACKHOLE:
			blackHoleNode->SetScale(0.1f);
			blackHoleNode->SetEnabled(true);
			break;
	}
}

void KilledKaaba::LeaveState(const int oldState)
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
		body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
		break;
	case STATE_MISSILE:
		body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
		break;
	case STATE_BLACKHOLE:
		blackHoleNode->SetScale(1.0f);
		blackHoleNode->SetEnabled(false);
		break;
	}
}

KilledKaaba::~KilledKaaba()
{
}

void KilledKaaba::RegisterObject(Context* context)
{
	context->RegisterFactory<KilledKaaba>();
}