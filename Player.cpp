#include "Player.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/DrawableEvents.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D\Physics\PhysicsUtils.h>
#include <Urho3D/Math/Matrix3x4.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "TempEffect.h"

#include "Zeus.h"
#include "Projectile.h"
#include "GunPriest.h"
#include "Settings.h"

using namespace Urho3D;

#define IDLE_ANIM "Models/grungle_idle.ani"
#define WALK_ANIM "Models/grungle_walk.ani"
#define JUMP_ANIM "Models/grungle_jump.ani"
#define SLIDE_ANIM "Models/grungle_slide.ani"
#define HAIL_ANIM "Models/grungle_hailmary.ani"
#define REVIVE_ANIM "Models/grungle_revive.ani"

#define PITCH_LIMIT_HIGHER 65
#define PITCH_LIMIT_LOWER 15

#define STATE_DEFAULT 0
#define STATE_REVIVE 1
#define STATE_SLIDE 2
#define STATE_DEAD 3
#define STATE_WIN 4
#define STATE_DROWN 5

#define ACCELERATION 150.0f
#define FRICTION 0.85f
#define FALLSPEED 50.0f
#define MAXFALL 30.0f
#define JUMPSTRENGTH 18.0f
#define WALKSPEED 15.0f
#define SLIDESPEED 20.0f

#define MAXHEALTH 100

const float Player::reviveCooldownMax = 1.25f;
Vector3 Player::cameraOffset = Vector3(0.0f, 14.4579f, -12.0f);

Player::Player(Context* context) : 
	LogicComponent(context),
	reviveCount(0),
	hailTimer(0),
	stateTimer(0),
	hurtTimer(0),
	reviveCooldown(0.0f),
	state(STATE_DEFAULT),
	health(MAXHEALTH),
	cameraPitch(0.0f),
	optimalCamPos(Vector3::ZERO),
	splashNode(nullptr),
	lastChance(false)
{
}

void Player::RegisterObject(Context* context)
{
	context->RegisterFactory<Player>();
}

void Player::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);

	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
	shape = node_->GetComponent<CollisionShape>(); //The main shape is always first
	cache = GetSubsystem<ResourceCache>(); 
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	input = GetSubsystem<Input>();
	cameraNode = game->cameraNode;
	camera = game->camera;
	pivot = scene->CreateChild();
	cameraNode->SetParent(pivot);
	cameraNode->SetPosition(cameraOffset);

	if (!node_->HasComponent<Actor>())
	{
		actor = node_->CreateComponent<Actor>();
	}
	else
	{
		actor = node_->GetComponent<Actor>();
	}
	actor->acceleration = ACCELERATION;
	actor->friction = FRICTION;
	actor->fallspeed = FALLSPEED;
	actor->maxfall = MAXFALL;
	actor->jumpStrength = JUMPSTRENGTH;
	actor->maxspeed = WALKSPEED;

	groundDetector = node_->GetChild("groundDetector")->GetComponent<RigidBody>();
	if (!groundDetector)
		std::cout << "GROUND DETECTOR MISSING" << std::endl;

	//Get Model
	modelNode = node_->GetChild("model");
	if (!modelNode)
		std::cout << "PLAYER HAS NO MODEL!" << std::endl;
	newRotation = modelNode->GetRotation();
	node_->RemoveChild(modelNode);
	scene->AddChild(modelNode);

	if (modelNode->HasComponent<AnimationController>())
		animController = modelNode->GetComponent<AnimationController>();
	else
		animController = modelNode->CreateComponent<AnimationController>();

	//Drop Shadow
	dropShadow = scene->CreateChild();
	StaticModel* shadModel = dropShadow->CreateComponent<StaticModel>();
	shadModel->SetModel(cache->GetResource<Model>("Models/shadow.mdl"));
	shadModel->SetMaterial(cache->GetResource<Material>("Materials/shadow.xml"));

	bloodEmitter = node_->GetChild("blood")->GetComponent<ParticleEmitter>();
	bloodEmitter->SetEmitting(false);

	soundSource = node_->CreateComponent<SoundSounder>();
	currentCheckpoint = scene->GetChild("exit");
	if (!currentCheckpoint.Get())
	{
		currentCheckpoint = scene->CreateChild("exit");
		currentCheckpoint->SetWorldPosition(node_->GetWorldPosition());
	}

	//Arrow
	arrowNode = scene->CreateChild();
	arrowNode->SetScale(0.5f);
	StaticModel* sm = arrowNode->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/arrow.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/arrow.xml"));

	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion(360.0f, Vector3::UP));
	arrowNode->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);

	arrowNode->SetEnabled(false);
	WeakChild::MakeWeakChild(arrowNode, node_);
	
	lastChance = false;

	cache->GetResource<ParticleEffect>("Particles/splash.xml");

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
	SubscribeToEvent(Projectile::E_PROJECTILEHIT, URHO3D_HANDLER(Player, OnProjectileHit));
	SubscribeToEvent(God::E_BEAMED, URHO3D_HANDLER(Player, OnBeamed));

	animController->StopAll();
}

void Player::FixedUpdate(float timeStep)
{	
	if (state != STATE_WIN && state != STATE_DEAD)
	{
		modelNode->SetPosition(node_->GetWorldPosition());
		modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
	}
	//Cheats
	if (input->GetKeyDown(KEY_K))
	{
		health = 0.0f; lastChance = true;
	}
	if (input->GetKeyDown(KEY_KP_PLUS))
		health = 100.0f;
	if (input->GetKeyDown(KEY_KP_0))
		actor->onGround = true;
	speedy = input->GetKeyDown(KEY_KP_PERIOD);

	bloodEmitter->ApplyEffect();
	float newAngle = 0.0f;
	
	forwardKey = Settings::IsKeyDown(input, Settings::GetForwardKey());
	backwardKey = Settings::IsKeyDown(input, Settings::GetBackwardKey());
	rightKey = Settings::IsKeyDown(input, Settings::GetRightKey());
	leftKey = Settings::IsKeyDown(input, Settings::GetLeftKey());
	jumpKey = Settings::IsKeyDown(input, Settings::GetJumpKey());
	reviveKey = Settings::IsKeyDown(input, Settings::GetReviveKey());
	slideKey = Settings::IsKeyDown(input, Settings::GetSlideKey());

	if (game->winState == 1)
	{
		ChangeState(STATE_WIN);
	}
	if (hurtTimer > 0)
	{
		hurtTimer -= 1;
		if (hurtTimer <= 0)
		{
			bloodEmitter->SetEmitting(false);
		}
	}

	if (health <= 0 && !lastChance)
	{
		health = 0;
		lastChance = true;
	}
	else if (health < 0 && lastChance)
	{
		lastChance = false;
		health = -10;
		ChangeState(STATE_DEAD);
	}

	HandleShadow();

	FindNearestCorpse();

	switch (state) 
	{
	case STATE_DEFAULT:
		ST_Default(timeStep);
		break;
	case STATE_SLIDE:
		ST_Slide(timeStep);
		break;
	case STATE_DEAD:
		break;
	case STATE_WIN:
		ST_Win(timeStep);
		break;
	case STATE_DROWN:
		ST_Drown(timeStep);
		break;
	}

	if (state != STATE_WIN && state != STATE_DEAD) 
	{
		//modelNode->SetPosition(body->GetPosition());
		//modelNode->SetRotation(modelNode->GetRotation().Slerp(newRotation, 0.25f));
		HandleCamera();
	}
}

void Player::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 2)
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
		}
	}
	else if (other->HasTag("water"))
	{
		if (state != STATE_DROWN)
		{
			health -= floor(22 * Settings::GetDifficulty());
			ChangeState(STATE_DROWN);
		}
	}
	else if (other->HasTag("checkpoint"))
	{
		if (currentCheckpoint != other) std::cout << "CHECKPOINT HIT" << std::endl;
		currentCheckpoint = other;
	}
	else if (otherBody->GetCollisionLayer() & 32)
	{
		if (other->HasTag("medkit") && health != MAXHEALTH)
		{
			game->FlashScreen(Color(1.0f, 1.0f, 1.0f, 0.5f), 0.01f);
			health += other->GetVar("health").GetInt();
			if (health > MAXHEALTH) health = MAXHEALTH;
			lastChance = false;
			other->Remove();
			soundSource->Play("Sounds/itm_medkit.wav");
		}
		else if (other->HasTag("bonus"))
		{
			SendEvent(Gameplay::E_BONUSCOLLECTED);
			other->RemoveTag("bonus"); //To stop colliding with it
			SharedPtr<TempEffect> tempEffect(new TempEffect(context_));
			tempEffect->life = 0.5f;
			other->AddComponent(tempEffect, 4200, LOCAL);
			//Make it shrink away
			SharedPtr<ValueAnimation> shrink(new ValueAnimation(context_));
			shrink->SetKeyFrame(0.0f, other->GetScale());
			shrink->SetKeyFrame(0.25f, Vector3::ZERO);
			other->SetAttributeAnimation("Scale", shrink, WM_ONCE, 1.0f);
		}
	}
}

void Player::Hurt(Node* source, int amount)
{
	if (state != STATE_WIN && hurtTimer <= 0 && state != STATE_SLIDE && (lastState != STATE_DROWN || stateTimer > 3.5f) && state != STATE_DROWN) 
	{
		health -= amount * Settings::GetDifficulty();
		bloodEmitter->SetEmitting(true);
		hurtTimer = 25;
		if (source)
		{
			actor->KnockBack(12.0f, source->GetWorldRotation());
		}
		String conditional = "";
		if (!Settings::IsBloodEnabled()) conditional = "b";
		soundSource->Play("Sounds/ply_hurt" + String(Random(0, 5)) + conditional + ".wav", true);
	}
}

void Player::OnProjectileHit(StringHash eventType, VariantMap& eventData)
{
	Node* proj = (Node*)eventData["perpetrator"].GetPtr();
	Node* victim = (Node*)eventData["victim"].GetPtr();
	int damage = eventData["damage"].GetInt();
	if (victim == node_)
	{
		if (!input->GetKeyDown(KEY_KP_PLUS)) Hurt(proj, damage);
	}
}

void Player::OnBeamed(StringHash eventType, VariantMap& eventData)
{
	Node* targ = (Node*)eventData["target"].GetPtr();
	if (targ == GetNode())
	{
		if (state == STATE_WIN) 
		{
			node_->SetParent(scene);
			node_->RemoveAllChildren();
			node_->Remove();
			//std::cout << "THE PLAYER HAS BEEN BEAMED, BROS" << std::endl;
		}
 	}
}

void Player::HandleCamera()
{
	const Vector3 worldPos = body->GetPosition();
	if (state != STATE_DROWN) pivot->SetWorldPosition(body->GetPosition());
	pivot->Rotate(Quaternion(input->GetMouseMoveX() * Settings::GetMouseSensitivity(), Vector3::UP));

	if (input->GetKeyDown(KEY_KP_ENTER)) //Cheaty first person mode for fun
	{
		modelNode->SetEnabled(false);
		cameraNode->SetWorldPosition(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f));
		cameraNode->SetWorldRotation(pivot->GetWorldRotation());
		return;
	}
	else if (state != STATE_DROWN && !input->GetKeyDown(KEY_KP_DIVIDE))
	{
		modelNode->SetEnabled(true);
	}

	const Vector3 orgCamPos = cameraNode->GetWorldPosition();
	cameraNode->SetPosition(cameraOffset); //Temporarily reset camera to simplify some calculations

	const Vector3 maxCameraHeight = Vector3(cameraNode->GetWorldPosition().x_, 42.0f, cameraNode->GetWorldPosition().z_);
	const float diff = maxCameraHeight.y_ - cameraNode->GetWorldPosition().y_;

	float peekOffset = 5.0f;

	optimalCamPos = cameraNode->GetWorldPosition();

	PhysicsRaycastResult cameraDownCast;
	physworld->SphereCast(cameraDownCast, Ray(maxCameraHeight, Vector3::DOWN), 0.5f, 500.0f, 2);
	if (cameraDownCast.body_)
	{
		peekOffset = 5.0f + fabs(cameraDownCast.distance_ - diff) * 0.1f;
		if (cameraDownCast.distance_ - peekOffset < diff)
		{
			optimalCamPos.y_ = cameraDownCast.position_.y_ + peekOffset;
		}
	}

	cameraNode->SetWorldPosition(orgCamPos);
	float transitionSpeed = Min(0.15f + (1.0f / cameraDownCast.distance_), 0.9f);
	if (input->GetMouseMoveX() > 8)
	{
		transitionSpeed = 1.0f;
	}
	cameraNode->Translate((optimalCamPos - cameraNode->GetWorldPosition()) * transitionSpeed, TS_WORLD);

	//Camera rotation
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((worldPos - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetWorldRotation(newAngle);

	float mvy = input->GetMouseMoveY();
	if (Settings::IsMouseInverted())
		mvy = -mvy;
	cameraPitch = Clamp(cameraPitch + (mvy * Settings::GetMouseSensitivity() * 0.25f), -15.0f, 15.0f);
	cameraNode->Rotate(Quaternion(cameraPitch, Vector3::RIGHT), TS_LOCAL);
}

void Player::HandleShadow()
{
	PhysicsRaycastResult shadowRaycast;
	Vector3 doot = Vector3(0.0f, 0.1f, 0.0f);
	physworld->RaycastSingle(shadowRaycast, Ray(node_->GetWorldPosition() + doot, Vector3::DOWN), 500.0f, 2);
	if (shadowRaycast.body_)
	{
		if (!actor->onGround && shadowRaycast.distance_ > 0.5f)
		{
			dropShadow->SetEnabled(true);
			dropShadow->SetWorldPosition(shadowRaycast.position_ + Vector3(0.0f, 0.1f, 0.0f));
			Quaternion q = Quaternion();
			q.FromLookRotation(shadowRaycast.normal_);
			dropShadow->SetRotation(q);
		}
		else
		{
			dropShadow->SetEnabled(false);
		}
	}
	else
	{
		dropShadow->SetEnabled(false);
	}
}

void Player::ChangeState(int newState)
{
	if (state != newState)
	{
		lastState = state;
		EnterState(newState);
		LeaveState(state);
	}
	stateTimer = 0;
	state = newState;
}

void Player::EnterState(int newState)
{
	PODVector<Node*> children;
	Node* gibs;
	switch (newState) 
	{
		case STATE_SLIDE:
			actor->maxspeed = SLIDESPEED;
			node_->SetRotation(Quaternion(0.0f, newRotation.EulerAngles().y_ + 90.0f, 0.0f));
			slideDirection = node_->GetDirection() * SLIDESPEED;
			if (body->GetCollisionLayer() & 128)
			{
				body->SetCollisionLayer(body->GetCollisionLayer() - 128);
			}
			soundSource->Play("Sounds/ply_slide.wav", true);
			break;
		case STATE_DEAD:
			bloodEmitter->SetEmitting(true);
			//Make gibs
			gibs = scene->CreateChild();
			gibs->LoadXML(cache->GetResource<XMLFile>("Objects/playergibs.xml")->GetRoot());
			gibs->SetWorldPosition(node_->GetWorldPosition());
			gibs->SetWorldRotation(node_->GetWorldRotation());
			gibs->SetScale(modelNode->GetScale());

			//PODVector<Node*> children;
			gibs->GetChildren(children, true);
			for (PODVector<Node*>::Iterator i = children.Begin(); i != children.End(); ++i)
			{
				Node* n = (Node*)*i;
				if (n)
				{
					n->GetComponent<RigidBody>()->ApplyImpulse(Vector3(Random(-250.0f, 250.0f), Random(-10.0f, 500.0f), Random(-250.0f, 250.0f)));
					n->CloneComponent(bloodEmitter, 0U);
					//Replace with flowers if blood disabled
					if (!Settings::IsBloodEnabled()) 
					{
						StaticModel* sm = n->GetComponent<StaticModel>();
						sm->SetModel(cache->GetResource<Model>("Models/grungle_gibflower.mdl"));
						sm->SetMaterial(cache->GetResource<Material>("Materials/skins/flower_skin.xml"));
					}
				}
			}
			//Remove everything
			HandleCamera();
			node_->Remove();
			modelNode->Remove();
			dropShadow->Remove();
			groundDetector->Remove();

			game->FlashScreen(Color::RED, 0.02f);
			game->Lose();
			break;
		case STATE_WIN:
			modelNode->SetParent(node_);
			dropShadow->Remove();
			groundDetector->Remove();
			break;
		case STATE_DROWN:
			soundSource->Play("Sounds/env_splash.wav");

			splashNode = node_->CreateChild("splash");
			ParticleEmitter* splashEmit = splashNode->CreateComponent<ParticleEmitter>();
			splashEmit->SetEffect(cache->GetResource<ParticleEffect>("Particles/splash.xml"));

			drownPhase = 0;
			break;
	}
}

void Player::LeaveState(int oldState)
{
	if (oldState == STATE_SLIDE)
	{
		actor->maxspeed = WALKSPEED;
		if (!(body->GetCollisionLayer() & 128))
		{
			body->SetCollisionLayer(body->GetCollisionLayer() + 128);
		}
	}
	else if (oldState == STATE_DROWN)
	{
		modelNode->SetEnabled(true);
		stateTimer = 0.0f;
	}
}

void Player::ST_Default(float timeStep)
{
	/*if (input->GetKeyDown(KEY_KP_DIVIDE))
	{
		modelNode->SetEnabled(false);
	}
	else
	{
		modelNode->SetEnabled(true);
	}*/

	stateTimer += timeStep;

	if (rightKey)
	{
		moveX += ACCELERATION * timeStep;
	}
	else if (leftKey)
	{
		moveX -= ACCELERATION * timeStep;
	}
	else
	{
		moveX *= FRICTION;
		if (fabs(moveX) < 0.1f) moveX = 0.0f;
	}
	if (!speedy) moveX = Clamp(moveX, -WALKSPEED, WALKSPEED);

	if (forwardKey)
	{
		moveZ += ACCELERATION * timeStep;
	}
	else if (backwardKey)
	{
		moveZ -= ACCELERATION * timeStep;
	}
	else
	{
		moveZ *= FRICTION;
		if (fabs(moveZ) < 0.1f) moveZ = 0.0f;
	}
	if (!speedy) moveZ = Clamp(moveZ, -WALKSPEED, WALKSPEED);

	if (jumpKey)
	{
		actor->Jump();
	}
	actor->SetMovement(pivot->GetWorldRotation() * Vector3(moveX, 0.0f, moveZ));

	//Decide what angle the model will be facing
	if (rightKey || leftKey || forwardKey || backwardKey)
	{
		Quaternion dir = Quaternion();
		dir.FromLookRotation(actor->rawMovement.Normalized().CrossProduct(Vector3::UP));
		node_->SetRotation(dir);
		newRotation = dir;
	}

	//Ground detection
	if (!groundDetector->IsActive())
		groundDetector->Activate();
	groundDetector->SetPosition(body->GetPosition());

	PODVector<RigidBody*> grounds;
	bool diddly = false;
	physworld->GetRigidBodies(grounds, groundDetector);
	for (PODVector<RigidBody*>::Iterator i = grounds.Begin(); i != grounds.End(); ++i)
	{
		RigidBody* b = (RigidBody*)*i;
		if (b)
		{
			if (b->GetCollisionLayer() & 2)
			{
				diddly = true;
				break;
			}
		}
	}	

	//Select Animation
	if (!diddly)
	{
		animController->PlayExclusive(JUMP_ANIM, 0, false, 0.2f);
	}
	else
	{
		dropShadow->SetEnabled(false);
		if (forwardKey || backwardKey || leftKey || rightKey)
		{
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
			hailTimer = 0;
		}
		else
		{
			hailTimer += 1;
			if (hailTimer == 500)
			{
				animController->PlayExclusive(HAIL_ANIM, 0, false, 0.2f);
			}
			else if (hailTimer < 500)
			{
				animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
			}
			else if (animController->IsAtEnd(HAIL_ANIM))
			{
				hailTimer = 0;
			}
		}
	}

	//Reviving
	if (reviveKey && reviveCooldown <= 0.0f)
	{
		animController->Play(REVIVE_ANIM, 128, false, 0.2f);
		animController->SetStartBone(REVIVE_ANIM, "torso");
		animController->SetAutoFade(REVIVE_ANIM, 0.2f);
		reviveCooldown = reviveCooldownMax;
		revived = false;
	}
	if (reviveCooldown > 0) reviveCooldown = Max(0.0f, reviveCooldown - timeStep);
	if (nearestCorpse)
	{
		const float distance = (nearestCorpse->GetNode()->GetWorldPosition() - node_->GetWorldPosition()).Length();
		if (distance < 8.0f)
		{
			arrowNode->SetEnabled(true);
			arrowNode->SetWorldPosition(nearestCorpse->GetNode()->GetWorldPosition() + Vector3(0.0f, 5.0f, 0.0f));
			if (!revived && reviveCooldown > reviveCooldownMax - 0.5f && reviveCooldown <= reviveCooldownMax - 0.25f)
			{
				revived = true;
				Zeus::MakeLightBeam(scene, nearestCorpse->GetNode()->GetWorldPosition());
				nearestCorpse->Revive();
				reviveCount += 1;
				soundSource->Play("Sounds/ply_revive.wav", true);
			}
		}
		else
		{
			arrowNode->SetEnabled(false);
		}
	}
	else
	{
		arrowNode->SetEnabled(false);
	}
	

	//Sliding
	if (slideKey && actor->onGround && stateTimer > 0.5f)
	{
		ChangeState(STATE_SLIDE);
	}

	actor->Move(timeStep);
}

void Player::ST_Slide(float timeStep)
{
	stateTimer += timeStep;
	animController->PlayExclusive(SLIDE_ANIM, 0, false, 0.2f);

	actor->SetMovement(slideDirection);
	actor->Move(timeStep);

	if (stateTimer > 0.5f)
	{
		stateTimer = 0;
		ChangeState(STATE_DEFAULT);
	}

	PhysicsRaycastResult forwardCast;
	physworld->RaycastSingle(forwardCast, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), slideDirection), shape->GetSize().x_, 2);
	if (forwardCast.body_ && fabs(forwardCast.normal_.y_) <= 0.42f)
	{
		stateTimer = 0;
		ChangeState(STATE_DEFAULT);
	}
}

void Player::ST_Win(float timeStep)
{
	actor->SetMovement(0.0f, 0.0f);
	actor->Move(timeStep);
	animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
}

void Player::ST_Drown(float timeStep)
{
	stateTimer += timeStep;
	if (stateTimer > 0.25f)
	{
		switch (drownPhase) 
		{
		case 0:
			beamNode = Zeus::MakeLightBeam(scene, node_->GetWorldPosition(), 256.0f);
			modelNode->SetEnabled(false);
			break;
		case 1:
			if (splashNode.Get())
			{
				splashNode->GetComponent<ParticleEmitter>()->SetEmitting(false);
			}
			break;
		case 4:
			if (splashNode.Get()) splashNode->Remove();
			node_->SetWorldPosition(currentCheckpoint->GetWorldPosition() + Vector3(0.0f, 5.0f, 0.0f));
			break;
		case 6:
			Zeus::MakeLightBeam(scene, node_->GetWorldPosition());
			break;
		case 7:
			modelNode->SetEnabled(true);
			ChangeState(STATE_DEFAULT);
			break;
		}
		stateTimer = 0.0f;
		drownPhase++;
	}
	if (drownPhase > 0 && beamNode.Get())
	{
		beamNode->SetWorldPosition(node_->GetWorldPosition());
	}
	if (drownPhase <= 4) 
	{
		actor->SetMovement(0.0f, 0.0f);
		actor->Move(timeStep);
	}
	else
	{
		body->SetLinearVelocity(Vector3::ZERO);
		pivot->SetWorldPosition(node_->GetWorldPosition());
	}
}

void Player::FindNearestCorpse()
{
	PODVector<Node*> enemies;
	scene->GetChildrenWithTag(enemies, "enemy", true);
	float smallestDistance = FLT_MAX;
	nearestCorpse = nullptr;
	for (PODVector<Node*>::Iterator i = enemies.Begin(); i != enemies.End(); ++i)
	{
		Node* enemy = (Node*)*i;
		if (enemy)
		{
			Enemy* e = enemy->GetDerivedComponent<Enemy>();
			if (e != nullptr) 
			{
				if (!e->revived) //'Tis dead
				{
					const float dist = (enemy->GetWorldPosition() - node_->GetWorldPosition()).LengthSquared();
					if (dist < smallestDistance)
					{
						smallestDistance = dist;
						nearestCorpse = e;
					}
				}
			}
		}
	}
	//Update the HUD compass
	if (nearestCorpse) 
	{
		Quaternion q = Quaternion();
		const Vector3 nyeh = cameraNode->GetWorldTransform().Inverse() * Vector3(nearestCorpse->GetNode()->GetWorldPosition().x_, 0.0f, nearestCorpse->GetNode()->GetWorldPosition().z_);
		const Vector3 bleh = cameraNode->GetWorldTransform().Inverse() * Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_);
		q.FromLookRotation((bleh - nyeh).Normalized());
		game->compassScene->compassRotation = q.EulerAngles().y_;
	}
	else
	{
		game->compassScene->compassRotation += 25.0f;
	}
}

Player::~Player()
{
}
