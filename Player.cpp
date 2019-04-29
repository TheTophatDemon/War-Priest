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
#include <Urho3D/Graphics/Light.h>

#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "TempEffect.h"
#include "Zeus.h"
#include "Projectile.h"
#include "GunPriest.h"
#include "Settings.h"
#include "Missile.h"
#include "TempShield.h"
#include "God.h"
#include "WeakChild.h"
#include "ProjectileWarner.h"

using namespace Urho3D;

#define IDLE_ANIM "Models/grungle_idle.ani"
#define WALK_ANIM "Models/grungle_walk.ani"
#define JUMP_ANIM "Models/grungle_jump.ani"
#define SLIDE_ANIM "Models/grungle_slide.ani"
#define HAIL_ANIM "Models/grungle_hailmary.ani"
#define REVIVE_ANIM "Models/grungle_revive.ani"
#define DROWN_ANIM "Models/grungle_drown.ani"

#define MAX_CAMERA_HEIGHT 128.0f

#define STATE_DEFAULT 0
#define STATE_REVIVE 1
#define STATE_SLIDE 2
#define STATE_DEAD 3
#define STATE_WIN 4
#define STATE_DROWN 5
#define STATE_PUPPET 6

#define MAXHEALTH 100

const float Player::reviveCooldownMax = 1.25f;
const Matrix3x4 Player::cameraOffset = Matrix3x4(Vector3(0.0f, 18.0f, -15.0f), Quaternion::IDENTITY, 1.0f);

Player::Player(Context* context) : LogicComponent(context),
	cheating(false),
	scene(nullptr),
	cache(nullptr),
	game(nullptr),
	physworld(nullptr),
	cameraNode(nullptr),
	camera(nullptr),
	actor(nullptr),
	body(nullptr),
	modelNode(nullptr),
	animController(nullptr),
	dropShadow(nullptr),
	groundDetector(nullptr),
	bloodEmitter(nullptr),
	shape(nullptr),
	input(nullptr),
	pivot(nullptr),
	soundSource(nullptr),
	cheatWindow(nullptr),
	nearestCorpse(nullptr),
	currentCheckpoint(nullptr),
	splashNode(nullptr),
	beamNode(nullptr),
	arrowNode(nullptr),
	slideDirection(0,0,0),
	startingPosition(0,0,0),
	cheatString(""),
	state(STATE_DEFAULT),
	health(MAXHEALTH),
	reviveCount(0),
	lastState(STATE_DEFAULT),
	drownPhase(0),
	stateTimer(0),
	reviveCooldown(0.0f),
	cameraPitch(-15.0f),
	cameraYaw(0.0f),
	hailTimer(0),
	hurtTimer(0),
	walkSpeed(17.0f),
	slideSpeed(22.0f),
	speedy(false),
	lastChance(false),
	revived(false),
	hovering(false),
	firstPerson(false)
{
#if _DEBUG
	cheating = true;
#endif
}

void Player::RegisterObject(Context* context)
{
	context->RegisterFactory<Player>();
	asIScriptEngine* scrEngine = context->GetSubsystem<Script>()->GetScriptEngine();
	RegisterComponent<Player>(scrEngine, "Player");
	scrEngine->RegisterObjectProperty("Player", "Node@ modelNode", offsetof(Player, modelNode));
	scrEngine->RegisterObjectProperty("Player", "float walkSpeed", offsetof(Player, walkSpeed));
	scrEngine->RegisterObjectProperty("Player", "float slideSpeed", offsetof(Player, slideSpeed));
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
	pivot->SetWorldPosition(node_->GetWorldPosition());
	pivot->SetWorldRotation(node_->GetWorldRotation());
	cameraNode->SetParent(pivot);
	cameraNode->SetPosition(cameraOffset.Translation());
	
	startingPosition = node_->GetWorldPosition();

	actor = node_->GetOrCreateComponent<Actor>();
	actor->maxSpeed = walkSpeed;

	groundDetector = node_->GetChild("groundDetector")->GetComponent<RigidBody>();
	if (!groundDetector)
		std::cout << "GROUND DETECTOR MISSING" << std::endl;

	modelNode = node_->GetChild("model");
	if (!modelNode)
		std::cout << "PLAYER HAS NO MODEL!" << std::endl;
	modelNode->SetParent(scene); //Detach from player node so that it can rotate smoothly while the player node turns instantaneously
	WeakChild::MakeWeakChild(modelNode, node_);
	modelNode->Rotate(Quaternion(-90.0f, Vector3::UP));
	
	animController = modelNode->GetOrCreateComponent<AnimationController>();
	animController->StopAll();

	dropShadow = scene->CreateChild();
	StaticModel* shadModel = dropShadow->CreateComponent<StaticModel>();
	shadModel->SetModel(cache->GetResource<Model>("Models/shadow.mdl"));
	shadModel->SetMaterial(cache->GetResource<Material>("Materials/shadow_simple.xml"));
	dropShadow->SetRotation(Quaternion::IDENTITY);

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
	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion(360.0f, Vector3::UP));

	arrowNode = scene->CreateChild();
	arrowNode->SetScale(0.5f);
	StaticModel* sm = arrowNode->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/arrow.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/arrow.xml"));
	arrowNode->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	arrowNode->SetEnabled(false);
	WeakChild::MakeWeakChild(arrowNode, node_);

	node_->CreateComponent<ProjectileWarner>();
	
	lastChance = false;

	cheatWindow = static_cast<Window*>(game->ourUI->GetChild("cheatWindow", true));
	cheatWindow->SetVisible(false);

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Player, OnCollision));
	SubscribeToEvent(Projectile::E_PROJECTILEHIT, URHO3D_HANDLER(Player, OnProjectileHit));
	SubscribeToEvent(God::E_BEAMED, URHO3D_HANDLER(Player, OnBeamed));
	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(Player, OnSettingsChange));
	SubscribeToEvent(Gameplay::E_CUTSCENE_START, URHO3D_HANDLER(Player, OnCutsceneEvent));
	SubscribeToEvent(Gameplay::E_CUTSCENE_END, URHO3D_HANDLER(Player, OnCutsceneEvent));

	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Player, OnKeyPress));
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(Player, OnCheatWindowEvent));

	SendEvent(Settings::E_SETTINGSCHANGED); //We fake a settings change
	//This is so that we only have to initialize the settings-dependent values in one piece of code
}

void Player::OnKeyPress(StringHash eventType, VariantMap& eventData)
{
	cheatString += input->GetScancodeName(eventData["Scancode"].GetInt()) + ";";
	if (cheatString.Contains("T;D;U;R;I;N;A;L;", true))
	{
		cheatString = "";
		cheating = true;
		cheatWindow->SetVisible(true);
		std::cout << "CHEAT MODE ACTIVATED" << std::endl;
	}
	else if (cheatString.Contains("T;D;B;L;E;H", true))
	{
		//Print player's node structure to console
		cheatString = "";
		UniquePtr<XMLFile> file;
		file.Reset(new XMLFile(context_));
		file->GetOrCreateRoot("player");
		using namespace std;
		cout << "==========PLAYER INFO============" << endl << endl;
		cout << "STATE: " << state << endl;
		if (node_->SaveXML(file->GetRoot()))
		{
			std::cout << file->ToString().CString() << std::endl;
		}
		else
		{
			std::cout << "COULD NOT SAVE PLAYER NODE INTO XML" << std::endl;
		}
		file->CreateRoot("pivot");
		cout << "============PIVOT INFO============" << endl << endl;
		if (pivot->SaveXML(file->GetRoot()))
		{
			cout << file->ToString().CString() << endl;
		}
		file->CreateRoot("model");
		cout << "==============MODEL INFO=============" << endl << endl;
		if (modelNode->SaveXML(file->GetRoot()))
		{
			cout << file->ToString().CString() << endl;
		}
	}
	const int keyCode = eventData["Key"].GetInt();
	if (cheating) 
	{
		switch (keyCode)
		{
		case KEY_KP_3: //Get the node pointed to by the cursor and print its properties to the console
		{
			PhysicsRaycastResult query;
			IntVector2 mousePos = input->GetMousePosition();
			Ray mouseRay = camera->GetScreenRay((float)mousePos.x_ / (float)Settings::GetResolutionX(), (float)mousePos.y_ / (float)Settings::GetResolutionY());
			//mouseRay.origin_ = cameraNode->GetWorldPosition();
			physworld->RaycastSingle(query, mouseRay, 500.0f);
			if (query.body_)
			{
				Node* mark = scene->CreateChild();
				mark->LoadXML(cache->GetResource<XMLFile>("Objects/god.xml")->GetRoot());
				mark->SetWorldPosition(query.position_);
				Node* node = query.body_->GetNode();
				UniquePtr<XMLFile> file;
				file.Reset(new XMLFile(context_));
				file->GetOrCreateRoot("rooty");
				if (node->SaveXML(file->GetRoot()))
				{
					std::cout << file->ToString().CString() << std::endl;
				}
				else
				{
					std::cout << "COULD NOT SAVE QUERIED NODE INTO XML" << std::endl;
				}
			}
			break;
		}
		case KEY_KP_9:
			Missile::MakeMissile(scene, node_->GetWorldPosition() + Vector3(0.0f, 4.0f, 0.0f), node_->GetWorldRotation(), node_, node_);
			break;
		case KEY_KP_4:
		{
			TempShield::MakeTempShield(scene, node_->GetWorldPosition(), node_);
			break;
		}
		case KEY_KP_5:
			reviveCount = 666;
			break;
		case KEY_KP_6:
			health = -10000.0f; lastChance = true;
			break;
		case KEY_KP_PLUS:
			health = MAXHEALTH;
			break;
		case KEY_KP_PERIOD:
			speedy = !speedy;
			break;
		case KEY_KP_ENTER:
			firstPerson = !firstPerson;
			break;
		case KEY_KP_1:
			SendEvent(Gameplay::E_CUTSCENE_END);
			break;
		}
	}
}

void Player::OnCheatWindowEvent(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = dynamic_cast<UIElement*>(eventData["Element"].GetPtr());
	if (source) 
	{
		if (source->GetTypeName() == "Text")
		{
			source->SetEnabled(false);
		}
		if (source->HasTag("cheatWindowButton")) 
		{
			if (source->GetName() == "closeButton")
			{
				cheatWindow->SetVisible(false);
			}
			else if (source->GetName() == "winButton")
			{
				reviveCount = 666;
			}
			else if (source->GetName() == "dieButton")
			{
				health = -10000.0f; lastChance = true;
			}
			else if (source->GetName() == "healButton")
			{
				health = MAXHEALTH;
			}
			else if (source->GetName() == "hoverButton")
			{
				hovering = !hovering;
			}
			else if (source->GetName() == "fastButton")
			{
				speedy = !speedy;
			}
			else if (source->GetName() == "missileButton")
			{
				Missile::MakeMissile(scene, node_->GetWorldPosition() + Vector3(0.0f, 4.0f, 0.0f), node_->GetWorldRotation(), node_, node_);
			}
			else if (source->GetName() == "shieldButton")
			{
				Node* shield = scene->CreateChild("shield");
				shield->SetWorldPosition(node_->GetWorldPosition());
				TempShield* shieldComponent = shield->CreateComponent<TempShield>();
				shieldComponent->owner = node_;
			}
			else if (source->GetName() == "firstPersonButton")
			{
				firstPerson = !firstPerson;
			}
			else if (source->GetName() == "skipCutsceneButton")
			{
				SendEvent(Gameplay::E_CUTSCENE_END);
			}
		}
	}
}

void Player::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
}

void Player::OnCutsceneEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == Gameplay::E_CUTSCENE_START)
	{
		ChangeState(STATE_PUPPET);
	}
	else if (eventType == Gameplay::E_CUTSCENE_END)
	{
		ChangeState(STATE_DEFAULT);
	}
}

void Player::FixedUpdate(float timeStep)
{	
	if (state != STATE_WIN && state != STATE_DEAD)
	{
		modelNode->SetWorldPosition(node_->GetWorldPosition());
		modelNode->SetWorldRotation(modelNode->GetWorldRotation().Slerp(node_->GetWorldRotation() * Quaternion(-90.0f, Vector3::UP), 0.25f));
	}
	
	if ((input->GetKeyDown(KEY_KP_0) || hovering) && cheating)
	{
		actor->onGround = true;
		actor->gravity = false;
	}
	else
	{
		actor->gravity = true;
	}
	
	if (game->winState == 1)
	{
		ChangeState(STATE_WIN);
	}
	bloodEmitter->ApplyEffect();
	if (hurtTimer > 0.0f)
	{
		hurtTimer -= timeStep;
		if (hurtTimer <= 0.0f)
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
	HandleNearestCorpse();

	if (reviveCooldown > 0) reviveCooldown = Max(0.0f, reviveCooldown - timeStep);

	stateTimer += timeStep;
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

	if (state != STATE_WIN && state != STATE_DEAD && state != STATE_PUPPET) 
	{
		HandleCamera(timeStep);
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
			if (other->HasTag("instant_kill"))
			{
				health = -1000;
				lastChance = true;
			}
			else
			{
				health -= floor(22 * Settings::GetDifficulty());
			}
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
			game->FlashScreen(Color(1.0f, 1.0f, 1.0f, 0.4f), 0.005f);
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

			soundSource->Play("Sounds/itm_bonus.wav", false);
		}
	}
}

void Player::Hurt(Node* source, int amount)
{
	if (state != STATE_WIN && hurtTimer <= 0.0f && state != STATE_SLIDE && (lastState != STATE_DROWN || stateTimer > 3.5f) && state != STATE_DROWN) 
	{
		health -= amount * Settings::GetDifficulty();
		bloodEmitter->SetEmitting(true);
		hurtTimer = 0.5f;
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
	Node* proj = (Node*)eventData[Projectile::P_PERPETRATOR].GetPtr();
	Node* victim = (Node*)eventData[Projectile::P_VICTIM].GetPtr();
	int damage = eventData[Projectile::P_DAMAGE].GetInt();
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

void Player::HandleCamera(const float timeStep)
{
	const Vector3 worldPos = body->GetPosition();

	//Yaw rotation via pivot
	if (state != STATE_DROWN) pivot->SetWorldPosition(body->GetPosition());
	float theta = input->GetMouseMoveX() * Settings::GetMouseSensitivity();
	theta += timeStep * 1000.0f * Settings::GetMouseSensitivity()
		* (Settings::GetActionValue(ActionType::CAM_RIGHT) - Settings::GetActionValue(ActionType::CAM_LEFT));
	pivot->Rotate(Quaternion(theta, Vector3::UP));

	//Cheaty first person mode for fun
	if (firstPerson)
	{
		modelNode->SetEnabled(false);
		cameraNode->SetWorldPosition(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f));
		cameraNode->SetWorldRotation(pivot->GetWorldRotation());
		return;
	}
	else if (state != STATE_DROWN)
	{
		modelNode->SetEnabled(true);
	}

	//Move camera above obstacles
	const Vector3 desiredPosition = (pivot->GetWorldTransform() * cameraOffset).Translation();
	Vector3 optimalPosition = desiredPosition;

	PhysicsRaycastResult cameraDownCast;
	const Ray ray = Ray(Vector3(cameraNode->GetWorldPosition().x_, MAX_CAMERA_HEIGHT, cameraNode->GetWorldPosition().z_), Vector3::DOWN);
	//When the camera is below the map geometry, a ray cast down from the heavens will strike at a point above the camera
	physworld->SphereCast(cameraDownCast, ray, 0.5f, fabs(MAX_CAMERA_HEIGHT - worldPos.y_) + 5.0f, 2U);
	//The further up the obstacle is, the higher the camera must go above it to see the player
	const float peekOffset = 5.0f + (0.1 * fabs(cameraDownCast.position_.y_ - pivot->GetPosition().y_));
	if (cameraDownCast.body_ && cameraDownCast.position_.y_ > desiredPosition.y_ - peekOffset)
	{
		optimalPosition.y_ = cameraDownCast.position_.y_ + peekOffset;
	}
	optimalPosition.y_ = Min(optimalPosition.y_, MAX_CAMERA_HEIGHT);
	
	//Smooth movement
	const Vector3 toOptimal = (optimalPosition - cameraNode->GetWorldPosition());
	if (toOptimal.x_ != 0.0f || toOptimal.y_ != 0.0f || toOptimal.z_ != 0.0f)
	{
		float transitionSpeed = Min(0.15f + toOptimal.Length() * 0.01, 0.9f);
		if (input->GetMouseMoveX() > 8)
		{
			transitionSpeed = 1.0f;
		}
		cameraNode->Translate(toOptimal * transitionSpeed, TS_WORLD);
		//cameraNode->SetWorldPosition(optimalPosition);
	}

	//Make camera look at player's feet
	Quaternion newAngle = Quaternion();
	newAngle.FromLookRotation((worldPos - cameraNode->GetWorldPosition()).Normalized());
	cameraNode->SetWorldRotation(newAngle);

	//Pitch rotation
	float mvy = input->GetMouseMoveY();
	if (Settings::IsActionDown(ActionType::CAM_UP)) mvy -= timeStep * 1440.0f;
	if (Settings::IsActionDown(ActionType::CAM_DOWN)) mvy += timeStep * 1440.0f;
	if (Settings::IsMouseInverted()) mvy = -mvy;
	cameraPitch = Clamp(cameraPitch + (mvy * Settings::GetMouseSensitivity() * 0.25f), -15.0f, 15.0f);
	cameraNode->Rotate(Quaternion(cameraPitch, Vector3::RIGHT), TS_LOCAL);
}

void Player::HandleShadow()
{
	if (actor->downCast.body_)
	{
		if (!actor->onGround && actor->downCast.distance_ > 0.8f)
		{
			dropShadow->SetEnabled(true);
			dropShadow->SetWorldPosition(actor->downCast.position_ + Vector3(0.0f, 0.1f, 0.0f));
			Quaternion q = Quaternion();
			q.FromLookRotation(actor->downCast.normal_);
			dropShadow->SetRotation(q);
			StaticModel* sm = dropShadow->GetComponent<StaticModel>();
			sm->GetMaterial(0)->SetShaderParameter("MatDiffColor", Vector4(1.0f, 1.0f, 1.0f, Min(1.0f, actor->downCast.distance_ * 0.8f)));
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
			actor->maxSpeed = slideSpeed;
			slideDirection = node_->GetWorldDirection();
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
			HandleCamera(0.0f);
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
		actor->maxSpeed = walkSpeed;
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
	if (speedy)
		actor->maxSpeed = 413.0f;
	else
		actor->maxSpeed = walkSpeed;

	Vector3 keyVec = Vector3(
		Settings::GetActionValue(ActionType::RIGHT) - Settings::GetActionValue(ActionType::LEFT),
		0.0f,
		Settings::GetActionValue(ActionType::FORWARD) - Settings::GetActionValue(ActionType::BACK)
	);
	game->gunPriest->debugRenderer->AddLine(node_->GetWorldPosition() + Vector3::UP, node_->GetWorldPosition() + Vector3::UP + keyVec * Settings::GetAction(ActionType::FORWARD).joyBinding->getValue(), 0xFF00FF, false);
	actor->SetInputVec(pivot->GetWorldRotation() * keyVec);
	
	if (Settings::IsActionDown(ActionType::JUMP))
		actor->Jump();

	//Decide angle
	if (actor->input != Vector3::ZERO)
	{
		Quaternion newRotation;
		newRotation.FromLookRotation(actor->input);
		node_->SetWorldRotation(newRotation);
	}

	//Ground detection is done manually for animations because the physics engine is retarded.
	if (!groundDetector->IsActive()) groundDetector->Activate();
	groundDetector->SetPosition(body->GetPosition());

	PODVector<RigidBody*> grounds;
	bool detectedGround = false;
	physworld->GetRigidBodies(grounds, groundDetector);
	for (RigidBody* body : grounds)
	{
		if (body->GetCollisionLayer() & 2)
		{
			detectedGround = true;
			break;
		}
	}	

	//Select Animation
	if (!detectedGround)
	{
		if ((lastState == STATE_DROWN && stateTimer < 0.5f)
			|| Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD)
		{
			animController->PlayExclusive(DROWN_ANIM, 0, true, 0.2f);
		}
		else 
		{
			animController->PlayExclusive(JUMP_ANIM, 0, false, 0.2f);
		}
	}
	else
	{
		if (actor->input != Vector3::ZERO)
		{
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
			hailTimer = 0.0f;
		}
		else
		{
			hailTimer += timeStep;
			if (hailTimer >= 10.0f)
			{
				hailTimer = 0.0f;
				animController->PlayExclusive(HAIL_ANIM, 0, false, 0.2f);
			}
			if (!animController->IsPlaying(HAIL_ANIM) || animController->IsAtEnd(HAIL_ANIM))
			{
				animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
			}
		}
	}

	//Reviving
	if (Settings::IsActionDown(ActionType::REVIVE) && reviveCooldown <= 0.0f)
	{
		animController->Play(REVIVE_ANIM, 128, false, 0.2f);
		animController->SetStartBone(REVIVE_ANIM, "torso");
		animController->SetAutoFade(REVIVE_ANIM, 0.2f);
		revived = false;
		reviveCooldown = reviveCooldownMax;
		//The actual reviving is done in HandleNearestCorpse()
	}

	//Sliding
	if (Settings::IsActionDown(ActionType::SLIDE) && actor->onGround && stateTimer > 0.5f)
	{
		animController->Stop(REVIVE_ANIM, 0.2f);
		ChangeState(STATE_SLIDE);
	}

	actor->Move(timeStep);
}

void Player::ST_Slide(float timeStep)
{
	animController->PlayExclusive(SLIDE_ANIM, 0, false, 0.2f);

	actor->SetInputVec(slideDirection);
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
	actor->SetInputVec(Vector3::ZERO);
	actor->Move(timeStep);
	animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
}

void Player::ST_Drown(float timeStep)
{
	animController->PlayExclusive(DROWN_ANIM, 0, true, 0.2f);
	if (stateTimer > 0.25f)
	{
		switch (drownPhase) 
		{
		case 0:
			beamNode = Zeus::MakeLightBeam(scene, node_->GetWorldPosition(), 1);
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
			Zeus::MakeLightBeam(scene, node_->GetWorldPosition(), 2);
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
		actor->SetInputVec(Vector3::ZERO);
		actor->Move(timeStep);
	}
	else
	{
		body->SetLinearVelocity(Vector3::ZERO);
		pivot->SetWorldPosition(node_->GetWorldPosition());
	}
}

void Player::HandleNearestCorpse()
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
	const Matrix3x4 invCamTrans = cameraNode->GetWorldTransform().Inverse();
	Quaternion q = Quaternion();
	Vector3 to;
	if (nearestCorpse)
	{
		to = invCamTrans * Vector3(nearestCorpse->GetNode()->GetWorldPosition().x_, 0.0f, nearestCorpse->GetNode()->GetWorldPosition().z_);
	}
	else
	{
		to = invCamTrans * Vector3(startingPosition.x_, 0.0f, startingPosition.z_); //Point to start of level when no enemies are left
	}
	const Vector3 from = invCamTrans * Vector3(node_->GetWorldPosition().x_, 0.0f, node_->GetWorldPosition().z_);
	q.FromLookRotation((from - to).Normalized());
	game->compassScene->compassRotation = q.EulerAngles().y_;

	//Handle revival and floating arrow
	if (nearestCorpse)
	{
		const float distance = (nearestCorpse->GetNode()->GetWorldPosition() - node_->GetWorldPosition()).Length();
		if (distance < 6.0f)
		{
			arrowNode->SetEnabled(true);
			arrowNode->SetWorldPosition(nearestCorpse->GetNode()->GetWorldPosition() + Vector3(0.0f, 5.0f, 0.0f));
			if (!revived && reviveCooldown > reviveCooldownMax - 0.5f && reviveCooldown <= reviveCooldownMax - 0.25f)
			{
				revived = true;
				Zeus::MakeLightBeam(scene, nearestCorpse->GetNode()->GetWorldPosition(), 0);
				nearestCorpse->Revive();
				reviveCount += 1;
				if (Random(0, 3000) == 42)
				{
					soundSource->Play("Sounds/ply_wololo.wav", true);
				}
				else
				{
					soundSource->Play("Sounds/ply_revive.wav", true);
				}
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
}

Player::~Player()
{
}
