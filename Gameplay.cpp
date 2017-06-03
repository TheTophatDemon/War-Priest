#include "Gameplay.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D\Physics\PhysicsUtils.h>
#include <Urho3D\Resource\ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/View.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/StaticModelGroup.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/ObjectAnimation.h>


#include <iostream>

#include "Player.h"
#include "Boulder.h"
#include "Enemy.h"
#include "PyroPastor.h"
#include "Projectile.h"
#include "TempEffect.h"
#include "GunPriest.h"
#include "God.h"
#include "PostalPope.h"
#include "Water.h"

using namespace Urho3D;

Gameplay::Gameplay(Context* context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	flashSpeed = 0.0f;
	flashColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
	oldHealth = 100.0f;
	initialized = false;
	restartTimer = 0;

	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	renderer = GetSubsystem<Renderer>();
	audio = GetSubsystem<Audio>();

	MakeHUD();
}

void Gameplay::RegisterObject(Context* context)
{
	context->RegisterFactory<Gameplay>();
}

void Gameplay::Start()
{
	viewport = renderer->GetViewport(0);
	scene_ = SharedPtr<Scene>(GetScene());

	GetSettings();
	input->SetMouseGrabbed(true);
	ourUI->SetEnabledRecursive(true);
	ourUI->SetVisible(true);
	viewport->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward.xml"));
	viewport->GetRenderPath()->SetShaderParameter("State", 0.0f); //Always use decimal
	input->SetMouseVisible(false);
	scene_->SetUpdateEnabled(true);
}

void Gameplay::SetupGame()
{
	SetGlobalVar("PROJECTILE COUNT", 0);
	winState = 0;
	restartTimer = 0;
	mapNode = scene_->GetChild("map");

	//Setup Player
	playerNode = scene_->GetChild("player");
	Matrix3x4 trans = playerNode->GetWorldTransform();
	playerNode->LoadXML(cache->GetResource<XMLFile>("Objects/player.xml")->GetRoot());
	playerNode->SetWorldTransform(trans.Translation(), trans.Rotation(), trans.Scale());
	player = new Player(context_);
	audio->SetListener(playerNode->GetComponent<SoundListener>());

	//Setup Camera
	cameraNode = scene_->CreateChild();
	cameraNode->SetPosition(Vector3(0.0f, 12.0f, -12.0f));
	camera = cameraNode->CreateComponent<Camera>();
	camera->SetFov(sCameraFov);

	playerNode->AddComponent(player, 666, LOCAL);

	viewport->SetScene(scene_);
	viewport->SetCamera(camera);

	skybox = scene_->GetChild("skybox");
	water = scene_->GetChild("water");
	water->CreateComponent<Water>();
	
	//Setup Lifts
	PODVector<Node*> lifts;
	scene_->GetChildrenWithTag(lifts, "lift", true);
	for (PODVector<Node*>::Iterator i = lifts.Begin(); i != lifts.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			Vector3 movement = n->GetVar("movement").GetVector3();
			float restSpeed = n->GetVar("restTime").GetFloat(); if (restSpeed == 0.0f) restSpeed = 1.0f;
			float speed = n->GetVar("speed").GetFloat(); if (speed == 0.0f) speed = 2.0f;
			float rotSpeed = n->GetVar("rotateSpeed").GetFloat();
			n->AddComponent(Lift::MakeLiftComponent(context_, movement, restSpeed, speed, rotSpeed), 1200, LOCAL);
		}
	}
	//Setup Medkits
	SharedPtr<ValueAnimation> va(new ValueAnimation(context_));
	va->SetKeyFrame(0.0f, Color::BLACK);
	va->SetKeyFrame(1.0f, Color::GRAY);
	va->SetKeyFrame(2.0f, Color::BLACK);
	cache->GetResource<Material>("Materials/skins/medkit_skin.xml")->SetShaderParameterAnimation("MatEmissiveColor", va, WM_LOOP, 1.0f);

	PODVector<Node*> medkits;
	scene_->GetChildrenWithTag(medkits, "medkit", true);
	for (PODVector<Node*>::Iterator i = medkits.Begin(); i != medkits.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			RigidBody* rb = n->CreateComponent<RigidBody>();
			rb->SetCollisionLayer(32);
			rb->SetCollisionMask(128);
			rb->SetTrigger(true);
			StaticModel* sm = n->GetComponent<StaticModel>();
			CollisionShape* cs = n->CreateComponent<CollisionShape>();
			cs->SetBox(sm->GetBoundingBox().Size(),sm->GetBoundingBox().Center(), Quaternion::IDENTITY);
		}
	}

	SetupEnemy();
	SetupProps();

	loseText->SetVisible(false);
	winText->SetVisible(false);
	viewport->GetRenderPath()->SetShaderParameter("State", 0.0f);
	initialized = true;
}

void Gameplay::FixedUpdate(float timeStep)
{
	if (IsEnabled()) 
	{
		UpdateHUD(timeStep);


		if (skybox)
		{
			skybox->Rotate(Quaternion(timeStep * 5.0f, Vector3::UP));
		}
		if (flashColor.a_ > 0.0f)
		{
			flashColor.a_ -= flashSpeed * timeStep * 100.0f;
			if (flashColor.a_ < 0.0f) flashColor.a_ = 0.0f;
		}
		viewport->GetRenderPath()->SetShaderParameter("FlashColor", flashColor);


		if ((player->reviveCount >= enemyCount && enemyCount > 0) || input->GetKeyDown(KEY_L))
		{
			Win();
		}
		if (restartTimer > 0)
		{
			restartTimer -= 1;
			if (restartTimer <= 0)
			{
				initialized = false;
				gunPriest->ChangeState(GunPriest::STATE_TITLE);
			}
		}
		if (winState == -1)
			viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
	}
}

void Gameplay::UpdateHUD(float timeStep)
{
	//Count things
	PODVector<Node*> projs;
	scene_->GetChildrenWithTag(projs, "projectile", true);
	SetGlobalVar("PROJECTILE COUNT", projs.Size());

	if (player) 
	{
		if (oldHealth != player->health)
		{
			float diff = (oldHealth - player->health);
			oldHealth -= diff * 0.25f;
			if (fabs(diff) < 0.1f) oldHealth = player->health;
		}
		healthMeter->SetSize(floor((oldHealth / 100.0f) * 628.0f), 52);
		projectileCounter->SetText("PROJECTILE: " + String(GetGlobalVar("PROJECTILE COUNT").GetInt()));
	}
}

Gameplay::~Gameplay()
{
}

void Gameplay::GetSettings()
{
	sMouseSensitivity = 0.25f;
	sCameraFov = 70.0f;
	sKeyForward = KEY_W;
	sKeyBackward = KEY_S;
	sKeyRight = KEY_D;
	sKeyLeft = KEY_A;
	sKeyJump = KEY_SPACE;
}

void Gameplay::MakeHUD()
{
	UI* ui = GetSubsystem<UI>();
	ui->GetRoot()->LoadChildXML(cache->GetResource<XMLFile>("UI/HUDLayout.xml")->GetRoot());
	ourUI = ui->GetRoot()->GetChild("hud", true);
	int width = ourUI->GetWidth();
	int height = ourUI->GetHeight();

	text = new Text(context_);
	text->SetText("WAR PRIEST ALPHA : WWW.BITENDOSOFTWARE.COM");
	text->SetFont("Fonts/Anonymous Pro.ttf", 12);
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_TOP);
	ourUI->AddChild(text);

	loseText = new Text(context_);
	loseText->SetText("YOU HAVE FAILED. THE LORD FROWNS UPON YOU.");
	loseText->SetFont("Fonts/Anonymous Pro.ttf", 24);
	loseText->SetHorizontalAlignment(HA_CENTER);
	loseText->SetVerticalAlignment(VA_CENTER);
	ourUI->AddChild(loseText);
	loseText->SetVisible(false);

	winText = new Text(context_);
	winText->SetText("MISSION COMPLETE");
	winText->SetFont("Fonts/Anonymous Pro.ttf", 24);
	winText->SetHorizontalAlignment(HA_CENTER);
	winText->SetVerticalAlignment(VA_CENTER);
	ourUI->AddChild(winText);
	winText->SetVisible(false);

	projectileCounter = new Text(context_);
	projectileCounter->SetText("PROJECTILE: " + String(GetGlobalVar("PROJECTILE COUNT").GetInt()));
	projectileCounter->SetFont("Fonts/Anonymous Pro.ttf", 12);
	projectileCounter->SetHorizontalAlignment(HA_LEFT);
	projectileCounter->SetVerticalAlignment(VA_CENTER);
	ourUI->AddChild(projectileCounter);

	healthMeter = (Sprite*)ourUI->GetChild("healthbar", true)->GetChild(0);

	ourUI->SetEnabled(false);
	ourUI->SetVisible(false);
}

void Gameplay::FlashScreen(Color c, float spd)
{
	flashColor = c;
	flashSpeed = spd;
	viewport->GetRenderPath()->SetShaderParameter("FlashColor", c);
}

void Gameplay::GetNextFrame(Sprite* spr, int cellWidth, int cellHeight, int cellCount)
{
	IntRect rect = spr->GetImageRect();
	Texture* tex = spr->GetTexture();

	rect.bottom_ += cellHeight;
	rect.top_ += cellHeight;
	if (rect.bottom_ > tex->GetHeight())
	{
		rect.top_ = 0.0f;
		rect.bottom_ = cellHeight;

		rect.left_ += cellWidth;
		rect.right_ += cellHeight;
		if (rect.right_ > tex->GetWidth())
		{
			rect.left_ = 0.0f;
			rect.right_ = cellWidth;
		}
	}
	int columnCount = tex->GetWidth() / cellWidth;
	int rowCount = tex->GetHeight() / cellHeight;
	int cellIndex = (rowCount * (rect.left_ / cellWidth)) + (rect.top_ / cellHeight);
	if (cellIndex >= cellCount)
	{
		rect.top_ = 0.0f;
		rect.bottom_ = cellHeight;
		rect.left_ = 0.0f;
		rect.right_ = cellWidth;
	}

	spr->SetImageRect(rect);
}

void Gameplay::Lose()
{
	winState = -1;
	if (restartTimer == 0)
	{
		loseText->SetVisible(true);
		viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
		restartTimer = 250;
	}
}

void Gameplay::Win()
{
	winState = 1;
	if (restartTimer == 0)
	{
		winText->SetVisible(true);
		viewport->GetRenderPath()->SetShaderParameter("State", 0.0f);
		restartTimer = 250;

		Node* god = scene_->CreateChild();
		god->LoadXML(cache->GetResource<XMLFile>("Objects/god.xml")->GetRoot(), false);
		god->CreateComponent<God>();
	}
}

void Gameplay::SetupEnemy()
{
	enemyCount = 0;
	PODVector<Node*> enemies;
	scene_->GetChildrenWithTag(enemies, "enemy", true);
	for (PODVector<Node*>::Iterator i = enemies.Begin(); i != enemies.End(); ++i)
	{
		Node* n = (Node*)*i;
		if (n)
		{
			String enemyType = n->GetName();
			Matrix3x4 t = n->GetWorldTransform();
			n->LoadXML(cache->GetResource<XMLFile>("Objects/" + enemyType + ".xml")->GetRoot());
			n->SetWorldTransform(t.Translation(), t.Rotation(), t.Scale());
			if (enemyType == "pyropastor")
			{
				n->CreateComponent<PyroPastor>();
			}
			else if (enemyType == "postalpope")
			{
				n->CreateComponent<PostalPope>();
			}

			enemyCount += 1;
		}
	}
}

void Gameplay::SetupProps()
{
	float mapScale = mapNode->GetScale().x_;

	PODVector<Node*> props; //Get the existing props
	mapNode->GetChildrenWithTag(props, "prop", true);

	PODVector<Node*> instancers;
	mapNode->GetChildrenWithTag(instancers, "propInstancer", false);

	//Go through each prop and add rigid bodies
	for (PODVector<Node*>::Iterator i = props.Begin(); i != props.End(); ++i) 
	{
		Node* n = (Node*)*i;
		if (n)
		{
			StaticModel* m = n->GetComponent<StaticModel>();
			if (m)
			{
				if (!n->HasComponent<CollisionShape>()) //Generate collision shapes for those props unassigned
				{
					CollisionShape* newShape = new CollisionShape(context_);
					newShape->SetBox(m->GetBoundingBox().Size());
					n->AddComponent(newShape, 1200, LOCAL);
				}
				if (!n->GetParent()->HasTag("propInstancer")) //Add rigidbodies to uninstanced props
				{
					RigidBody* body = n->CreateComponent<RigidBody>();
					body->SetCollisionLayer(2);
					body->SetCollisionEventMode(CollisionEventMode::COLLISION_ACTIVE);
				}
			}
		}
	}

	//Find the instancing groups and remove the children's StaticMesh components and add them to the instancing groups
	for (PODVector<Node*>::Iterator i = instancers.Begin(); i != instancers.End(); ++i)
	{
		Node* n = (Node*)*i;
		if (n)
		{
			if (n->GetPosition() != Vector3::ZERO)
			{
				std::cout << "HEY! One of the instancing groups are mispositioned! " << n->GetName().CString() << std::endl;
			}
			StaticModelGroup* modelGroup = n->GetComponent<StaticModelGroup>();
			RigidBody* body = n->CreateComponent<RigidBody>();
			body->SetCollisionLayer(2);
			body->SetCollisionEventMode(CollisionEventMode::COLLISION_ACTIVE);
			body->DisableMassUpdate();

			PODVector<Node*> children;
			n->GetChildren(children);
			for (PODVector<Node*>::Iterator c = children.Begin(); c != children.End(); c++)
			{
				Node* child = (Node*)*c;
				child->RemoveComponent<StaticModel>();
				if (child->HasComponent<CollisionShape>()) //Also add children's collision shapes to parent's rigid body
				{
					SharedPtr<CollisionShape> shape = SharedPtr<CollisionShape>(child->GetComponent<CollisionShape>());
					shape->SetPosition((shape->GetPosition() * child->GetScale()) + child->GetPosition());
					shape->SetSize(shape->GetSize() * child->GetScale());
					child->RemoveComponent<CollisionShape>();
					n->AddComponent(shape, 1200, LOCAL);
				}
				modelGroup->AddInstanceNode(child);
			}

			body->EnableMassUpdate();
		}
	}
}