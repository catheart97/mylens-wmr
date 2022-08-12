#pragma once

#include "pch.h"

#include "Eye/Object.h"
#include "Eye/EnvironmentMaterial.h"
#include "Eye/EnvironmentMesh.h"

#include "Utility/winrtUtility.h"

namespace My::Eye
{

namespace _implementation
{

using namespace DirectX;

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::MixedReality;
using namespace winrt::Microsoft::MixedReality::SceneUnderstanding;

class Environment : public Scene
{
    // Data //
private:
    bool _query_quads{true};
    bool _query_meshes{true};
    bool _query_only_observed{false};
    bool _query_world_mesh{true};
    SceneMeshLevelOfDetail _query_detail{SceneMeshLevelOfDetail::Fine};

    float _radius{10.0f};

    SceneUnderstanding::Scene _scene{nullptr};

    winrt::com_ptr<ID3D11Device1> _device{nullptr};

    // Properties //
public:
    void query_quads(bool v) { _query_quads = v; }

    void query_meshes(bool v) { _query_meshes = v; }

    void query_only_observed(bool v) { _query_only_observed = v; }

    void query_world_mesh(bool v) { _query_world_mesh = v; }

    void query_detail(SceneMeshLevelOfDetail detail) { _query_detail = detail; }

    void radius(float r) { _radius = r; }

    // The environment is always in global space.
    XMMATRIX model_matrix() override { return XMMatrixIdentity(); }

    // Constructors //
public:
    Environment(winrt::com_ptr<ID3D11Device1> device, Object * parent = nullptr) : Scene(parent), _device{device}
    {
        visible(false);
        if (!SceneObserver::IsSupported())
        {
            Utility::winrtUtility::LogMessage(L"SceneObserver not supported.");
        }
        initialize();
    }

    // Methods //
protected:
    IAsyncAction initialize()
    {
        auto result = co_await SceneObserver::RequestAccessAsync();
        switch (result)
        {
            case SceneObserverAccessStatus::Allowed:
                Utility::winrtUtility::LogMessage(L"SceneObserver access 'Allowed'.");
                break;
            case SceneObserverAccessStatus::DeniedBySystem:
                Utility::winrtUtility::LogMessage(L"SceneObserver access 'Denied by System'.");
                break;
            case SceneObserverAccessStatus::DeniedByUser:
                Utility::winrtUtility::LogMessage(L"SceneObserver access 'Denied by User'.");
                break;
            case SceneObserverAccessStatus::NotDeclaredByApp:
                Utility::winrtUtility::LogMessage(L"SceneObserver access 'Not declared in app'.");
                break;
            case SceneObserverAccessStatus::UserPromptRequired:
                Utility::winrtUtility::LogMessage(L"SceneObserver access 'User prompt required'.");
                break;
        }

        if (result == SceneObserverAccessStatus::Allowed)
        {

            SceneUnderstanding::SceneQuerySettings query_settings;

            query_settings.EnableSceneObjectQuads = _query_quads;
            query_settings.EnableSceneObjectMeshes = _query_meshes;
            query_settings.EnableOnlyObservedSceneObjects = _query_only_observed;
            query_settings.EnableWorldMesh = _query_world_mesh;
            query_settings.RequestedMeshLevelOfDetail = _query_detail;

            // Initialize a new Scene
            SceneUnderstanding::Scene scene{nullptr};
            if (_scene)
                scene = co_await SceneObserver::ComputeAsync(query_settings, _radius, _scene);
            else
                scene = co_await SceneObserver::ComputeAsync(query_settings, _radius);
            Utility::winrtUtility::LogMessage(L"Environment query done.");

            auto scene_objects = scene.SceneObjects();

            std::vector<SceneObject> _walls;
            std::vector<SceneObject> _floor;
            std::vector<SceneObject> _ceiling;
            std::vector<SceneObject> _platform;
            std::vector<SceneObject> _world;
            std::vector<SceneObject> _background;

            for (SceneObject object : scene_objects)
            {
                auto c_meshes = object.ColliderMeshes();
                auto meshes = object.Meshes();

                for (SceneMesh mesh : meshes)
                {
                    // TODO: SolidEnvironmentMaterial + Mesh generation 
                }
            }

            _scene = scene;
            Utility::winrtUtility::LogMessage(L"Scene analysis done.");
        }
    }
};

} // namespace _implementation

using _implementation::Environment;

} // namespace My::Eye