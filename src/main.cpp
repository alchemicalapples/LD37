#include <ctime>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/AudioEvents.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>

#include "eyball.hpp"
#include "boolet.hpp"
#include "player.hpp"

enum CollisionLayers : unsigned {
    CL_DEFAULT = 0b00001,
    CL_PLAYER  = 0b00010,
    CL_PBOOLET = 0b00100,
    CL_ENEMY = 0b01000,
    CL_EBOOLET = 0b10000,
};

using namespace Urho3D;
/**
* Using the convenient Application API we don't have
* to worry about initializing the engine or writing a main.
* You can probably mess around with initializing the engine
* and running a main manually, but this is convenient and portable.
*/
class MyApp : public Application
{
public:
    int framecount_;
    float time_;
    SharedPtr<Text> text;
    SharedPtr<Scene> scene;
    SharedPtr<Node> playerNode;
    SharedPtr<Node> cameraNode;
    SharedPtr<Node> floorNode;

    double camPitch = 0;
    double camYaw = 0;

    float booletTimer = 0;

    float eyballTimer = 5;

    std::mt19937 rng;

    int score = 0;

    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    MyApp(Context * context) : Application(context),framecount_(0),time_(0)
    {
        context->RegisterFactory<Eyball>();
        context->RegisterFactory<Boolet>();
        context->RegisterFactory<Player>();
    }

    /**
    * This method is called before the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup()
    {
        // These parameters should be self-explanatory.
        // See http://urho3d.github.io/documentation/1.5/_main_loop.html
        // for a more complete list.
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
        engineParameters_["WindowTitle"]="LD37";
        engineParameters_["Sound"] = true;
    }

    /**
    * This method is called after the engine has been initialized.
    * This is where you set up your actual content, such as scenes,
    * models, controls and what not. Basically, anything that needs
    * the engine initialized and ready goes in here.
    */
    virtual void Start()
    {
        // We will be needing to load resources.
        // All the resources used in this example comes with Urho3D.
        // If the engine can't find them, check the ResourcePrefixPath (see http://urho3d.github.io/documentation/1.5/_main_loop.html).
        ResourceCache* cache=GetSubsystem<ResourceCache>();

        // Let's setup a scene to render.
        scene=new Scene(context_);
        scene->CreateComponent<Octree>();
        scene->CreateComponent<DebugRenderer>();

        auto physworld = scene->CreateComponent<PhysicsWorld>();
        physworld->SetGravity({0,-25,0});



        // Let's put some sky in there.
        // Again, if the engine can't find these resources you need to check
        // the "ResourcePrefixPath". These files come with Urho3D.
        //Node* skyNode=scene_->CreateChild("Sky");
        //skyNode->SetScale(500.0f); // The scale actually does not matter
        //Skybox* skybox=skyNode->CreateComponent<Skybox>();
        //skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        //skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        floorNode = scene->CreateChild("Floor");
        floorNode->SetPosition({0,0,0});
        auto floorbody = floorNode->CreateComponent<RigidBody>();
        auto floorshape = floorNode->CreateComponent<CollisionShape>();
        floorshape->SetBox({100,1,100});
        auto floormodel = floorNode->CreateComponent<StaticModel>();
        floormodel->SetModel(cache->GetResource<Model>("Models/Floor.mdl"));
        floormodel->SetMaterial(cache->GetResource<Material>("Materials/Floor.xml"));

        playerNode = scene->CreateChild("Player");
        auto playerbody = playerNode->CreateComponent<RigidBody>();
        auto playerShape = playerNode->CreateComponent<CollisionShape>();
        playerShape->SetCapsule(1.f, 3);
        playerNode->SetPosition({0,5,0});
        playerbody->SetAngularFactor(Vector3::ZERO);
        playerbody->SetMass(1);
        playerbody->SetCollisionLayer(CL_PLAYER);
        auto player = playerNode->CreateComponent<Player>();
        player->hp = 1;

        {
            auto light = playerNode->CreateComponent<Light>();
            light->SetBrightness(1);
            light->SetRange(10.0);
        }

        if(0){
            auto lightnode = scene->CreateChild("Light");
            auto light = lightnode->CreateComponent<Light>();
            light->SetLightType(LightType::LIGHT_DIRECTIONAL);
            lightnode->LookAt({0,-1,0},{0,1,0});
            light->SetBrightness(1);
        }

        cameraNode = playerNode->CreateChild("Camera");
        auto camera = cameraNode->CreateComponent<Camera>();
        camera->SetFarClip(1000);
        camera->SetFov(100);
        cameraNode->SetPosition({0,1.5,0});
        {
            auto playersound = cameraNode->CreateComponent<SoundSource>();
            auto listener = cameraNode->CreateComponent<SoundListener>();
            GetSubsystem<Audio>()->SetListener(listener);
        }

        // Now we setup the viewport. Of course, you can have more than one!
        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene,cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);

        auto uiRoot = GetSubsystem<UI>()->GetRoot();
        uiRoot->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

        text = new Text(context_);
        text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
        text->SetColor(Color(.5,.5,.9));
        text->SetHorizontalAlignment(HA_CENTER);
        text->SetVerticalAlignment(VA_TOP);

        uiRoot->AddChild(text);

        SubscribeToEvent(E_BEGINFRAME,URHO3D_HANDLER(MyApp,HandleBeginFrame));
        SubscribeToEvent(E_KEYDOWN,URHO3D_HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(MyApp,HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE,URHO3D_HANDLER(MyApp,HandlePostUpdate));
        SubscribeToEvent(E_RENDERUPDATE,URHO3D_HANDLER(MyApp,HandleRenderUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE,URHO3D_HANDLER(MyApp,HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME,URHO3D_HANDLER(MyApp,HandleEndFrame));
    }

    /**
    * Good place to get rid of any system resources that requires the
    * engine still initialized. You could do the rest in the destructor,
    * but there's no need, this method will get called when the engine stops,
    * for whatever reason (short of a segfault).
    */
    virtual void Stop()
    {
    }

    /**
    * Every frame's life must begin somewhere. Here it is.
    */
    void HandleBeginFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }

    /**
    * Input from keyboard is handled here. I'm assuming that Input, if
    * available, will be handled before E_UPDATE.
    */
    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();
        if(key==KEY_ESCAPE)
            engine_->Exit();

        if(key==KEY_TAB)    // toggle mouse cursor when pressing tab
        {
            GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    /**
    * You can get these events from when ever the user interacts with the UI.
    */
    void HandleClosePressed(StringHash eventType,VariantMap& eventData)
    {
        engine_->Exit();
    }
    /**
    * Your non-rendering logic should be handled here.
    * This could be moving objects, checking collisions and reaction, etc.
    */
    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        auto input = GetSubsystem<Input>();

        camYaw += input->GetMouseMoveX();
        camPitch += input->GetMouseMoveY();

        camPitch = std::max(camPitch,-90.0);
        camPitch = std::min(camPitch,90.0);

        auto camRot = Quaternion(camPitch,camYaw,0);

        cameraNode->SetRotation(camRot);

        auto playerbody = playerNode->GetComponent<RigidBody>();

        if (playerNode->GetComponent<Player>()->knockbackTimer <= 0) {
            auto mvec = Vector3{0,0,0};

            if (input->GetKeyDown(KEY_W)) {
                mvec.z_ += 1;
            }
            if (input->GetKeyDown(KEY_S)) {
                mvec.z_ += -1;
            }

            if (input->GetKeyDown(KEY_A)) {
                mvec.x_ += -1;
            }
            if (input->GetKeyDown(KEY_D)) {
                mvec.x_ += 1;
            }

            if (mvec.LengthSquared() > 0) {
                mvec.Normalize();
            }

            mvec *= 30;

            mvec = Quaternion(0,camYaw,0) * mvec;

            mvec = mvec - (playerbody->GetLinearVelocity() * Vector3(1,0,1));

            playerbody->ApplyImpulse(mvec);

            if (input->GetKeyPress(KEY_SPACE)) {
                playerbody->ApplyImpulse({0,15,0});
            }
        }

        auto cache = GetSubsystem<ResourceCache>();

        if (playerNode->GetComponent<Player>()->playHurt)
        {
            playerNode->GetComponent<Player>()->playHurt = false;
            auto sound = cache->GetResource<Sound>("Sounds/HurtPlayer.wav");
            auto source = cameraNode->GetComponent<SoundSource>();
            source->Play(sound);
            source->SetGain(0.6);
        }

        float tstep = eventData[Update::P_TIMESTEP].GetFloat();

        booletTimer -= tstep;

        if (input->GetMouseButtonDown(MOUSEB_LEFT) && booletTimer <= 0) {
            booletTimer = 0.25;

            auto boolet = scene->CreateChild("Boolet");
            boolet->SetPosition(cameraNode->GetWorldPosition() + camRot * Vector3::FORWARD * 2);
            boolet->SetScale(0.5);
            auto booletbody = boolet->CreateComponent<RigidBody>();
            auto booletshape = boolet->CreateComponent<CollisionShape>();
            booletshape->SetSphere(0.5);
            booletbody->SetMass(1);
            booletbody->SetLinearVelocity(camRot * Vector3::FORWARD * 35);
            booletbody->SetCollisionMask(~(CL_PLAYER & CL_PBOOLET));
            booletbody->SetCollisionLayer(CL_PBOOLET);
            booletbody->SetUseGravity(false);
            auto booletmodel = boolet->CreateComponent<StaticModel>();
            booletmodel->SetModel(cache->GetResource<Model>("Models/Boolet.mdl"));
            booletmodel->SetMaterial(cache->GetResource<Material>("Materials/Boolet.xml"));

            auto booletlamp = boolet->CreateChild("Lamp");
            auto booletlamplight = booletlamp->CreateComponent<Light>();
            booletlamplight->SetColor({0,1,0});

            auto booletdata = boolet->CreateComponent<Boolet>();
            booletdata->align = Boolet::PLAYER;
            booletdata->damage = 1;
            booletdata->score = &score;

            auto sound = cache->GetResource<Sound>("Sounds/Boolet.wav");
            auto source = cameraNode->GetComponent<SoundSource>();
            source->Play(sound);
        }

        eyballTimer -= tstep;

        if (eyballTimer <= 0) {
            eyballTimer = 5;

            auto yaw = std::uniform_real_distribution<float>(0, 360)(rng);

            auto eyball = scene->CreateChild("Eyball");
            eyball->SetPosition(Quaternion(0,yaw,0) * Vector3{50,20,0});
            eyball->SetScale(2.0);
            auto eyballbody = eyball->CreateComponent<RigidBody>();
            auto eyballshape = eyball->CreateComponent<CollisionShape>();
            eyballshape->SetSphere(2);
            eyballbody->SetMass(1);
            //eyballbody->SetCollisionMask(~(CL_ENEMY));
            eyballbody->SetCollisionLayer(CL_ENEMY);
            eyballbody->SetUseGravity(false);
            auto eyballmodel = eyball->CreateComponent<StaticModel>();
            eyballmodel->SetModel(cache->GetResource<Model>("Models/Eyball.mdl"));
            eyballmodel->SetMaterial(cache->GetResource<Material>("Materials/Eyball.xml"));

            auto eyballlamp = eyball->CreateChild("Lamp");
            auto eyballlamplight = eyballlamp->CreateComponent<Light>();
            eyballlamplight->SetColor({1,0,0});

            auto eyballeyball = eyball->CreateComponent<Eyball>();
            eyballeyball->SetTarget(playerNode);

            auto eyballsound = eyball->CreateComponent<SoundSource3D>();
            eyballsound->SetFarDistance(50.0);
            eyballsound->SetNearDistance(1.0);

            auto sound = cache->GetResource<Sound>("Sounds/Eyball.wav");
            sound->SetLooped(true);

            eyballsound->Play(sound);
            eyballsound->SetGain(0.2);

            eyball->AddTag("Enemy");
        }

        text->SetText(String(std::to_string(score).c_str()));
    }
    /**
    * Anything in the non-rendering logic that requires a second pass,
    * it might be well suited to be handled here.
    */
    void HandlePostUpdate(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * If you have any details you want to change before the viewport is
    * rendered, try putting it here.
    * See http://urho3d.github.io/documentation/1.32/_rendering.html
    * for details on how the rendering pipeline is setup.
    */
    void HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * After everything is rendered, there might still be things you wish
    * to add to the rendering. At this point you cannot modify the scene,
    * only post rendering is allowed. Good for adding things like debug
    * artifacts on screen or brush up lighting, etc.
    */
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We could draw some debuggy looking thing for the octree.
        //scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
        //scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    }
    /**
    * All good things must come to an end.
    */
    void HandleEndFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
};

/**
* This macro is expanded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)
