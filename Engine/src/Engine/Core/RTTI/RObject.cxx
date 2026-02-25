#include "Engine/Core/RTTI/RObject.hxx"

#include <shared_mutex>
#include <unordered_set>

static std::unordered_set<RObject*> s_LiveReferences;
static std::shared_mutex s_LiveReferenceMutex;

void RObjectUtils::AddToLiveReferences(RObject* instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    s_LiveReferences.insert(instance);
}

void RObjectUtils::RemoveFromLiveReferences(RObject* instance)
{
    check(instance);

    std::scoped_lock lock(s_LiveReferenceMutex);
    check(s_LiveReferences.find(instance) != s_LiveReferences.end());
    s_LiveReferences.erase(instance);
}

int RObjectUtils::IsLive(RObject* instance)
{
    check(instance);
    std::shared_lock lock(s_LiveReferenceMutex);
    auto Iter = s_LiveReferences.find(instance);
    if (Iter != s_LiveReferences.end())
    {
        return (*Iter)->GetRefCount();
    }
    return 0;
}

bool RObjectUtils::AreThereAnyLiveObject(bool bPrintObjects)
{
    std::scoped_lock lock(s_LiveReferenceMutex);

    if (bPrintObjects)
    {
        for (RObject* ObjectPtr: s_LiveReferences)
        {
            LOG(LogRObject, Trace, "{}<{}> ({:p}) have {} references", ObjectPtr->GetBaseTypeName(),
                ObjectPtr->GetName(), (void*)ObjectPtr, ObjectPtr->GetRefCount());
        }
    }
    return s_LiveReferences.size() > 0;
}

FNamedClass::FNamedClass(const std::string_view& InName): m_Name(InName)
{
}

void FNamedClass::SetName(std::string_view InName)
{
    m_Name = InName;
}

const std::string& FNamedClass::GetName() const
{
    return m_Name;
}

std::string FNamedClass::ToString() const
{
    return std::format("(\"{:s}\" <{:s}> {:p})", GetName(), GetBaseTypeName(), (void*)this);
}
