#include "bulletManager.h"
#include "game.h"
#include "bullet.h"

void BulletManager::Init(Game* game)
{
    assert(game);
    owningGame = game;
    
    std::function<void(EventBase*)> delegate = [this](EventBase* event) 
    {
        this->CreateBullet(event);
    };

    owningGame->GetEventSystem()->AddEventReply(EventType::TankFired, delegate);
}

void BulletManager::CreateBullet(EventBase* inEvent)
{
    Bullet* bullet = static_cast<Bullet*>(owningGame->CreateAndAddObject(GameObjectTypes::Bullet));

    // We might not have authority to create this object, in which case we terminate here.
    if (bullet)
    {
        TankFiredEvent* tankFiredEvent = static_cast<TankFiredEvent*>(inEvent);

        bullet->SetRotation(tankFiredEvent->direction);
        bullet->SetPosition(tankFiredEvent->position);
        bullet->shooter = tankFiredEvent->shooter;
    }
}
