#ifndef ALLOCATOR_ALLOCATOR_H
#define ALLOCATOR_ALLOCATOR_H

#include "allocator/region.h"
#include "allocator/gc_region.h"
#include <sys/mman.h>
#include <cstdint>
#include <new>

namespace k3s {

class Allocator
{
    static constexpr uintptr_t ALLOC_START_ADDR = 0xE000000;
    static constexpr size_t ALLOC_SIZE = 1024 * 1024 * 32;

    static constexpr uintptr_t CONSTANTS_REGION_PTR = ALLOC_START_ADDR;
    static constexpr size_t    CONSTANTS_REGION_SIZE = ALLOC_SIZE / 4;
    static constexpr uintptr_t GC_INTERNALS_REGION_PTR = CONSTANTS_REGION_PTR + CONSTANTS_REGION_SIZE;
    static constexpr size_t    GC_INTERNALS_REGION_SIZE = ALLOC_SIZE / 8;
    static constexpr uintptr_t STACK_REGION_PTR = GC_INTERNALS_REGION_PTR + GC_INTERNALS_REGION_SIZE;
    static constexpr size_t    STACK_REGION_SIZE = ALLOC_SIZE / 8;
    static constexpr uintptr_t OBJECTS_REGION_PTR = STACK_REGION_PTR + STACK_REGION_SIZE;
    static constexpr size_t    OBJECTS_REGION_SIZE = ALLOC_SIZE / 2;

public:
    using ConstRegionT = Region<CONSTANTS_REGION_PTR, CONSTANTS_REGION_SIZE>;
    using GCInternalsRegionT = Region<GC_INTERNALS_REGION_PTR, GC_INTERNALS_REGION_SIZE>;
    using StackRegionT = Region<STACK_REGION_PTR, STACK_REGION_SIZE>;
    using RuntimeRegionT = GCRegion<OBJECTS_REGION_PTR, OBJECTS_REGION_SIZE>;

    static void Init()
    {
        void *buf = mmap(reinterpret_cast<void *>(ALLOC_START_ADDR), ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        ASSERT(buf == reinterpret_cast<void *>(ALLOC_START_ADDR));
        ConstRegionT::Reset();
        GCInternalsRegionT::Reset();
        StackRegionT::Reset();
        RuntimeRegionT::Reset();
    }

    static void Destroy()
    {
        munmap(reinterpret_cast<void *>(ALLOC_START_ADDR), ALLOC_SIZE);
    }

    auto &ConstRegion()
    {
        return const_region_;
    }

    auto &GcInternalsRegion()
    {
        return gc_internals_region_;
    }

    auto &StackRegion()
    {
        return stack_region_;
    }

    auto &ObjectsRegion()
    {
        return objects_region_;
    }

private:
    ConstRegionT const_region_;
    GCInternalsRegionT gc_internals_region_;
    StackRegionT stack_region_;
    RuntimeRegionT objects_region_;
};

}  // namespace k3s

#endif  // ALLOCATOR_ALLOCATOR_H
