#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Player : public LogicComponent
{
    URHO3D_OBJECT(Player, LogicComponent);

public:
    /// Construct.
    Player(Context* context);

    void Update(float timeStep) override;

    int hp = 0;

    float knockbackTimer = 0;

    bool playHurt = false;
};

#endif // PLAYER_HPP
