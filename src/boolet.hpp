#ifndef BOOLET_HPP
#define BOOLET_HPP

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Boolet : public LogicComponent
{
    URHO3D_OBJECT(Boolet, LogicComponent);

public:
    /// Construct.
    Boolet(Context* context);
    void Start() override;
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    enum Alignment {
        PLAYER,
        ENEMY
    };

    Alignment align = PLAYER;

    int damage = 1;

    int* score = nullptr;
};

#endif // BOOLET_HPP
