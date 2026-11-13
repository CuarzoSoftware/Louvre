#include <CZ/Louvre/Manager/LImageCaptureManager.h>
#include <cassert>

using namespace CZ;

LImageCaptureManager::LImageCaptureManager(const void *params) noexcept : LFactoryObject(FactoryObjectType)
{
    assert(params != nullptr && "Invalid parameter passed to LImageCaptureManager constructor.");
    LImageCaptureManager **ptr { (LImageCaptureManager**) params };
    assert(*ptr == nullptr && *ptr == CZ::imageCaptureManager() && "Only a single LImageCaptureManager instance can exist.");
    *ptr = this;
}

LImageCaptureManager::~LImageCaptureManager() noexcept
{
    notifyDestruction();
}
