#include "EditorApplication.hxx"

#include "Engine/Core/Engine.hxx"
#include "RHI/RHI.hxx"
#include "RHI/RHIScene.hxx"

#include "AssetRegistry/AssetRegistry.hxx"
#include "RHI/Resources/RHIViewport.hxx"

#include "Components/Oscillator.hxx"

#include <cpplogger/sinks/FileSink.hpp>
#include <cpplogger/sinks/StdoutSink.hpp>

DECLARE_LOGGER_CATEGORY(Core, LogApplication, Info)

EditorApplication::EditorApplication()
{
}

EditorApplication::~EditorApplication()
{
}

bool EditorApplication::OnEngineInitialization()
{
    VIT_PROFILE_FUNC()

    Super::OnEngineInitialization();

    FRHIGraphicsPipelineSpecification Spec{
        .VertexShader = "Shapes/ShapeShader.vert",
        .FragmentShader = "Shapes/ShapeShader.frag",
        .VertexBufferLayouts =
            {
                {
                    .InputMode = EVertexInputMode::PerVertex,
                    .Parameter =
                        {
                            {.Name = "Position", .Type = EVertexElementType::Float3},
                            {.Name = "Normal", .Type = EVertexElementType::Float3},
                            {.Name = "Tangent", .Type = EVertexElementType::Float3},
                            {.Name = "Binormal", .Type = EVertexElementType::Float3},
                            {.Name = "Texcoord", .Type = EVertexElementType::Float2},
                        },
                },
                {
                    .InputMode = EVertexInputMode::PerInstance,
                    .Parameter =
                        {
                            {.Name = "MatrixRow_a", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_b", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_c", .Type = EVertexElementType::Float4},
                            {.Name = "MatrixRow_d", .Type = EVertexElementType::Float4},
                        },
                },
            },
        .Rasterizer =
            {
                .PolygonMode = EPolygonMode::Fill,
                .CullMode = ECullMode::None,
                .FrontFaceCulling = EFrontFace::CounterClockwise,
            },
        .AttachmentFormats =
            {
                .ColorFormats = {MainViewport->GetBackbuffer()->GetDescription().Format},
                .DepthFormat = EImageFormat::D32_SFLOAT,
                .StencilFormat = std::nullopt,
            },
    };

    Ref<RRHIGraphicsPipeline> Pipeline = RHI::CreateGraphicsPipeline(Spec);
    Ref<RRHIMaterial> Material = RHI::CreateMaterial(Pipeline);
    Material->SetName("Shape Material");
    GEngine->AssetRegistry.RegisterMemoryOnlyMaterial(Material);

    FRHITextureSpecification DepthTexture = MainViewport->GetBackbuffer()->GetDescription();
    DepthTexture.Format = EImageFormat::D32_SFLOAT;
    DepthTexture.Flags = ETextureUsageFlags::DepthStencilTargetable;

    World = ecs::CreateWorld();
    World->SetName("Editor World");
    World->RegisterComponent<FOscillator>();
    GEngine->SetWorld(World);
    CameraEntity = World->CreateEntity()
                       .WithComponent(ecs::FRenderTargetComponent{
                           .Viewport = MainViewport,
                       })
                       .WithComponent(ecs::FTransformComponent{{0, 15, 0}, {}, {1, 1, 1}})
                       .WithComponent(ecs::FCameraComponent())
                       .Build();

    const unsigned GridSize = 10;
    for (float Row = 0; Row < GridSize; Row++)
    {
        for (float Col = 0; Col < GridSize; Col++)
        {
            float x = (Col - (GridSize - 1) / 2.0f) * 2;
            float y = (Row - (GridSize - 1) / 2.0f) * 2;
            std::string Name = std::format("Oscillator_{:f}_{:f}", x, y);
            FVector3 Position = {x, y, 0};
            FQuaternion Rotation;
            FVector3 Scale = {1, 1, 1};

            World->CreateEntity()
                .WithComponent(ecs::FMeshComponent{
                    .Asset = GEngine->AssetRegistry.GetCapsuleAsset(),
                    .Material = Material,
                    .RenderTarget = CameraEntity,
                })
                .WithComponent(ecs::FTransformComponent(Position, Rotation, Scale))
                .WithComponent(FOscillator())
                .Build();
        }
    }

    {
        Ref<ADirectionalLightActor> DirectionalLight = World->CreateActor<ADirectionalLightActor>("Directional Light");
    }
    {
        Ref<APointLightActor> PointLight = World->CreateActor<APointLightActor>("Point Light");
        PointLight->SetActorLocation({1.2, 1.0, 2.0});
        PointLight->LightComponent->Radius = 2.0f;
    }

    return true;
}

void EditorApplication::OnEngineDestruction()
{
    ecs::DestroyWorld(&World);
    Super::OnEngineDestruction();
}

void EditorApplication::Tick(const double DeltaTime)
{
    VIT_PROFILE_FUNC()

    Super::Tick(DeltaTime);

    MainWindow->SetText(std::to_string(1.0f / DeltaTime));
}

IApplication* CreateApplication()
{
    return new EditorApplication();
}
