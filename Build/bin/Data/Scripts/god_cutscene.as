int numTriggers = 0;
class GodCutscene : ScriptObject
{
	Color godColor = Color(1.0f, 0.5f, 0.5f);
	Gameplay@ game;
	Node@ god;
	SoundSource@ godVoice;
	Sound@ postCutsceneMusic;
	Viewport@ viewport;
	Node@ oldCameraNode;
	Node@ newCameraNode;
	Node@ playerNode;
	Node@ modelNode;
	Camera@ oldCamera;
	Camera@ newCamera;
	AnimationController@ animController;
	Actor@ actor;
	int eventNo;
	bool triggered = false;
	void DelayedStart()
	{
		viewport = renderer.viewports[0];
		playerNode = scene.GetChild("player");
		modelNode = cast<Player@>(playerNode.GetComponent("Player")).modelNode;
		animController = modelNode.GetComponent("AnimationController");
		actor = playerNode.GetComponent("Actor");
		game = scene.GetComponent("Gameplay");
		godVoice = node.CreateComponent("SoundSource");
		godVoice.soundType = "GAMEPLAY";
		
		//Try to preload voice lines
		cache.GetResource("Sound", "Sounds/god_talk0.wav");
		cache.GetResource("Sound", "Sounds/god_talk1.wav");
		cache.GetResource("Sound", "Sounds/god_talk2.wav");
		
		postCutsceneMusic = cache.GetResource("Sound", "Music/riseofthewarpriest.ogg");
		postCutsceneMusic.looped = true;
		
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
		//Must reset things here, or you won't be able to skip it with the cheat menu.
		SubscribeToEvent("CutsceneEnd", "OnCutsceneEnd");
	}
	void Trigger()
	{
		SendEvent("CutsceneStart");
		triggered = true;
		numTriggers++;
		
		//Animate with a new camera so we don't mess up the old one
		oldCameraNode = viewport.camera.node;
		oldCamera = viewport.camera;
		newCameraNode = oldCameraNode.Clone();
		newCameraNode.parent = scene;
		newCamera = newCameraNode.GetComponent("Camera");
		viewport.camera = newCamera;
		audio.listener = newCameraNode.GetComponent("SoundListener");
		
		//Print("This cutscene has been triggered " + numTriggers + " times.");
		if (numTriggers > 1 && numTriggers != 7) //Don't play the cutscene more than once
		{
			DoEvent(999);
			return;
		}
		
		//Put it in its rightful place
		Node@ ref = node.GetChild("camera_ref");
		Vector3 newCameraPosition = ref.worldTransform.Translation();
		Quaternion newCameraRotation = ref.worldTransform.Rotation();
		ref.Remove();
		
		//Make the transition smooth
		ValueAnimation@ camPosAnim = ValueAnimation();
		camPosAnim.SetKeyFrame(0.0f, Variant(oldCameraNode.worldPosition));
		camPosAnim.SetKeyFrame(1.0f, Variant(newCameraPosition));
		ValueAnimation@ camRotAnim = ValueAnimation();
		camRotAnim.SetKeyFrame(0.0f, Variant(oldCameraNode.worldRotation));
		camRotAnim.SetKeyFrame(1.0f, Variant(newCameraRotation));
		
		newCameraNode.SetAttributeAnimation("Position", camPosAnim, WM_ONCE, 1.0f);
		newCameraNode.SetAttributeAnimation("Rotation", camRotAnim, WM_ONCE, 1.0f);
		
		//Spawn the lord
		god = scene.CreateChild("god");
		god.LoadXML(cast<XMLFile>(cache.GetResource("XMLFile", "Objects/god.xml")).GetRoot());
		ref = node.GetChild("god_ref");
		god.SetWorldTransform(ref.worldTransform.Translation(), ref.worldTransform.Rotation());
		ref.Remove();
		ValueAnimation@ spinAnim = ValueAnimation();
		spinAnim.SetKeyFrame(0.0f, Variant(Quaternion(0.0f, Vector3::UP)));
		spinAnim.SetKeyFrame(1.0f, Variant(Quaternion(90.0f, Vector3::UP)));
		spinAnim.SetKeyFrame(2.0f, Variant(Quaternion(180.0f, Vector3::UP)));
		spinAnim.SetKeyFrame(3.0f, Variant(Quaternion(270.0f, Vector3::UP)));
		spinAnim.SetKeyFrame(4.0f, Variant(Quaternion(360.0f, Vector3::UP)));
		god.SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 2.0f);
		ValueAnimation@ fallAnim = ValueAnimation();
		fallAnim.SetKeyFrame(0.0f, Variant(god.worldPosition + Vector3(0.0f, 50.0f, 0.0f)));
		fallAnim.SetKeyFrame(2.0f, Variant(god.worldPosition + Vector3(0.0f, 50.0f, 0.0f)));
		fallAnim.SetKeyFrame(5.0f, Variant(god.worldPosition));
		fallAnim.SetKeyFrame(22.0f, Variant(god.worldPosition));
		fallAnim.SetKeyFrame(26.0f, Variant(god.worldPosition + Vector3(0.0f, 50.0f, 0.0f)));
		god.SetAttributeAnimation("Position", fallAnim, WM_CLAMP, 1.0f);
		
		actor.SetInputFPS(false, false, false, false);
		
		if (numTriggers != 7)
		{
			DoEvent(0);
			ScheduleEvent(1.75f, 1);
			
			ScheduleEvent(4.0f, 2);
			ScheduleEvent(10.5f, 3);
			ScheduleEvent(18.0f, 4);
			
			ScheduleEvent(25.5f, 5);
			ScheduleEvent(26.5f, 999);
		}
		else //Alternate cutscene for struggling players
		{
			DoEvent(0);
			ScheduleEvent(1.75f, 1);
			ScheduleEvent(4.0f, 7);
			ScheduleEvent(9.0f, 8);
			ScheduleEvent(19.5f, 9);
			ScheduleEvent(22.0f, 999);
		}
	}
	void ScheduleEvent(float secs, int eventNo)
	{
		Array<Variant> param;
		param.Push(eventNo);
		DelayedExecute(secs, false, "void DoEvent(int)", param);
	}
	void DoEvent(int number)
	{
		eventNo = number;
		//Print(number);
		switch (number)
		{
			case 0:
				animController.PlayExclusive("Models/grungle_walk.ani", 0, true, 0.2f);
				break;
			case 1:
				animController.PlayExclusive("Models/grungle_idle.ani", 0, true, 0.2f);
				break;
			case 2:
				game.DisplayMessage("MY LOYAL SERVANT...IT IS TIME FOR THE FIGHTING TO END.", godColor, 6.0f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk0.wav"));
				break;
			case 3:
				game.DisplayMessage("MY EARTH HAS BEEN DESTROYED. YOU ARE AT FAULT AS MUCH AS THE OTHERS.", godColor, 7.0f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk1.wav"));
				break;
			case 4:
				game.DisplayMessage("I WILL GRANT YOU NEW POWERS. FIX THIS MESS, AND I WILL CONSIDER MERCY.", godColor, 8.5f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk2.wav"));
				break;
			case 5:
				Zeus::MakeLightBeam(scene, playerNode.worldPosition, 256.0f);
				break;
			case 7:
				game.DisplayMessage("YOUR PERSEVERENCE IS ADMIRABLE.", godColor, 5.0f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk3.wav"));
				break;
			case 8:
				game.DisplayMessage("TRY THIS MYSTICAL CODE WORD: T-D-U-R-I-N-A-L.", godColor, 9.0f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk4.wav"));
				break;
			case 9:
				game.DisplayMessage("BYE.", godColor, 2.0f, 50);
				godVoice.Play(cache.GetResource("Sound", "Sounds/god_talk5.wav"));
				god.SetAttributeAnimationSpeed("Position", 10.0f);
				break;
			case 999:
				SendEvent("CutsceneEnd");
				break;
		}
	}
	void FixedUpdate(float timeStep)
	{
		if (triggered)
		{
			switch(eventNo)
			{
				case 0:
				{
					const Vector3 diff = Vector3(god.worldPosition.x - playerNode.worldPosition.x, 0.0f, god.worldPosition.z - playerNode.worldPosition.z);
					Quaternion newRot;
					newRot.FromLookRotation(diff.Normalized());
					playerNode.worldRotation = newRot;
					actor.SetInputFPS(true, false, false, false);
					break;
				}
				default:
					actor.SetInputFPS(false, false, false, false);
					break;
			}
			actor.Move(timeStep);
		}
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherNode.name == "player" && !triggered)
		{
			Trigger();
			UnsubscribeFromEvent(node, "NodeCollision");
		}
	}
	void OnCutsceneEnd(StringHash eventType, VariantMap& eventData)
	{
		//Change music
		game.musicSource.Play(postCutsceneMusic);
		//Reset everything
		triggered = false;
		viewport.camera = oldCamera;
		audio.listener = oldCameraNode.GetComponent("SoundListener");
		if (newCameraNode != null) newCameraNode.Remove();
		if (god != null) god.Remove(); //Wait, what!?
	}
}