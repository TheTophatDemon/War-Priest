class PortalAnimator : ScriptObject
{
	Material@ mat;
	PortalAnimator()
	{
		
	}
	void DelayedStart()
	{
		mat = cache.GetResource("Material", "Materials/portal.xml");
		ValueAnimation@ glow = ValueAnimation();
		glow.SetKeyFrame(0.0f, Variant(Color(0.0, 0.0, 0.0)));
		glow.SetKeyFrame(1.0f, Variant(Color(0.25, 0.25, 0.25)));
		glow.SetKeyFrame(2.0f, Variant(Color(0.0, 0.0, 0.0)));
		mat.SetShaderParameterAnimation("MatEmissiveColor", glow, WM_LOOP, 1.0f);
		
		ValueAnimation@ xTwist = ValueAnimation();
		xTwist.SetKeyFrame(0.0f, Variant(Vector4(1.0, 0.0, 0.0, 0.0)));
		xTwist.SetKeyFrame(1.0f, Variant(Vector4(0.71, 0.71, 1.0, -0.25)));
		xTwist.SetKeyFrame(1.5f, Variant(Vector4(0.2, 0.4, -0.5, 0.5)));
		xTwist.SetKeyFrame(2.5f, Variant(Vector4(-2.0, -0.75, 2.0, 0.5)));
		xTwist.SetKeyFrame(3.5f, Variant(Vector4(1.0, 0.0, 0.0, 0.0)));
		mat.SetShaderParameterAnimation("UOffset", xTwist);
		
		ValueAnimation@ yTwist = ValueAnimation();
		yTwist.SetKeyFrame(0.0f, Variant(Vector4(0.0, 1.0, 0.0, 0.0)));
		yTwist.SetKeyFrame(1.0f, Variant(Vector4(-0.71, 0.71, -0.75, 0.5)));
		yTwist.SetKeyFrame(1.5f, Variant(Vector4(-0.4, 0.2, 1.0, 1.0)));
		yTwist.SetKeyFrame(2.5f, Variant(Vector4(0.75, -2.0, -0.5, -1.2)));
		yTwist.SetKeyFrame(3.5f, Variant(Vector4(0.0, 1.0, 0.0, 0.0)));
		mat.SetShaderParameterAnimation("VOffset", yTwist);
		
		SubscribeToEvent(node.children[0], "NodeCollision", "OnCollision");
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherNode.name == "player")
		{
			ValueAnimation@ fade = ValueAnimation();
			fade.SetKeyFrame(0.0f, Variant(Color(1.0f, 1.0f, 1.0f)));
			fade.SetKeyFrame(1.0f, Variant(Color(0.0f, 0.0f, 0.0f)));
			mat.SetShaderParameterAnimation("MatEmissiveColor", fade, WM_CLAMP, 1.0f);
		}
	}
}