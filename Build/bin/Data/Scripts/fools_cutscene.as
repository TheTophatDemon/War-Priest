
class FoolsCutscene : ScriptObject
{
	Color godColor = Color(1.0f, 0.5f, 0.5f);
	Node@ god;
	Viewport@ viewport;
	Node@ oldCameraNode;
	Node@ newCameraNode;
	Camera@ oldCamera;
	Camera@ newCamera;
	int eventNo;
	bool triggered = false;
	void DelayedStart()
	{
		viewport = renderer.viewports[0];
		
		//SubscribeToEvent(node, "NodeCollision", "OnCollision");
		//Must reset things here, or you won't be able to skip it with the cheat menu.
		SubscribeToEvent("CutsceneEnd", "OnCutsceneEnd");
	}
	void Trigger()
	{
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
		fallAnim.SetKeyFrame(4.0f, Variant(god.worldPosition + Vector3(0.0f, 50.0f, 0.0f)));
		fallAnim.SetKeyFrame(7.0f, Variant(god.worldPosition));
		god.SetAttributeAnimation("Position", fallAnim, WM_CLAMP, 1.0f);
	}
	void FixedUpdate(float timeStep)
	{
		if (input.keyPress[KEY_H])
		{
			Trigger();
		}
	}
	void OnCutsceneEnd(StringHash eventType, VariantMap& eventData)
	{
		//Reset everything
		triggered = false;
		viewport.camera = oldCamera;
		audio.listener = oldCameraNode.GetComponent("SoundListener");
		if (newCameraNode != null) newCameraNode.Remove();
		if (god != null) god.Remove(); //Wait, what!?
	}
}