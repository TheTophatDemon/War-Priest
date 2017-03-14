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
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/IO/FileSystem.h>


#include <iostream>

#include "Player.h"
#include "Cross.h"
#include "NPC.h"
#include "Boulder.h"

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

void Gameplay::OnSetEnabled()
{
	if (IsEnabled()) 
	{
		GetSettings();
		input->SetMouseGrabbed(true);
		renderer->SetViewport(0, viewport);
		ourUI->SetEnabledRecursive(true);
		ourUI->SetVisible(true);
		renderer->GetViewport(0)->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward.xml"));
		input->SetMouseVisible(false);
	}
	else
	{
		ourUI->SetVisible(false);
		ourUI->SetEnabledRecursive(false);
	}
}

void Gameplay::Start()
{
	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	scene_ = SharedPtr<Scene>(GetScene());
	renderer = GetSubsystem<Renderer>();
	audio = GetSubsystem<Audio>();

	viewport = SharedPtr<Viewport>(new Viewport(context_));
	MakeHUD(scene_->GetGlobalVar("SCREEN WIDTH").GetInt(), scene_->GetGlobalVar("SCREEN HEIGHT").GetInt());
	
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Gameplay, AfterRenderUpdate));
}

void Gameplay::SetupGame()
{
	cache->ReleaseAllResources(false);
	playerNode = scene_->GetChild("player");
	player = playerNode->CreateComponent<Player>();
	audio->SetListener(player->GetComponent<SoundListener>());
	cameraNode = playerNode->GetChild("camera");
	camera = cameraNode->GetComponent<Camera>();
	player->input = input;

	viewport->SetScene(scene_);
	viewport->SetCamera(camera);

	//Setup boulder
	boulderNode = scene_->GetChild("boulder");
	Vector3 position = boulderNode->GetWorldPosition();
	boulderNode->LoadXML(cache->GetResource<XMLFile>("Objects/boulder.xml")->GetRoot());
	boulderNode->CreateComponent<Boulder>();
	boulderNode->SetWorldPosition(position);

	skybox = scene_->GetChild("skybox");

	SetupCrosses();

	//Get NPC skin information
	FileSystem* fs = GetSubsystem<FileSystem>();

	//Get number of models
	StringVector modelDir;
	fs->ScanDir(modelDir, fs->GetProgramDir() + "Data/Npcs/", "", SCAN_DIRS, false);
	int numModels = 0;
	for (StringVector::Iterator i = modelDir.Begin(); i != modelDir.End(); ++i)
	{
		String dir = (String)*i;
		if (dir.Contains('.', false))
			continue;
		if (dir.Contains("model", false))
			numModels += 1;
	}
	//Get number of skins
	int modelInfo[16][2];
	for (int i = 0; i < numModels; ++i)
	{
		StringVector skinDir;
		fs->ScanDir(skinDir, fs->GetProgramDir() + "Data/Npcs/model" + String(i), "", SCAN_FILES, false);
		int numSkins = 0; int numVoices = 0;
		for (StringVector::Iterator j = skinDir.Begin(); j != skinDir.End(); ++j)
		{
			String f = String(*j);
			if (f.Contains(".png"))
				numSkins += 1;
			if (f.Contains(".wav")) 
			{
				numVoices += 1;
			}
		}
		modelInfo[i][0] = numSkins;
		modelInfo[i][1] = numVoices;
	}

	//Spawn NPCs
	PODVector<Node*> npcs;
	scene_->GetChildrenWithTag(npcs, "npc", true);
	XMLFile* npcObject = cache->GetResource<XMLFile>("Objects/npc.xml");
	for (PODVector<Node*>::Iterator i = npcs.Begin(); i != npcs.End(); ++i)
	{
		Node* npc = (Node*)*i;
		Matrix3x4 transform = npc->GetWorldTransform();
		npc->LoadXML(npcObject->GetRoot());
		npc->SetWorldTransform(transform.Translation(), transform.Rotation(), Vector3::ONE);

		int model = 0;
		int skin = floor(Random() * modelInfo[model][0]);
		int voice = floor(Random() * modelInfo[model][1]);

		NPC* cNPC = new NPC(context_);
		cNPC->modelIndex = model;
		cNPC->voiceIndex = voice;
		cNPC->skinIndex = skin;
		npc->AddComponent(cNPC, 10, LOCAL);
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
		renderer->GetViewport(0)->GetRenderPath()->SetShaderParameter("FlashColor", flashColor);
	}
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
	scene_->SetGlobalVar("MOUSE SENSITIVITY", 0.25f);
	scene_->SetGlobalVar("FORWARD KEY", KEY_W);
	scene_->SetGlobalVar("BACKWARD KEY", KEY_S);
	scene_->SetGlobalVar("RIGHT KEY", KEY_D);
	scene_->SetGlobalVar("LEFT KEY", KEY_A);
	scene_->SetGlobalVar("JUMP KEY", KEY_SPACE);
	scene_->SetGlobalVar("VOICE VOLUME", 0.5f);
}

void Gameplay::MakeHUD(int width, int height)
{
	UI* ui = GetSubsystem<UI>();
	ui->GetRoot()->LoadChildXML(cache->GetResource<XMLFile>("UI/HUDLayout.xml")->GetRoot());
	ourUI = ui->GetRoot()->GetChild("hud", true);

	text = new Text(context_);
	text->SetText("GUN PRIEST ALPHA : WWW.BITENDOSOFTWARE.COM");
	text->SetFont("Fonts/Anonymous Pro.ttf", 12);
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_TOP);
	ourUI->AddChild(text);

	crosshair = new Sprite(context_);
	crosshair->SetTexture(cache->GetResource<Texture2D>("Textures/crosshair.png"));
	crosshair->SetFullImageRect();
	crosshair->SetSize(16, 16);
	crosshair->SetHotSpot(16, 16);
	crosshair->SetBlendMode(BLEND_ADDALPHA);
	crosshair->SetHorizontalAlignment(HA_CENTER);
	crosshair->SetVerticalAlignment(VA_CENTER);
	ourUI->AddChild(crosshair);

	crossIcon = new Sprite(context_);
	crossIcon->SetTexture(cache->GetResource<Texture2D>("Textures/cross_ui.png"));
	crossIcon->SetImageRect(IntRect(0, 0, 128, 128));
	crossIcon->SetSize(128, 128);
	crossIcon->SetHotSpot(128, 128);
	crossIcon->SetBlendMode(BLEND_ALPHA);
	crossIcon->SetHorizontalAlignment(HA_LEFT);
	crossIcon->SetVerticalAlignment(VA_TOP);
	ourUI->AddChild(crossIcon);
	crossIcon->SetPosition(width * 0.9f, height * 0.9f);

	crossCounter = new Text(context_);
	crossCounter->SetText("x0");
	crossCounter->SetFont("Fonts/Anonymous Pro.ttf", 24);
	crossCounter->SetVerticalAlignment(VA_CENTER);
	crossCounter->SetHorizontalAlignment(HA_LEFT);
	crossIcon->AddChild(crossCounter);
	crossCounter->SetPosition(128.0f, 0.0f);

	ourUI->SetEnabledRecursive(false);
	ourUI->SetVisible(false);
}

void Gameplay::FlashScreen(Color c, float spd)
{
	flashColor = c;
	flashSpeed = spd;
	renderer->GetViewport(0)->GetRenderPath()->SetShaderParameter("FlashColor", c);
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

void Gameplay::AfterRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	//scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(debugRenderer, true);
}
