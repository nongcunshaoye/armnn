//
// Copyright © 2017 Arm Ltd. All rights reserved.
// See LICENSE file in the project root for full license information.
//
#pragma once

#ifdef ARMNN_HEAP_PROFILING_ENABLED

#include <string>
#include <cstddef>

// this is conditional so we can change the environment variable
// at build time
#ifndef ARMNN_HEAP_PROFILE_DUMP_DIR
#define ARMNN_HEAP_PROFILE_DUMP_DIR "ARMNN_HEAP_PROFILE_DUMP_DIR"
#endif // ARMNN_HEAP_PROFILE_DUMP_DIR

namespace armnnUtils
{
class ScopedHeapProfiler final
{
public:
    ScopedHeapProfiler(const std::string & tag);
    ~ScopedHeapProfiler();

private:
    // Location comes from the ARMNN_HEAP_PROFILE_DUMP_DIR
    // if not available then it dumps to /tmp
    std::string m_Location;
    std::string m_Tag;

    // No default construction and copying
    ScopedHeapProfiler() = delete;
    ScopedHeapProfiler(const ScopedHeapProfiler &) = delete;
    ScopedHeapProfiler & operator=(const ScopedHeapProfiler &) = delete;
};

} // namespace armnnUtils

#define ARMNN_SCOPED_HEAP_PROFILING(TAG) \
    armnnUtils::ScopedHeapProfiler __scoped_armnn_heap_profiler__(TAG)

#else // ARMNN_HEAP_PROFILING_ENABLED

#define ARMNN_SCOPED_HEAP_PROFILING(TAG)

#endif // ARMNN_HEAP_PROFILING_ENABLED
