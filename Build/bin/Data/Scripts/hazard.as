class Hazard : ScriptObject
{
	int damage = 50;
	Hazard()
	{
		
	}
	void Start()
	{
		SubscribeToEvent(node, "NodeCollision", "OnCollision");
	}
	void OnCollision(StringHash eventType, VariantMap& eventData)
	{
		Node@ otherNode = eventData["OtherNode"].GetPtr();
		RigidBody@ otherBody = eventData["OtherBody"].GetPtr();
		if (otherNode.name == "player")
		{
			VariantMap params;
			params["victim"] = otherNode;
			params["perpetrator"] = node;
			params["damage"] = damage;
			SendEvent("ProjectileHit", params);
		}
	}
}