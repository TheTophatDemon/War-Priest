class GodCutscene : ScriptObject
{
	Gameplay@ game;
	Node@ god;
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
		
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
	}
	void Trigger()
	{
		//Don't play this one more than once
		if (game.levelVisits > 1)
		{
			triggered = false;
			return;
		}
	
		SendEvent("CutsceneStart");
		triggered = true;
		
		//Animate with a new camera so we don't mess up the old one
		oldCameraNode = viewport.camera.node;
		oldCamera = viewport.camera;
		newCameraNode = oldCameraNode.Clone();
		newCameraNode.parent = scene;
		newCamera = newCameraNode.GetComponent("Camera");
		viewport.camera = newCamera;
		audio.listener = newCameraNode.GetComponent("SoundListener");
		
		//Put it in its rightful place
		Node@ ref = node.GetChild("camera_ref");
		newCameraNode.SetWorldTransform(ref.worldTransform.Translation(), ref.worldTransform.Rotation());
		ref.Remove();
		
		//Make the transition smooth
		ValueAnimation@ camPosAnim = ValueAnimation();
		camPosAnim.SetKeyFrame(0.0f, Variant(oldCameraNode.worldPosition));
		camPosAnim.SetKeyFrame(1.0f, Variant(newCameraNode.worldPosition));
		ValueAnimation@ camRotAnim = ValueAnimation();
		camRotAnim.SetKeyFrame(0.0f, Variant(oldCameraNode.worldRotation));
		camRotAnim.SetKeyFrame(1.0f, Variant(newCameraNode.worldRotation));
		
		newCameraNode.SetAttributeAnimation("Position", camPosAnim, WM_CLAMP, 1.0f);
		newCameraNode.SetAttributeAnimation("Rotation", camRotAnim, WM_CLAMP, 1.0f);
		
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
		
		DoEvent(0);
		ScheduleEvent(1.75f, 1);
		
		ScheduleEvent(4.0f, 2);
		ScheduleEvent(10.5f, 3);
		ScheduleEvent(17.0f, 4);
		
		ScheduleEvent(23.5f, 5);
		ScheduleEvent(24.5f, 999);
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
				game.DisplayMessage("MY LOYAL SERVANT...IT IS TIME FOR THE FIGHTING TO END.", Color(1.0f, 0.5f, 0.5f), 6.0f, 50);
				break;
			case 3:
				game.DisplayMessage("MY EARTH HAS BEEN DESTROYED. YOU ARE AT FAULT AS MUCH AS THE OTHERS.", Color(1.0f, 0.5f, 0.5f), 6.0f, 50);
				break;
			case 4:
				game.DisplayMessage("I WILL GRANT YOU NEW POWERS. FIX THIS MESS, AND I WILL CONSIDER MERCY.", Color(1.0f, 0.5f, 0.5f), 6.0f, 50);
				break;
			case 5:
			{
				Zeus::MakeLightBeam(scene, playerNode.worldPosition, 256.0f);
				break;
			}
			case 999:
				SendEvent("CutsceneEnd");
				triggered = false;
				viewport.camera = oldCamera;
				audio.listener = oldCameraNode.GetComponent("SoundListener");
				newCameraNode.Remove();
				god.Remove(); //Wait, what!?
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
				case 1:
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
}