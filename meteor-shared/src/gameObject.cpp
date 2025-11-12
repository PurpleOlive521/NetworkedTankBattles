#include "gameObject.h"
#include <assert.h>
#include "controller.h"

GameObject::~GameObject()
{
}

void GameObject::PreBeginPlay(Game* CreationGame)
{
	assert(CreationGame); // No valid game given
	owningGame = CreationGame;
	
}

void GameObject::BeginPlay()
{
}

bool GameObject::ShouldUpdate() const
{
	return !bPendingKill;
}

void GameObject::Update(float DeltaTime)
{

}

void GameObject::Render()
{

}

bool GameObject::IsPendingKill() const
{
	return bPendingKill;
}

void GameObject::SetIsPendingKill(bool bInState)
{
	bPendingKill = bInState;
}

void GameObject::OnDestroy()
{

}

GameObjectTypes GameObject::GetType() const
{
	return GameObjectTypes::GameObject;
}
