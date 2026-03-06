#include "RHI/RHIShaderCompiler.hxx"

#include <slang.h>

DECLARE_LOGGER_CATEGORY(Core, LogShaderCompiler, Info)

static ERHIShaderType FromSlang(SlangStage Stage)
{
    switch (Stage)
    {
        case SlangStage::SLANG_STAGE_COMPUTE:
            return ERHIShaderType::Compute;
        case SlangStage::SLANG_STAGE_FRAGMENT:
            return ERHIShaderType::Fragment;
        case SlangStage::SLANG_STAGE_VERTEX:
            return ERHIShaderType::Vertex;
        default:
            checkNoEntry();
    }
}

static RTTI::EParameterType ParameterTypeFromSlang(slang::TypeReflection::ScalarType Type)
{
    switch (Type)
    {
        case slang::TypeReflection::ScalarType::Bool:
            return RTTI::EParameterType::Bool;
        case slang::TypeReflection::ScalarType::Float16:
            return RTTI::EParameterType::HFloat;
        case slang::TypeReflection::ScalarType::Float32:
            return RTTI::EParameterType::Float;
        case slang::TypeReflection::ScalarType::Float64:
            return RTTI::EParameterType::Double;
        case slang::TypeReflection::ScalarType::Int8:
            return RTTI::EParameterType::Int8;
        case slang::TypeReflection::ScalarType::Int16:
            return RTTI::EParameterType::Int16;
        case slang::TypeReflection::ScalarType::Int32:
            return RTTI::EParameterType::Int32;
        case slang::TypeReflection::ScalarType::Int64:
            return RTTI::EParameterType::Int64;
        case slang::TypeReflection::ScalarType::UInt8:
            return RTTI::EParameterType::Uint8;
        case slang::TypeReflection::ScalarType::UInt16:
            return RTTI::EParameterType::Uint16;
        case slang::TypeReflection::ScalarType::UInt32:
            return RTTI::EParameterType::Uint64;
        case slang::TypeReflection::ScalarType::UInt64:
            return RTTI::EParameterType::Uint32;
        case slang::TypeReflection::ScalarType::None:
        case slang::TypeReflection::ScalarType::Void:
            checkNoEntry();
    }
}

static EVertexElementType VertexTypeFromSlang(slang::TypeReflection::ScalarType Type, int Row, int Columns)
{
#define SLANG_CONVERT_VEC(SlangType, Type)                \
    case slang::TypeReflection::ScalarType::SlangType:    \
    {                                                     \
        if (Row == 1)                                     \
        {                                                 \
            switch (Columns)                              \
            {                                             \
                case 1:                                   \
                    return EVertexElementType::Type;      \
                case 2:                                   \
                    return EVertexElementType::Type##2;   \
                case 3:                                   \
                    return EVertexElementType::Type##3;   \
                case 4:                                   \
                    return EVertexElementType::Type##4;   \
                default:                                  \
                    checkNoEntry();                       \
            }                                             \
        }                                                 \
        else                                              \
        {                                                 \
            check(Columns == Row);                        \
            switch (Columns)                              \
            {                                             \
                case 2:                                   \
                    return EVertexElementType::Type##2x2; \
                case 3:                                   \
                    return EVertexElementType::Type##3x3; \
                case 4:                                   \
                    return EVertexElementType::Type##4x4; \
                default:                                  \
                    checkNoEntry();                       \
            }                                             \
        }                                                 \
    }
    switch (Type)
    {
        SLANG_CONVERT_VEC(Float32, Float)
        SLANG_CONVERT_VEC(Int32, Int)
        SLANG_CONVERT_VEC(UInt32, Uint)
        default:
            checkNoEntry();
    }
#undef SLANG_CONVERT_VEC
}

static void FillParameterType(::RTTI::FParameter& Param, slang::TypeLayoutReflection* TypeLayout)
{
    slang::TypeReflection* Type = TypeLayout->getType();
    switch (Type->getKind())
    {
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::Resource:
        case slang::TypeReflection::Kind::DynamicResource:
        case slang::TypeReflection::Kind::Struct:
        {
            Param.Type = RTTI::EParameterType::Struct;
        }
        break;
        case slang::TypeReflection::Kind::Scalar:
        {
            Param.Type = ParameterTypeFromSlang(Type->getScalarType());
        }
        break;
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Matrix:
        {
            Param.Type = ParameterTypeFromSlang(Type->getElementType()->getScalarType());
        }
        break;
        default:
        {
            checkMsg(false, "Unsupported type {}", magic_enum::enum_name(Type->getKind()));
        }
    }

    Param.Rows = Type->getColumnCount();
    Param.Columns = Type->getRowCount();

    slang::ParameterCategory Category = TypeLayout->getParameterCategory();
    Param.Size = TypeLayout->getSize(Category);
}

static int offset = 0;

static ::RTTI::FParameter RecuriveTypeDescription(slang::VariableLayoutReflection* TypeLayout);
static ::RTTI::FParameter RecuriveTypeDescription(slang::TypeLayoutReflection* TypeLayout)
{
    slang::TypeReflection* Type = TypeLayout->getType();

    ::RTTI::FParameter Parameter;
    Parameter.Name = Type->getName();

    std::string offsetString;
    for (int i = 0; i < offset; i++)
    {
        offsetString += "  ";
    }
    slang::TypeLayoutReflection* ElemTypeLayout = TypeLayout->getElementTypeLayout();
    if (ElemTypeLayout)
    {
        slang::TypeReflection* ElemType = ElemTypeLayout->getType();
        LOG(LogShaderCompiler, Trace, "{}Type: {}", offsetString, magic_enum::enum_name(ElemType->getKind()));
        LOG(LogShaderCompiler, Trace, "{}ElemType: {}", offsetString, magic_enum::enum_name(ElemType->getScalarType()));
        LOG(LogShaderCompiler, Trace, "{}Col: {}", offsetString, ElemType->getColumnCount());
        LOG(LogShaderCompiler, Trace, "{}Row: {}", offsetString, ElemType->getRowCount());

        for (unsigned i = 0; i < ElemTypeLayout->getFieldCount(); i++)
        {
            slang::VariableLayoutReflection* field = ElemTypeLayout->getFieldByIndex(i);
            offset += 1;
            Parameter.Members.Emplace(RecuriveTypeDescription(field));
            offset -= 1;
        }
        FillParameterType(Parameter, ElemTypeLayout);
    }
    else
    {
        FillParameterType(Parameter, TypeLayout);
    }
    return Parameter;
}

static ::RTTI::FParameter RecuriveTypeDescription(slang::VariableLayoutReflection* Type)
{
    std::string offsetString;
    for (int i = 0; i < offset; i++)
    {
        offsetString += "  ";
    }
    LOG(LogShaderCompiler, Trace, "{}Name: {}", offsetString, Type->getName());
    return RecuriveTypeDescription(Type->getTypeLayout());
}

static ShaderResource::FStageIO GetStageIO(slang::VariableLayoutReflection* Layout)
{
    ShaderResource::FStageIO StageIO;
    StageIO.Name = Layout->getName();
    StageIO.Binding = Layout->getBindingSpace();
    StageIO.Location = Layout->getBindingIndex();
    StageIO.Offset = Layout->getOffset();

    slang::TypeReflection* Type = Layout->getType();
    int Row = Type->getRowCount();
    int Columns = Type->getColumnCount();
    switch (Type->getKind())
    {
        case slang::TypeReflection::Kind::Scalar:
        {
            StageIO.Type = VertexTypeFromSlang(Type->getScalarType(), Row, Columns);
        }
        break;
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Matrix:
        {
            StageIO.Type = VertexTypeFromSlang(Type->getElementType()->getScalarType(), Row, Columns);
        }
        break;
        default:
        {
            checkMsg(false, "Unsupported type {}", magic_enum::enum_name(Type->getKind()));
        }
    }

    LOG(LogShaderCompiler, Info, "- {}", StageIO);
    return StageIO;
}

static TArray<ShaderResource::FStageIO> GetStageReflection(slang::VariableLayoutReflection* Reflect)
{
    TArray<ShaderResource::FStageIO> Stage;
    slang::TypeLayoutReflection* TypeReflection = Reflect->getTypeLayout();
    LOG(LogShaderCompiler, Info, "Name: {}", TypeReflection->getName());
    for (unsigned i = 0; i < TypeReflection->getFieldCount(); i++)
    {
        slang::VariableLayoutReflection* field = TypeReflection->getFieldByIndex(i);
        std::string_view Name = field->getSemanticName();
        if (Name.starts_with("SV_"))
        {
            // We don't care about the "expected" shader outputs
            continue;
        }
        Stage.Emplace(GetStageIO(field));
    }

    return Stage;
}

static void GetDescriptorSetReflectionForType(ShaderResource::FReflectionData& Data,
                                              slang::VariableLayoutReflection* Param)
{
    using EDescriptorType = ShaderResource::FDescriptorSetInfo::EDescriptorType;
    slang::ParameterCategory Category = Param->getCategory();

    switch (Category)
    {
        case slang::ParameterCategory::PushConstantBuffer:
        {
            ShaderResource::FPushConstantRange Push;
            Push.Parameter = RecuriveTypeDescription(Param);
            Push.Offset = Param->getOffset();
            Data.PushConstants = Push;
        }
        break;
        case slang::ParameterCategory::DescriptorTableSlot:
        {
            const int Binding = Param->getOffset(Category);
            const int Set = Param->getBindingSpace(Category);
            ShaderResource::FDescriptorSetInfo Info;
            switch (Param->getType()->getKind())
            {
                case slang::TypeReflection::Kind::ConstantBuffer:
                {
                    Info.Type = EDescriptorType::StorageBuffer;
                }
                case slang::TypeReflection::Kind::Resource:
                {
                    Info.Type = EDescriptorType::Sampler;
                }
                break;
                default:
                {
                    checkMsg(false, "Unsupported type {}", magic_enum::enum_name(Param->getType()->getKind()));
                }
            }
            Info.Parameter = RecuriveTypeDescription(Param);

            Data.DescriptorSetDeclaration.FindOrAdd(Set).FindOrAdd(Binding) = Info;
        }
        break;
        default:
            checkNoEntry();
    }
}

ShaderResource::FReflectionData RRHIShaderCompiler::GetReflection(const std::string_view& Path,
                                                                  slang::ProgramLayout* programLayout,
                                                                  unsigned EntryPointIndex)
{
    ShaderResource::FReflectionData Data;
    slang::EntryPointReflection* Ref = programLayout->getEntryPointByIndex(EntryPointIndex);
    LOG(LogShaderCompiler, Info, "===========================");
    LOG(LogShaderCompiler, Info, "Shader Reflection - {}", magic_enum::enum_name(Data.Type));
    LOG(LogShaderCompiler, Info, "Entry point: {}", Ref->getName());
    LOG(LogShaderCompiler, Info, "{}", Path);
    LOG(LogShaderCompiler, Info, "===========================");

    Data.Type = FromSlang(Ref->getStage());
    LOG(LogShaderCompiler, Info, "Stage input:{}", Ref->getParameterCount() ? "" : " None");
    for (unsigned x = 0; x < Ref->getParameterCount(); x++)
    {
        slang::VariableLayoutReflection* Reflect = Ref->getParameterByIndex(x);
        slang::VariableReflection* Variable = Reflect->getVariable();
        if (Variable->getType()->getKind() == slang::TypeReflection::Kind::Struct)
        {
            Data.StageInput = GetStageReflection(Reflect);
        }
        else
        {
            Data.StageInput.Emplace(GetStageIO(Reflect));
        }
    }

    slang::VariableLayoutReflection* ReturnLayout = Ref->getResultVarLayout();
    LOG(LogShaderCompiler, Info, "Stage output:{}", ReturnLayout ? "" : " None");
    Data.StageOutput = GetStageReflection(ReturnLayout);

    for (unsigned i = 0; i < programLayout->getParameterCount(); i++)
    {
        slang::VariableLayoutReflection* Param = programLayout->getParameterByIndex(i);
        GetDescriptorSetReflectionForType(Data, Param);
    }

    LOG(LogShaderCompiler, Info, "Push Constant Buffers:{}", Data.PushConstants.has_value() ? "" : " None");
    if (Data.PushConstants.has_value())
    {
        LOG(LogShaderCompiler, Info, "{}", Data.PushConstants.value());
    }
    LOG(LogShaderCompiler, Info, "Descriptor Sets:{}", Data.DescriptorSetDeclaration.IsEmpty() ? " None" : "");
    for (const auto& [Set, Bindings]: Data.DescriptorSetDeclaration)
    {
        for (const auto& [Binding, Info]: Bindings)
        {
            LOG(LogShaderCompiler, Info, "({},{}) {}", Set, Binding, Info);
        }
    }

    return Data;
}
