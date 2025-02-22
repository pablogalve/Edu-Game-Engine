#include "Component.h"
#include "Globals.h"

#include "Leaks.h"

// ---------------------------------------------------------
Component::Component(GameObject* container, Component::Types type) : game_object(container), type(type)
{
	if (game_object != nullptr)
		SetActive(true);
}

// ---------------------------------------------------------
Component::~Component()
{}

// ---------------------------------------------------------
void Component::SetActive(bool active)
{
	if (this->active != active)
	{
		this->active = active;
		if (active)
			OnActivate();
		else
			OnDeActivate();
	}
}

// ---------------------------------------------------------
bool Component::IsActive() const
{
	return active;
}

// ---------------------------------------------------------
Component::Types Component::GetType() const
{
	return type;
}

// ---------------------------------------------------------
const char * Component::GetTypeStr() const
{
	static_assert(Component::Types::Unknown == 13, "String list needs update");

	static const char* names[] = {
	"MeshRenderer",
	"AudioListener",
	"AudioSource",
	"Camera",
	"RigidBody",
	"Animation",
	"Steering",
	"Path",
	"RootMotion",
	"CharacterController",
	"ParticleSystem",
	"Trail",
    "Grass",
	"Invalid" };

	return names[type];
}

// ---------------------------------------------------------
const GameObject * Component::GetGameObject() const
{
	return game_object;
}

// ---------------------------------------------------------
GameObject * Component::GetGameObject() 
{
	return game_object;
}
