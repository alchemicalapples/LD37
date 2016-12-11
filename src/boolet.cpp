
#include "boolet.hpp"

#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

Boolet::Boolet(Context* context) :
    LogicComponent(context)
{

}

void Boolet::Start()
{
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Boolet, HandleNodeCollision));
}

void Boolet::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    auto otherbody = static_cast<Node*>(eventData[NodeCollision::P_OTHERNODE].GetPtr());
    auto tags = otherbody->GetTags();

    if (tags.Find("Enemy") != tags.End())
    {
        otherbody->Remove();

        if (score) ++*score;
    }

    node_->Remove();
}
