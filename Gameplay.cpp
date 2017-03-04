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
#include <Urho3D/Graphics/StaticModelGroup.h>

#include <iostream>

#include "Player.h"
#include "Cross.h"
#include "NPC.h"

using namespace Urho3D;

Gameplay::Gameplay(Context* context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	flashSpeed = 0.0f;
	flashColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
	crossCount = 0;
}

void Gameplay::RegisterObject(Context* context)
{
	context->RegisterFactory<Gameplay>();
}

void Gameplay::Start()
{
	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	scene_ = SharedPtr<Scene>(GetScene());
	renderer = WeakPtr<Renderer>(GetSubsystem<Renderer>());

	input->SetMouseGrabbed(true);

	GetSettings();
	SetupGame();

	viewport = SharedPtr<Viewport>(new Viewport(context_, scene_, cameraNode->GetComponent<Camera>()));
	renderer->SetViewport(0, viewport);
	renderer->GetDefaultRenderPath()->Append(cache->GetResource<XMLFile>("PostProcess/screenflash.xml"));
	renderer->GetDefaultRenderPath()->SetShaderParameter("FlashColor", Color(0.0f, 0.0f, 0.0f, 0.0f));

	debugHud = engine_->CreateDebugHud();
#if _DEBUG
	debugHud->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
	debugHud->SetMode(DEBUGHUD_SHOW_PROFILER);
#endif
	
	MakeHUD(scene_->GetVar("Screen Width").GetInt(), scene_->GetVar("Screen Height").GetInt());
}

void Gameplay::SetupGame()
{
	playerNode = scene_->GetChild("player");
	player = playerNode->CreateComponent<Player>();
	cameraNode = playerNode->GetChild("camera");
	camera = cameraNode->GetComponent<Camera>();
	player->input = input;

	skybox = scene_->GetChild("skybox");

	SetupCrosses();

	//Spawn NPCs
	PODVector<Node*> npcs;
	scene_->GetChildrenWithTag(npcs, "npc", true);
	for (PODVector<Node*>::Iterator i = npcs.Begin(); i != npcs.End(); ++i)
	{
		Node* npc = (Node*)*i;
		int model = 0;
		int skin = floor(Random() * 3);
		String path = "Npcs/model";
		path += model;
		path += "/skin";
		path += skin;
		path += ".xml";
		Material* mat = cache->GetResource<Material>(path);
		npc->GetChild("model")->GetComponent<AnimatedModel>()->SetMaterial(mat);
		npc->SetVar("MODEL", model);

		npc->CreateComponent<NPC>();
	}
}

void Gameplay::SetupCrosses()
{
	//Spawn Crosses
	Node* crosses = scene_->GetChild("crosses");
	PODVector<Node*> spawners;
	scene_->GetChildrenWithTag(spawners, "spawner", true);
	for (PODVector<Node*>::Iterator i = spawners.Begin(); i != spawners.End(); ++i)
	{
		Node* child = (Node*)*i;
		String type = child->GetVar("spawnertype").GetString();
		float distance = child->GetVar("spawnerdistance").GetFloat();
		if (distance == 0.0f) distance = 3.0f;
		//Spawn crosses in the described formation, raycasting downwards to ensure that they're all on the ground.
		if (type == "ring")
		{
			for (int j = 0; j < 5; ++j)
			{
				Node* cross = crosses->CreateChild();
				cross->SetName("cross");
				cross->AddTag("cross");
				Vector3 position = child->GetWorldPosition() + (Quaternion(72.0f * j, Vector3::UP) * Vector3::FORWARD * distance);
				SetOnFloor(cross, position);
			}
		}
		else if (type == "xline")
		{
			for (int j = 0; j < 5; ++j)
			{
				Node* cross = crosses->CreateChild();
				cross->SetName("cross");
				cross->AddTag("cross");
				Vector3 position = child->GetWorldPosition() + Vector3((-distance * 2.5f) + (j * distance), 0.0f, 0.0f);
				SetOnFloor(cross, position);
			}
		}
		else if (type == "zline")
		{
			for (int j = 0; j < 5; ++j)
			{
				Node* cross = crosses->CreateChild();
				cross->SetName("cross");
				cross->AddTag("cross");
				Vector3 position = child->GetWorldPosition() + Vector3(0.0f, 0.0f, (-distance * 2.0f) + (j * distance));
				SetOnFloor(cross, position);
			}
		}
		child->Remove();
	}

	//Setup Crosses
	StaticModelGroup* crossGroup = crosses->GetComponent<StaticModelGroup>();
	crossCount = crosses->GetNumChildren();
	for (unsigned int i = 0; i < crosses->GetNumChildren(); ++i)
	{
		Node* child = crosses->GetChild(i);
		child->RemoveAllComponents(); //They all have static models just to see them in the editor. We need to add them to the staticmodelgroup.
		String name = child->GetName();
		if (name == "cross")
		{
			child->CreateComponent<Cross>();
			crossGroup->AddInstanceNode(child);
		}
	}

	//Make the crosses glowy
	SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
	colorAnimation->SetKeyFrame(0.0f, Color::BLACK);
	colorAnimation->SetKeyFrame(1.0f, Color::WHITE);
	colorAnimation->SetKeyFrame(2.0f, Color::BLACK);
	crossGroup->GetMaterial(0U)->SetShaderParameterAnimation("MatEmissiveColor", colorAnimation, WM_LOOP, 1.0f);
}

void Gameplay::FixedUpdate(float timeStep)
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
	renderer->GetDefaultRenderPath()->SetShaderParameter("FlashColor", flashColor);
}

void Gameplay::UpdateHUD(float timeStep)
{
	GetNextFrame(crossIcon, 128, 128, 60);
	String s = "";
	s += playerNode->GetVar("Cross Count").GetInt();
	s += "/";
	s += crossCount;
	crossCounter->SetText(s);
}

Gameplay::~Gameplay()
{
}

void Gameplay::GetSettings()
{
	scene_->SetVar("MOUSE SENSITIVITY", 0.25f);
	scene_->SetVar("FORWARD KEY", KEY_W);
	scene_->SetVar("BACKWARD KEY", KEY_S);
	scene_->SetVar("RIGHT KEY", KEY_D);
	scene_->SetVar("LEFT KEY", KEY_A);
	scene_->SetVar("JUMP KEY", KEY_SPACE);
}

void Gameplay::MakeHUD(int width, int height)
{
	UI* ui = GetSubsystem<UI>();

	ui->LoadLayout(cache->GetResource<XMLFile>("UI/HUDLayout.xml"));

	text = new Text(context_);
	text->SetText("GUN PRIEST ALPHA : WWW.BITENDOSOFTWARE.COM");
	text->SetFont("Fonts/Anonymous Pro.ttf", 12);
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_TOP);
	ui->GetRoot()->AddChild(text);

	crosshair = new Sprite(context_);
	crosshair->SetTexture(cache->GetResource<Texture2D>("Textures/crosshair.png"));
	crosshair->SetFullImageRect();
	crosshair->SetSize(16, 16);
	crosshair->SetHotSpot(16, 16);
	crosshair->SetBlendMode(BLEND_ADDALPHA);
	crosshair->SetHorizontalAlignment(HA_CENTER);
	crosshair->SetVerticalAlignment(VA_CENTER);
	ui->GetRoot()->AddChild(crosshair);

	crossIcon = new Sprite(context_);
	crossIcon->SetTexture(cache->GetResource<Texture2D>("Textures/cross_ui.png"));
	crossIcon->SetImageRect(IntRect(0, 0, 128, 128));
	crossIcon->SetSize(128, 128);
	crossIcon->SetHotSpot(128, 128);
	crossIcon->SetBlendMode(BLEND_ALPHA);
	crossIcon->SetHorizontalAlignment(HA_LEFT);
	crossIcon->SetVerticalAlignment(VA_TOP);
	ui->GetRoot()->AddChild(crossIcon);
	crossIcon->SetPosition(width * 0.9f, height * 0.9f);

	crossCounter = new Text(context_);
	crossCounter->SetText("x0");
	crossCounter->SetFont("Fonts/Anonymous Pro.ttf", 24);
	crossCounter->SetVerticalAlignment(VA_CENTER);
	crossCounter->SetHorizontalAlignment(HA_LEFT);
	crossIcon->AddChild(crossCounter);
	crossCounter->SetPosition(128.0f, 0.0f);
}

void Gameplay::FlashScreen(Color c, float spd)
{
	flashColor = c;
	flashSpeed = spd;
	renderer->GetDefaultRenderPath()->SetShaderParameter("FlashColor", c);
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

void Gameplay::SetOnFloor(Node* n, Vector3 pos)
{
	PhysicsRaycastResult result;
	scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(pos, Vector3::DOWN), 500.0f, 2);

	if (result.body_)
	{
		n->SetWorldPosition(result.position_);
	}
	else
	{
		std::cout << "A CROSS IS OUT OF BOUNDS" << std::endl;
		n->SetWorldPosition(pos);
	}
}
