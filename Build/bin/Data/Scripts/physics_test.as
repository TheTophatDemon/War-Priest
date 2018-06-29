//Drops physics cubes
class PhysicsTest : ScriptObject
{
	float speed = 1.0f;
	float timer = 0.0f;
	PhysicsTest()
	{
		
	}
	void Start()
	{
		
	}
	void FixedUpdate(float timeStep)
	{
		timer += timeStep;
		if (timer > speed)
		{
			timer = 0.0f;
			Node@ n = scene.CreateChild("Node");
			n.worldPosition = node.worldPosition;
			n.scale = Vector3(3.0f, 3.0f, 3.0f);
			StaticModel@ sm = n.CreateComponent("StaticModel");
			sm.model = cache.GetResource("Model", "Models/Box.mdl");
			RigidBody@ rb = n.CreateComponent("RigidBody");
			rb.mass = 100.0f;
			CollisionShape@ cs = n.CreateComponent("CollisionShape");
			cs.SetBox(Vector3(1.0f, 1.0f, 1.0));
		}
	}
}