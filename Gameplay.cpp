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

	compassScene = new CompassScene(context);

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

	input->SetMouseGrabbed(true);
	ourUI->SetEnabledRecursive(true);
	ourUI->SetVisible(true);
	input->SetMouseVisible(false);
	scene_->SetUpdateEnabled(true);

	viewport->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward.xml"));
	viewport->GetRenderPath()->SetShaderParameter("State", 0.0f); //Always use decimal

	audio->SetMasterGain("GAMEPLAY", Settings::GetSoundVolume());
	audio->SetMasterGain("TITLE", 0.0f);

	//Apply certain settings on resume
	if (Settings::IsBloodEnabled())
	{
		cache->GetResource<ParticleEffect>("Particles/blood.xml")->SetMaterial(cache->GetResource<Material>("Materials/particle_blood.xml"));
		cache->GetResource<Material>("Materials/skins/stake_skin.xml")->SetShaderParameter("MatDiffColor", Color::WHITE);
		cache->GetResource<Material>("Materials/blood.xml")->SetShaderParameter("MatDiffColor", Color::WHITE);
	}
	else
	{
		cache->GetResource<ParticleEffect>("Particles/blood.xml")->SetMaterial(cache->GetResource<Material>("Materials/particle_heart.xml"));
		cache->GetResource<Material>("Materials/skins/stake_skin.xml")->SetShaderParameter("MatDiffColor", Color::BLUE);
		cache->GetResource<Material>("Materials/blood.xml")->SetShaderParameter("MatDiffColor", Color(0.1f, 0.1f, 0.1f, 1.0f));
	}
	SetupLighting();
}

void Gameplay::SetupGame()
{
	SetGlobalVar("PROJECTILE COUNT", 0);
	winState = 0;
	restartTimer = 0;

	physworld = scene_->GetComponent<PhysicsWorld>();
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
	audio->SetListener(playerNode->GetComponent<SoundListener>());

	//Setup Camera
	cameraNode = scene_->CreateChild();
	cameraNode->SetPosition(Vector3(0.0f, 24.0f, -10.0f));
	camera = cameraNode->CreateComponent<Camera>();
	camera->SetFov(70.0f);

	//Initialize player now that camera is ready
	playerNode->AddComponent(player, 666, LOCAL);

	viewport->SetScene(scene_);
	viewport->SetCamera(camera);

	skybox = scene_->GetChild("skybox");
	exitNode = scene_->GetChild("exit");

	//Load "liquids"
	PODVector<Node*> waters;
	scene_->GetChildrenWithTag(waters, "water", true);
	for (PODVector<Node*>::Iterator i = waters.Begin(); i != waters.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			n->CreateComponent<Water>();
		}
	}

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

			Node* dest = n->GetChild("dest");
			if (dest)
			{
				movement = dest->GetPosition() * dest->GetWorldScale();
				dest->Remove();
			}

			n->AddComponent(Lift::MakeLiftComponent(context_, movement, restSpeed, speed, rotSpeed), 1200, LOCAL);
			RigidBody* r = n->GetComponent<RigidBody>();
			if (!r) r = n->CreateComponent<RigidBody>();
			r->SetLinearFactor(Vector3::ZERO);
			r->SetAngularFactor(Vector3::ZERO);
			r->SetCollisionEventMode(COLLISION_ALWAYS);
			r->SetCollisionLayer(3);
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
	}

	//How about the launchpads, mate?
	PODVector<Node*> launchPads;
	scene_->GetChildrenWithTag(launchPads, "launchpad", true);
	for (PODVector<Node*>::Iterator i = launchPads.Begin(); i != launchPads.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			Matrix3x4 trans = n->GetWorldTransform();

			const float lf = n->GetVar("launchForce").GetFloat();

			n->LoadXML(cache->GetResource<XMLFile>("Objects/launchpad.xml")->GetRoot());
			n->SetWorldTransform(trans.Translation(), trans.Rotation(), trans.Scale());

			Launchpad* lp = n->CreateComponent<Launchpad>();
			if (lf != 0.0f)
			{
				lp->launchForce = lf;
			}
		}
	}
	
	//Czechpoints
	PODVector<Node*> checkpoints;
	scene_->GetChildrenWithTag(checkpoints, "checkpoint", true);
	for (PODVector<Node*>::Iterator i = checkpoints.Begin(); i != checkpoints.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			n->RemoveComponent<StaticModel>();
			RigidBody* rb = nullptr;
			if (!n->HasComponent<RigidBody>())
				n->CreateComponent<RigidBody>();
			rb = n->GetComponent<RigidBody>();
			rb->SetTrigger(true);
		}
	}

	//Statues
	PODVector<Node*> statues;
	scene_->GetChildrenWithTag(statues, "statue", true);
	for (PODVector<Node*>::Iterator i = statues.Begin(); i != statues.End(); ++i)
	{
		Node* n = dynamic_cast<Node*>(*i);
		if (n)
		{
			const int hp = n->GetVar("health").GetInt();
			const Matrix3x4 trans = n->GetWorldTransform();
			n->LoadXML(cache->GetResource<XMLFile>("Objects/statue.xml")->GetRoot());
			n->SetWorldTransform(trans.Translation(), trans.Rotation(), trans.Scale());
			Statue* s = n->CreateComponent<Statue>();
			if (hp != 0) 
			{
				s->Damage(s->GetHealth() - hp, true);
			}
		}
	}

	SetupEnemy();
	SetupProps();
	
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
		audio->SetMasterGain("GAMEPLAY", Settings::GetSoundVolume());
		UpdateHUD(timeStep);
		if (skybox)
		{
			skybox->Rotate(Quaternion(timeStep * 5.0f, Vector3::UP));
		}
		if (input->GetKeyDown(KEY_L)) player->reviveCount = 666;
		if (input->GetKeyDown(KEY_F1)) player->health = 100;
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
	//Count things
	PODVector<Node*> projs;
	scene_->GetChildrenWithTag(projs, "projectile", true);
	SetGlobalVar("PROJECTILE COUNT", projs.Size());

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
		healthMeter->SetSize(floor((oldHealth / 100.0f) * 628.0f), 22);
		//projectileCounter->SetText("PROJECTILE: " + String(GetGlobalVar("PROJECTILE COUNT").GetInt()));
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
	int width = ourUI->GetWidth();
	int height = ourUI->GetHeight();

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
	messageText->SetEffectStrokeThickness(4);
	ourUI->AddChild(messageText);
	messageText->SetVisible(false);

	/*projectileCounter = new Text(context_);
	projectileCounter->SetText("PROJECTILE: " + String(GetGlobalVar("PROJECTILE COUNT").GetInt()));
	projectileCounter->SetFont("Fonts/Anonymous Pro.ttf", 12);
	projectileCounter->SetHorizontalAlignment(HA_LEFT);
	projectileCounter->SetVerticalAlignment(VA_CENTER);
	ourUI->AddChild(projectileCounter);*/

	healthMeter = (Sprite*)ourUI->GetChild("healthbar", true)->GetChild(0);
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
		//std::cout << viewport->GetRenderPath()->GetCommand(viewport->GetRenderPath()->GetNumCommands() - 1)->GetShaderParameter("State").GetFloat() << std::endl;
		restartTimer = 250;
		gunPriest->musicSource->Stop();
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

		gunPriest->musicSource->Play(cache->GetResource<Sound>("Music/theyfeeltherain.ogg"));
	}
	winState = 1;
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

			if (enemyType == "pyropastor"){n->CreateComponent<PyroPastor>();}
			else if (enemyType == "postalpope"){n->CreateComponent<PostalPope>();}
			else if (enemyType == "dangerdeacon"){n->CreateComponent<DangerDeacon>();}
			else if (enemyType == "temptemplar"){n->CreateComponent<TempTemplar>();}

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
}

void Gameplay::ExtractLiquidsFromMap() //For more dynamic liquid volumes
{
	StaticModel* mapModel = mapNode->GetComponent<StaticModel>();
	Model* model = mapModel->GetModel();
	physworld->RemoveCachedGeometry(model); //Important!

	SharedPtr<Model> waterModel(new Model(context_)); //We plan on separating the parts of the map with "liquid" materials into their own model
	Vector<SharedPtr<Geometry>> waterGeometries = Vector<SharedPtr<Geometry>>();

	SharedPtr<Geometry> emptyGeo = SharedPtr<Geometry>(new Geometry(context_)); //Fake geometry so they don't complain at us
	for (int i = 0; i < mapModel->GetNumGeometries(); i++)
	{
		if (mapModel->GetMaterial(i)->GetTechnique(0) == cache->GetResource<Technique>("Techniques/Liquid.xml"))
		{
			std::cout << "WATER FOUND" << std::endl;
			waterGeometries.Push(SharedPtr<Geometry>(model->GetGeometry(i, 0)));
			model->SetGeometry(i, 0, emptyGeo); //This only affects the collisionshape generation for some reason.
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