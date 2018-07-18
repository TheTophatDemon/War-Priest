#include "Gameplay.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
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
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Audio/Sound.h>

#include <iostream>

#include "Player.h"
#include "Enemy.h"
#include "PyroPastor.h"
#include "Projectile.h"
#include "TempEffect.h"
#include "GunPriest.h"
#include "God.h"
#include "PostalPope.h"
#include "DangerDeacon.h"
#include "Water.h"
#include "Settings.h"
#include "Launchpad.h"
#include "TempTemplar.h"
#include "ChaosCaliph.h"
#include "LevelSelectMenu.h"
#include "Bonus.h"
#include "MissileFinder.h"
#include "KillerKube.h"
#include "Statue.h"
#include "Lift.h"
#include "GravityPlate.h"

using namespace Urho3D;

StringHash Gameplay::E_BONUSCOLLECTED = StringHash("BonusCollected");

Gameplay::Gameplay(Context* context) : LogicComponent(context), 
	flashSpeed(0.0f), 
	oldHealth(100.0f), 
	initialized(false),
	restartTimer(0),
	flashColor(Color(0.0f,0.0f,0.0f,0.0f)),
	bonusFlag(false)
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	
	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	renderer = GetSubsystem<Renderer>();
	audio = GetSubsystem<Audio>();

	compassScene = new CompassScene(context);

	MakeHUD();
}

void Gameplay::RegisterObject(Context* context)
{
	context->RegisterFactory<Gameplay>();
}

void Gameplay::Start()
{
	audio->SetMasterGain("GAMEPLAY", Settings::GetSoundVolume());
	audio->SetMasterGain("TITLE", 0.0f);
	audio->SetMasterGain("MUSIC", Settings::GetMusicVolume());

	viewport = renderer->GetViewport(0);
	scene_ = SharedPtr<Scene>(GetScene());

	input->SetMouseGrabbed(true);
	ourUI->SetEnabledRecursive(true);
	ourUI->SetVisible(true);

	ourUI->SetSize(Settings::GetResolutionX(), Settings::GetResolutionY());
	if (Settings::GetResolutionY() < 600)
		GetSubsystem<UI>()->SetScale(0.5f);
	else
		GetSubsystem<UI>()->SetScale(1.0f);
	ourUI->SetAlignment(HA_CENTER, VA_BOTTOM);

	input->SetMouseVisible(false);
	scene_->SetUpdateEnabled(true);

	viewport->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward.xml"));
	viewport->GetRenderPath()->SetShaderParameter("State", 0.0f); //Always use decimal

	//Apply certain settings on resume
	if (Settings::IsBloodEnabled())
	{
		cache->GetResource<ParticleEffect>("Particles/blood.xml")->SetMaterial(cache->GetResource<Material>("Materials/particle_blood.xml"));
		cache->GetResource<Material>("Materials/skins/stake_skin.xml")->SetShaderParameter("MatDiffColor", Color::WHITE);
		cache->GetResource<Material>("Materials/blood.xml")->SetShaderParameter("MatDiffColor", Color::WHITE);
		cache->GetResource<Material>("Materials/decal_death.xml")->SetShaderParameter("MatDiffColor", Color::WHITE);
	}
	else
	{
		cache->GetResource<ParticleEffect>("Particles/blood.xml")->SetMaterial(cache->GetResource<Material>("Materials/particle_heart.xml"));
		cache->GetResource<Material>("Materials/skins/stake_skin.xml")->SetShaderParameter("MatDiffColor", Color::BLUE);
		cache->GetResource<Material>("Materials/blood.xml")->SetShaderParameter("MatDiffColor", Color(0.1f, 0.1f, 0.1f, 1.0f));
		cache->GetResource<Material>("Materials/decal_death.xml")->SetShaderParameter("MatDiffColor", Color(0.1f, 0.1f, 0.1f, 1.0f));
	}
	SetupLighting();

	SubscribeToEvent(E_BONUSCOLLECTED, URHO3D_HANDLER(Gameplay, HandleEvent));
}

void Gameplay::SetupGame()
{
	PreloadSounds();
	
	winState = 0;
	restartTimer = 0;
	bonusFlag = false;

	physworld = scene_->GetComponent<PhysicsWorld>();
	physworld->SetFps(60);
	physworld->SetMaxSubSteps(10);
	physworld->SetInterpolation(false);
	physworld->SetInternalEdge(true);

	mapNode = scene_->GetChild("map");

	ExtractLiquidsFromMap();
	
	compassScene->Start();

	//Setup Player
	playerNode = scene_->GetChild("player");
	Matrix3x4 trans = playerNode->GetWorldTransform();
	playerNode->LoadXML(cache->GetResource<XMLFile>("Objects/player.xml")->GetRoot());
	playerNode->SetWorldTransform(trans.Translation(), trans.Rotation(), trans.Scale());
	player = new Player(context_);
	

	//Setup Camera
	cameraNode = scene_->CreateChild();
	cameraNode->SetPosition(Vector3(0.0f, 24.0f, -10.0f));
	camera = cameraNode->CreateComponent<Camera>();
	camera->SetFov(70.0f);
	audio->SetListener(cameraNode->CreateComponent<SoundListener>());

	//Initialize player now that camera is ready
	playerNode->AddComponent(player, 666, LOCAL);

	viewport->SetScene(scene_);
	viewport->SetCamera(camera);

	skybox = scene_->GetChild("skybox");
	exitNode = scene_->GetChild("exit");

	PODVector<Node*> results;

	//Load "liquids"
	scene_->GetChildrenWithTag(results, "water", true);
	for (Node* n : results)
	{
		n->CreateComponent<Water>();
	}

	//Setup Lifts
	scene_->GetChildrenWithTag(results, "lift", true);
	for (Node* n : results)
	{
		Vector3 movement = n->GetVar("movement").GetVector3();
		float restTime = n->GetVar("restTime").GetFloat(); if (restTime == 0.0f) restTime = 1.0f;
		float speed = n->GetVar("speed").GetFloat(); if (speed == 0.0f) speed = 2.0f;
		const float rotateSpeed = n->GetVar("rotateSpeed").GetFloat();
		const float activeRadius = n->GetVar("activeRadius").GetFloat();
		const bool wait = n->GetVar("waitForPlayer").GetBool();

		Vector3 pointA = n->GetWorldPosition();
		Vector3 pointB = pointA + movement;

		Node* dest = n->GetChild("dest");
		if (dest)
		{
			pointB = dest->GetWorldPosition();
			dest->Remove();
		}

		Lift* lift = new Lift(context_);
		lift->pointA = pointA;
		lift->pointB = pointB;
		lift->restTime = restTime;
		//The speed value used to represent the number of seconds it would take
		//to get from point A to point B, but it's not like that anymore, so we
		//must calculate the correct speed value based on distance and time.
		lift->speed = ((pointA-pointB).Length()) / speed;
		lift->activeRadius = activeRadius;
		lift->rotateSpeed = rotateSpeed;
		lift->wait = wait;
		n->AddComponent(lift, 1200, LOCAL);
	}

	//Setup Medkits
	SharedPtr<ValueAnimation> va(new ValueAnimation(context_));
	va->SetKeyFrame(0.0f, Color::BLACK);
	va->SetKeyFrame(1.0f, Color::GRAY);
	va->SetKeyFrame(2.0f, Color::BLACK);
	cache->GetResource<Material>("Materials/skins/medkit_skin.xml")->SetShaderParameterAnimation("MatEmissiveColor", va, WM_LOOP, 1.0f);

	scene_->GetChildrenWithTag(results, "medkit", true);
	for (Node* n : results)
	{
		RigidBody* rb = n->CreateComponent<RigidBody>();
		rb->SetCollisionLayer(32);
		rb->SetCollisionMask(129); //128+1
		rb->SetTrigger(true);
		StaticModel* sm = n->GetComponent<StaticModel>();
		CollisionShape* cs = n->CreateComponent<CollisionShape>();
		cs->SetBox(sm->GetBoundingBox().Size(),sm->GetBoundingBox().Center(), Quaternion::IDENTITY);
		if (n->GetVar("health").GetInt() == 0)
		{
			n->SetVar("health", 20);
		}
	}

	//How about the launchpads, mate?
	scene_->GetChildrenWithTag(results, "launchpad", true);
	for (Node* n : results)
	{
		Matrix3x4 trans = n->GetTransform();

		const float lf = n->GetVar("launchForce").GetFloat();

		n->LoadXML(cache->GetResource<XMLFile>("Objects/launchpad.xml")->GetRoot());
		n->SetTransform(trans.Translation(), trans.Rotation(), trans.Scale());

		Launchpad* lp = n->CreateComponent<Launchpad>();
		if (lf != 0.0f)
		{
			lp->launchForce = lf;
		}
	}
	
	//Czechpoints
	scene_->GetChildrenWithTag(results, "checkpoint", true);
	for (Node* n : results)
	{
		n->RemoveComponent<StaticModel>();
		RigidBody* rb = nullptr;
		if (!n->HasComponent<RigidBody>())
			n->CreateComponent<RigidBody>();
		rb = n->GetComponent<RigidBody>();
		rb->SetTrigger(true);
	}

	//Statues
	scene_->GetChildrenWithTag(results, "statue", true);
	for (Node* n : results)
	{
		const int hp = n->GetVar("health").GetInt();
		const Matrix3x4 trans = n->GetWorldTransform();
		n->LoadXML(cache->GetResource<XMLFile>("Objects/statue.xml")->GetRoot());
		n->SetWorldTransform(trans.Translation(), trans.Rotation(), trans.Scale());
		n->AddComponent(Statue::MakeStatueComponent(context_, hp == 0 ? 100 : hp), 3333, LOCAL);
	}

	//BONUS (ducks)
	scene_->GetChildrenWithTag(results, "bonus", true);
	for (Node* n : results)
	{
		SharedPtr<Bonus> bonus(new Bonus(context_));
		const Matrix3x4 trans = n->GetWorldTransform();
		n->LoadXML(cache->GetResource<XMLFile>("Objects/bonus.xml")->GetRoot());
		n->SetWorldTransform(trans.Translation(), trans.Rotation());
		n->AddComponent(bonus, 0, LOCAL);
	}

	//Gravity Plates
	scene_->GetChildrenWithTag(results, "gravityplate", true);
	for (Node* n : results)
	{
		SharedPtr<GravityPlate> gravityPlate(new GravityPlate(context_));
		gravityPlate->rotationAxis = n->GetVar("rotationAxis").GetVector3().Normalized();
		gravityPlate->rotationForce = n->GetVar("rotationForce").GetFloat();
		const Matrix3x4 trans = n->GetWorldTransform();
		n->LoadXML(cache->GetResource<XMLFile>("Objects/gravityplate.xml")->GetRoot());
		n->SetWorldTransform(trans.Translation(), trans.Rotation());
		n->AddComponent(gravityPlate, 0, LOCAL);
	}

	//Killer Kube
	Node* kk = scene_->GetChild("killerkube");
	if (kk != nullptr)
	{
		KillerKube* kkc = new KillerKube(context_);
		kk->AddComponent(kkc, 999, LOCAL);
	}

	SetupEnemy();
	SetupProps();
	
	//Setup Music
	musicNode = scene_->GetChild("musicplayer");
	if (!musicNode.Get())
		musicNode = scene_->CreateChild("musicplayer", LOCAL, 1280U);
	musicSource = musicNode->CreateComponent<SoundSource>();
	musicSource->SetSoundType("MUSIC");
	if (musicNode->GetVar("music").GetString() != "")
	{
		Sound* music = cache->GetResource<Sound>("Music/" + musicNode->GetVar("music").GetString());
		music->SetLooped(true);
		musicSource->Play(music);
	}
	else if (scene_->GetVar("music").GetString() != "")
	{
		Sound* music = cache->GetResource<Sound>("Music/" + scene_->GetVar("music").GetString());
		music->SetLooped(true);
		musicSource->Play(music);
	}
	else
	{
		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!NO MUSIC IN THIS LEVEL!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	}

	messageText->SetVisible(false);
	RenderPathCommand* cmd = viewport->GetRenderPath()->GetCommand(viewport->GetRenderPath()->GetNumCommands() - 1);
	cmd->SetShaderParameter("State", Variant(0.0f));
	//std::cout << cmd->GetShaderParameter("State").GetFloat() << std::endl;
	initialized = true;
}

void Gameplay::FixedUpdate(float timeStep)
{
	if (IsEnabled()) 
	{
		if (input->GetKeyDown(KEY_ALT))
		{
			input->SetMouseGrabbed(false);
			input->SetMouseVisible(true);
		}
		else
		{
			input->SetMouseGrabbed(true);
			input->SetMouseVisible(false);
		}
		UpdateHUD(timeStep);
		if (skybox)
		{
			skybox->Rotate(Quaternion(timeStep * 5.0f, Vector3::UP));
		}
		if (player->reviveCount >= enemyCount && enemyCount > 0)
		{
			if (exitNode) 
			{
				if (!exitNode->HasComponent<ParticleEmitter>())
				{
					DisplayMessage("Mission Complete! Return to the start of the level!", Color::WHITE, 10.0f);
					ParticleEmitter* em = exitNode->CreateComponent<ParticleEmitter>();
					em->SetEffect(cache->GetResource<ParticleEffect>("Particles/muzzleflash.xml"));
					em->SetEmitting(true);
				}
				float dist = (playerNode->GetWorldPosition() - exitNode->GetWorldPosition()).Length();
				if (dist < 2.0f)
				{
					Win();
				}
			}
			else
			{
				Win();
			}
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
	}
}

void Gameplay::UpdateHUD(float timeStep)
{
	compass1->SetTexture(compassScene->renderedTexture);

	if (messageTimer > 0.0f)
	{
		messageTimer -= timeStep;
		if (messageTimer <= 0.0f)
		{
			messageTimer = 0.0f;
			messageText->SetVisible(false);
		}
	}

	if (player) 
	{
		if (oldHealth != player->health)
		{
			float diff = (oldHealth - player->health);
			oldHealth -= diff * 0.25f;
			if (fabs(diff) < 0.1f) oldHealth = player->health;
		}
		healthMeter->SetSize(floor((oldHealth / 100.0f) * 628.0f), healthMeter->GetHeight());
		reviveMeter->SetSize(floor(((player->reviveCooldownMax - player->reviveCooldown) / player->reviveCooldownMax) * 628.0f), reviveMeter->GetHeight());
	}

	if (flashColor.a_ > 0.0f)
	{
		flashColor.a_ -= flashSpeed * timeStep * 100.0f;
		if (flashColor.a_ < 0.0f) flashColor.a_ = 0.0f;
	}
	viewport->GetRenderPath()->SetShaderParameter("FlashColor", flashColor);
	if (winState == -1)
		viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
}

Gameplay::~Gameplay()
{
	
}

void Gameplay::MakeHUD()
{
	UI* ui = GetSubsystem<UI>();
	ui->GetRoot()->LoadChildXML(cache->GetResource<XMLFile>("UI/HUDLayout.xml")->GetRoot());
	ourUI = ui->GetRoot()->GetChild("hud", true);

	text = new Text(context_);
	text->SetText("WAR PRIEST ALPHA : WWW.BITENDOSOFTWARE.COM");
	text->SetFont("Fonts/Anonymous Pro.ttf", 12);
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_TOP);
	ourUI->AddChild(text);

	messageText = new Text(context_);
	messageText->SetText("");
	messageText->SetFont("Fonts/Anonymous Pro.ttf", 24);
	messageText->SetAlignment(HA_CENTER, VA_CENTER);
	messageText->SetTextAlignment(HA_CENTER);
	messageText->SetTextEffect(TextEffect::TE_STROKE);
	messageText->SetEffectStrokeThickness(2);
	ourUI->AddChild(messageText);
	messageText->SetVisible(false);

	debugText = new Text(context_);
	debugText->SetText("");
	debugText->SetFont("Fonts/Anonymous Pro.ttf", 12);
	debugText->SetAlignment(HA_LEFT, VA_TOP);
	debugText->SetPosition(64, 160);
	ourUI->AddChild(debugText);
	debugText->SetVisible(false);

	healthMeter = (Sprite*)ourUI->GetChild("healthbar", true)->GetChild("health", false);
	reviveMeter = (Sprite*)ourUI->GetChild("healthbar", true)->GetChild("reviveCooldown", false);
	compass1 = (Sprite*)ourUI->GetChild("compass1", true);
	compass1->SetTexture(compassScene->renderedTexture);
	compass1->SetFullImageRect();

	ourUI->SetEnabled(false);
	ourUI->SetVisible(false);
}

void Gameplay::FlashScreen(Color c, float spd)
{
	flashColor = c;
	flashSpeed = spd;
	viewport->GetRenderPath()->SetShaderParameter("FlashColor", c);
}

void Gameplay::DisplayMessage(String msg, Color col, float time)
{
	if (winState == 0) //So nobody tries to cover up the win message
	{
		messageText->SetText(msg);
		messageText->SetColor(col);
		messageText->SetVisible(true);
		messageTimer = time;
	}
}

void Gameplay::Lose()
{
	if (restartTimer == 0)
	{
		DisplayMessage("Mission Failed.\nThe Lord frowns upon you!", Color::WHITE, 250.0f);
		viewport->GetRenderPath()->SetShaderParameter("State", 1.0f);
		musicSource->Play(cache->GetResource<Sound>("Music/frownofthelord.ogg"));
		//std::cout << viewport->GetRenderPath()->GetCommand(viewport->GetRenderPath()->GetNumCommands() - 1)->GetShaderParameter("State").GetFloat() << std::endl;
		restartTimer = 250;
	}
	winState = -1;
}

void Gameplay::Win()
{
	if (restartTimer == 0)
	{
		DisplayMessage("Mission Complete!", Color::WHITE, 250.0f);
		viewport->GetRenderPath()->SetShaderParameter("State", 0.0f);
		restartTimer = 250;

		Node* godNode = scene_->CreateChild();
		godNode->LoadXML(cache->GetResource<XMLFile>("Objects/god.xml")->GetRoot(), false);
		God* god = godNode->CreateComponent<God>();
		god->SetTarget(playerNode);

		musicSource->Play(cache->GetResource<Sound>("Music/theyfeeltherain.ogg"));

		LevelSelectMenu* lsm = dynamic_cast<LevelSelectMenu*>(gunPriest->titleScreen->levelSelectMenu.Get());
		if (lsm) 
		{
			const int newFlags = bonusFlag ? LevelSelectMenu::LCF_BEATEN | LevelSelectMenu::LCF_CROSSGOTTEN : LevelSelectMenu::LCF_BEATEN;
			lsm->SetLevelCompletionFlag(levelPath, newFlags, true);
		}
	}
	winState = 1;
}

void Gameplay::SetupEnemy()
{
	enemyCount = 0;
	PODVector<Node*> enemies;
	scene_->GetChildrenWithTag(enemies, "enemy", true);
	for (Node* n : enemies)
	{
		String enemyType = n->GetName();
		Matrix3x4 t = n->GetWorldTransform();
		n->LoadXML(cache->GetResource<XMLFile>("Objects/" + enemyType + ".xml")->GetRoot());
		n->SetWorldTransform(t.Translation(), t.Rotation(), t.Scale());

		if (enemyType == "pyropastor"){n->CreateComponent<PyroPastor>();}
		else if (enemyType == "postalpope"){n->CreateComponent<PostalPope>();}
		else if (enemyType == "dangerdeacon"){n->CreateComponent<DangerDeacon>();}
		else if (enemyType == "temptemplar"){n->CreateComponent<TempTemplar>();}
		else if (enemyType == "chaoscaliph") { n->CreateComponent<ChaosCaliph>(); }

		enemyCount += 1;
	}
}

void Gameplay::SetupProps()
{
	float mapScale = mapNode->GetScale().x_;

	PODVector<Node*> props; //Get the existing props
	mapNode->GetChildrenWithTag(props, "prop", true);

	//Go through each prop and add rigid bodies
	for (Node* n : props) 
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

	PODVector<Node*> instancers;
	mapNode->GetChildrenWithTag(instancers, "propInstancer", false);

	//Find the instancing groups and remove the children's StaticMesh components and add them to the instancing groups
	for (Node* n : instancers)
	{
		if (n->GetPosition() != Vector3::ZERO)
		{
			std::cout << "HEY! One of the instancing groups are mispositioned! " << n->GetName().CString() << std::endl;
		}
		StaticModelGroup* modelGroup = n->GetComponent<StaticModelGroup>();
		assert(modelGroup != nullptr);
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
				shape->SetRotation(shape->GetRotation() * child->GetRotation());
				child->RemoveComponent<CollisionShape>();
				n->AddComponent(shape, 1200, LOCAL);
			}
			modelGroup->AddInstanceNode(child);
		}

		body->EnableMassUpdate();
	}
}

void Gameplay::ExtractLiquidsFromMap() //For more detailed liquid volumes
{
	StaticModel* mapModel = mapNode->GetComponent<StaticModel>();
	Model* model = mapModel->GetModel();
	physworld->RemoveCachedGeometry(model); //Important!

	SharedPtr<Model> waterModel(new Model(context_)); //We plan on separating the parts of the map with "liquid" materials into their own model
	Vector<SharedPtr<Geometry>> waterGeometries = Vector<SharedPtr<Geometry>>();

	SharedPtr<Geometry> emptyGeo = SharedPtr<Geometry>(new Geometry(context_)); //Fake geometry so they don't complain at us
	for (int i = 0; i < mapModel->GetNumGeometries(); i++)
	{
		if (mapModel->GetMaterial(i) != nullptr) 
		{
			if (mapModel->GetMaterial(i)->GetTechnique(0) == cache->GetResource<Technique>("Techniques/Liquid.xml"))
			{
				std::cout << "WATER FOUND" << std::endl;
				waterGeometries.Push(SharedPtr<Geometry>(model->GetGeometry(i, 0)));
				model->SetGeometry(i, 0, emptyGeo); //This only affects the collisionshape generation for some reason.
			}
		}
	}
	CollisionShape* cs = mapNode->GetComponent<CollisionShape>(); //Update the map's collision shape; should now exclude water geometry.
	cs->SetTriangleMesh(model, 0U);

	//Put extracted geometries into new model
	waterModel->SetNumGeometries(waterGeometries.Size());
	unsigned int i = 0;
	for (Vector<SharedPtr<Geometry>>::Iterator g = waterGeometries.Begin(); g != waterGeometries.End(); ++g)
	{
		SharedPtr<Geometry> geo = (SharedPtr<Geometry>)*g;
		if (geo)
		{
			waterModel->SetNumGeometryLodLevels(i, 1);
			waterModel->SetGeometry(i, 0, geo);
		}
		i++;
	}
	//Create new water node
	Node* waterNode = mapNode->CreateChild("water");
	waterNode->AddTag("water");
	CollisionShape* waterShape = waterNode->CreateComponent<CollisionShape>();
	waterShape->SetTriangleMesh(waterModel, 0U);
}

void Gameplay::SetupLighting()
{
	PODVector<Node*> lights;
	scene_->GetChildrenWithComponent<Light>(lights);
	for (PODVector<Node*>::Iterator i = lights.Begin(); i != lights.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			Light* l = n->GetComponent<Light>();
			if (Settings::AreGraphicsFast())
				l->SetPerVertex(true);
			else
				l->SetPerVertex(false);
		}
	}
}

void Gameplay::PreloadSounds()
{
	FileSystem* fs = GetSubsystem<FileSystem>();
	StringVector resDirs = cache->GetResourceDirs();
	for (String sd : resDirs)
	{
		StringVector sounds;
		fs->ScanDir(sounds, (sd + "Sounds"), "", SCAN_FILES, true);
		//std::cout << "THERE ARE " << sounds.Size() << " SOUNDS IN " << (sd + "Sounds").CString() << std::endl;
		for (String s : sounds)
		{
			if (!s.EndsWith(".xml", false)) //Skip the config files, m8
			{
				const int lastSlash = s.FindLast("/", 4000U, false);
				String soundName = s.Substring(lastSlash + 1);
				//std::cout << soundName.CString() << std::endl;
				cache->GetResource<Sound>("Sounds/" + soundName);
			}
		}
	}
}

void Gameplay::HandleEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_BONUSCOLLECTED)
	{
		FlashScreen(Color(1.0f, 0.0f, 1.0f, 1.0f), 0.01f);
		DisplayMessage(bonusFlag ? "YOU FOUND THE BONUS CROSS...AGAIN!" : "YOU FOUND THE BONUS CROSS!", Color::MAGENTA, 2.0f);
		bonusFlag = true;
	}
}