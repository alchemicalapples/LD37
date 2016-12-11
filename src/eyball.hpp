#ifndef EYEBALL_HPP
#define EYEBALL_HPP

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Eyball : public LogicComponent
{
    URHO3D_OBJECT(Eyball, LogicComponent);

public:
    /// Construct.
    Eyball(Context* context);

    void SetTarget(SharedPtr<Node> t);

    void Start() override;
    void FixedUpdate(float timeStep) override;

    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

private:
    /// Rotation speed.
    SharedPtr<Node> target;
    float timeAlive = 0;
};

#endif // EYEBALL_HPP
