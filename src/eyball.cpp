
#include "eyball.hpp"

#include <algorithm>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/IO/MemoryBuffer.h>

#include "boolet.hpp"
#include "player.hpp"

Eyball::Eyball(Context* context) :
    LogicComponent(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Eyball::Start()
{
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Eyball, HandleNodeCollision));
}

void Eyball::SetTarget(SharedPtr<Node> t)
{
    target = t;
}

void Eyball::FixedUpdate(float timeStep) //virtual
{
    timeAlive += timeStep;
    if (target) {
        node_->LookAt(target->GetPosition(), Vector3::UP);
        auto body = node_->GetComponent<RigidBody>();
        auto cvel = body->GetLinearVelocity();
        auto dist = target->GetComponent<RigidBody>()->GetPosition() - body->GetPosition();
        auto tvel = dist.Normalized() * (dist.Length() < 10 ? 2.0 : 30.0);
        body->ApplyImpulse(tvel - cvel);
    }
}

void Eyball::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    auto othernode = static_cast<Node*>(eventData[NodeCollision::P_OTHERNODE].GetPtr());

    if (auto player = othernode->GetComponent<Player>()) {
        if (player->knockbackTimer <= 0) {
            auto otherbody = othernode->GetComponent<RigidBody>();
            auto body = node_->GetComponent<RigidBody>();
            auto dir = otherbody->GetPosition() - body->GetPosition();
            otherbody->ApplyImpulse(dir.Normalized() * 10 + Vector3::UP * 10);
            player->hp -= 1;
            player->knockbackTimer = 0.5;
            player->playHurt = true;
        }
    }
}
