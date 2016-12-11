
#include "player.hpp"

Player::Player(Context* context) :
    LogicComponent(context)
{
    SetUpdateEventMask(USE_UPDATE);
}

void Player::Update(float timeStep) //virtual
{
    knockbackTimer -= timeStep;
}
