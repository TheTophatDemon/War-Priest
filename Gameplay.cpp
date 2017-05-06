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

using namespace Urho3D;

Gameplay::Gameplay(Context* context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	flashSpeed = 0.0f;
	flashColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
	oldHealth = 100.0f;
	initialized = false;

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
	SetGlobalVar("PROJECTILE COUNT", 0);
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
	camera->SetFov(scene_->GetGlobalVar("CAMERA FOV").GetFloat());
	Node* pivot = scene_->CreateChild();
	cameraNode->SetParent(pivot);

	player->pivot = pivot;
	player->input = input;
	playerNode->AddComponent(player, 666, LOCAL);

	viewport->SetScene(scene_);
	viewport->SetCamera(camera);

	skybox = scene_->GetChild("skybox");

	//SetupNPC();
	SetupEnemy();
	SetupProps();
	loseText->SetVisible(false);
	viewport->GetRenderPath()->SetShaderParameter("State", 0.0f);
	initialized = true;
}

void Gameplay::FixedUpdate(float timeStep)
{
	if (IsEnabled()) 
	{
		audio->SetMasterGain("VOICE", scene_->GetGlobalVar("VOICE VOLUME").GetFloat());
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
		if (loseTimer > 0)
		{
			viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
			loseTimer -= 1;
			if (loseTimer <= 0)
			{
				gunPriest->StartGame();
			}
		}
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
	scene_->SetGlobalVar("MOUSE SENSITIVITY", 0.25f);
	scene_->SetGlobalVar("FORWARD KEY", KEY_W);
	scene_->SetGlobalVar("BACKWARD KEY", KEY_S);
	scene_->SetGlobalVar("RIGHT KEY", KEY_D);
	scene_->SetGlobalVar("LEFT KEY", KEY_A);
	scene_->SetGlobalVar("JUMP KEY", KEY_SPACE);
	scene_->SetGlobalVar("VOICE VOLUME", 0.5f);
	scene_->SetGlobalVar("CAMERA FOV", 70.0f);
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

void Gameplay::SetOnFloor(Node* n, Vector3 pos, float offset)
{
	PhysicsRaycastResult result;
	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(pos, Vector3::DOWN), 500.0f, 2);

	if (result.body_)
	{
		n->SetWorldPosition(result.position_ + Vector3(0.0f, offset, 0.0f));
	}
	else
	{
		n->SetWorldPosition(pos);
	}
}

void Gameplay::Lose()
{
	if (loseTimer == 0) 
	{
		loseText->SetVisible(true);
		viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
		loseTimer = 250;
	}
}

void Gameplay::SetupEnemy()
{
	PODVector<Node*> enemies;
	scene_->GetChildrenWithTag(enemies, "enemy", true);
	for (PODVector<Node*>::Iterator i = enemies.Begin(); i != enemies.End(); ++i)
	{
		Node* n = (Node*)*i;
		if (n)
		{
			Matrix3x4 t = n->GetWorldTransform();
			n->LoadXML(cache->GetResource<XMLFile>("Objects/pyropastor.xml")->GetRoot());
			n->SetWorldTransform(t.Translation(), t.Rotation(), t.Scale());
			SetOnFloor(n, n->GetWorldPosition(), 0.1f);
			n->CreateComponent<PyroPastor>();
		}
	}
}

void Gameplay::SetupProps()
{
	RigidBody* mapBody = mapNode->GetComponent<RigidBody>();
	float mapScale = mapNode->GetScale().x_;
	mapBody->DisableMassUpdate();

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

	mapBody->EnableMassUpdate();
}



Node* Gameplay::MakeLightBeam(Vector3 position)
{
	Vector3 beamSize = Vector3(3.5f, 64.0f, 3.5f);
	Vector3 closed = Vector3(0.0f, beamSize.y_, 0.0f);

	Node* lightColumn = scene_->CreateChild();
	lightColumn->SetPosition(position);

	TempEffect* t = lightColumn->CreateComponent<TempEffect>();
	t->life = 0.75f;

	Node* smoker = scene_->CreateChild();
	smoker->SetPosition(position);
	TempEffect* t2 = smoker->CreateComponent<TempEffect>();
	t2->life = 1.5f;

	ParticleEmitter* emitter = smoker->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(cache->GetResource<ParticleEffect>("Particles/smoke.xml"));
	emitter->SetScaled(false);

	SharedPtr<ValueAnimation> emitAnim(new ValueAnimation(context_));
	emitAnim->SetKeyFrame(0.0f, true);
	emitAnim->SetKeyFrame(0.1f, false);
	emitter->SetAttributeAnimation("Is Emitting", emitAnim, WM_CLAMP, 1.0f);

	StaticModel* model = lightColumn->CreateComponent<StaticModel>();
	model->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/lightbeam.xml"));
	
	SharedPtr<ObjectAnimation> columnAnimation(new ObjectAnimation(context_));

	SharedPtr<ValueAnimation> diffAnim(new ValueAnimation(context_));
	diffAnim->SetKeyFrame(0.0f, Color::WHITE);
	diffAnim->SetKeyFrame(0.25f, Color::WHITE);
	diffAnim->SetKeyFrame(0.75f, Color(0.0f, 0.0f, 0.0f, 0.0f));
	model->GetMaterial()->SetShaderParameterAnimation("MatDiffColor", diffAnim, WM_CLAMP, 1.0f);

	SharedPtr<ValueAnimation> scaleAnim(new ValueAnimation(context_));
	scaleAnim->SetKeyFrame(0.0f, closed);
	scaleAnim->SetKeyFrame(0.25f, beamSize);
	scaleAnim->SetKeyFrame(0.75f, closed);
	columnAnimation->AddAttributeAnimation("Scale", scaleAnim, WM_CLAMP, 1.0f);

	lightColumn->SetObjectAnimation(columnAnimation);
	return lightColumn;
}