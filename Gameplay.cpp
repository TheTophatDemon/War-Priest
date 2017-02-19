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
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/StaticModelGroup.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include "Player.h"
#include "Cross.h"

using namespace Urho3D;

Gameplay::Gameplay(Context* context) : LogicComponent(context)
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
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

	debugHud = engine_->CreateDebugHud();
#if _DEBUG
	debugHud->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
	debugHud->SetMode(DEBUGHUD_SHOW_PROFILER);
#endif
	
	MakeHUD();
}

void Gameplay::SetupGame()
{
	playerNode = scene_->GetChild("player");
	player = playerNode->CreateComponent<Player>();
	cameraNode = playerNode->GetChild("camera");
	camera = cameraNode->GetComponent<Camera>();
	player->input = input;

	//Setup Crosses
	Node* crosses = scene_->GetChild("crosses");
	StaticModelGroup* crossGroup = crosses->GetComponent<StaticModelGroup>();
	for (unsigned int i = 0; i < crosses->GetNumChildren(); ++i)
	{
		Node* child = crosses->GetChild(i);
		child->RemoveAllComponents(); //They all have static models just to see them in the editor. We need to add them to the staticmodelgroup.
		child->CreateComponent<Cross>();
		crossGroup->AddInstanceNode(child);
	}
	SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
	colorAnimation->SetKeyFrame(0.0f, Color::BLACK);
	colorAnimation->SetKeyFrame(1.0f, Color::WHITE);
	colorAnimation->SetKeyFrame(2.0f, Color::BLACK);
	crossGroup->GetMaterial(0U)->SetShaderParameterAnimation("MatEmissiveColor", colorAnimation, WM_LOOP, 1.0f);
}

void Gameplay::FixedUpdate(float timeStep)
{
	
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

void Gameplay::MakeHUD()
{
	UI* ui = GetSubsystem<UI>();

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
}
