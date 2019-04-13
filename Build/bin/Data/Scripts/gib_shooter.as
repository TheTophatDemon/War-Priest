class GibShooter : ScriptObject
{
	GibShooter()
	{
		
	}
	void Start()
	{
		
	}
	void FixedUpdate(float timeStep)
	{
		if (input.keyPress[KEY_G])
		{
			Node@ playerGibs = scene.InstantiateXML(
				cache.GetResource("XMLFile", "Objects/playergibs.xml"),
				node.worldPosition,
				Quaternion(90.0f, Vector3::RIGHT) * node.worldRotation
			);
			playerGibs.scale = Vector3(0.7f, 0.7f, 0.7f);
			Node@[]@ gibs = playerGibs.GetChildren();
			for (int i = 0; i < gibs.length; i++)
			{
				if (gibs[i].HasTag("gore")) gibs[i].enabled = false;
				RigidBody@ body = gibs[i].GetComponent("RigidBody");
				body.ApplyImpulse(node.worldRotation * Vector3::FORWARD * 1250.0f);
			}
		}
	}
}