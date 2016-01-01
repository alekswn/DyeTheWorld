/***********************************************************************************
 *                C++11 Performance and Memory Benchmark                           *
 *   Copyright (C) 2015  Alexey Novikov (Email: alexey_AT_novikov_DOT_io)          *
 *   All rights reserved.                                                          *
 *                                                                                 *
 * Redistribution and use in source and binary forms, with or without              *
 * modification, are permitted provided that the following conditions are met:     *
 *                                                                                 *
 * 1. Redistributions of source code must retain the above copyright notice, this  *
 *    list of conditions and the following disclaimer.                             *
 * 2. Redistributions in binary form must reproduce the above copyright notice,    *
 *    this list of conditions and the following disclaimer in the documentation    *
 *    and/or other materials provided with the distribution.                       *
 *                                                                                 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND *
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED   *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          *
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES  *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND     *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                    *
 ***********************************************************************************/

#ifndef BENCHMARK_H
#define BENCHMARK_H

#ifdef BENCH
#include <ctime>
#include <chrono>
#include <iostream>

#include <cstdlib>
#include <unordered_map>
#include <atomic>

namespace BENCHMARK {
    static std::chrono::high_resolution_clock::time_point t_start, t_end;
    static std::clock_t c_start, c_end;

    std::atomic_bool trace_alloc;
    static bool am_i_tracing_alloc;
    std::unordered_map<void *, size_t> mem_map;
    size_t mem_allocated;
    size_t max_mem_usage;
}
#ifdef __cplusplus
#undef new
#endif

inline void * operatorNew( size_t size ) {
    using namespace BENCHMARK;
    void * p = malloc (size);
    if (p == nullptr) throw std::bad_alloc();
    if (trace_alloc.exchange(false)) {
        mem_map.emplace(p, size);
        mem_allocated += size;
        max_mem_usage = std::max(max_mem_usage, mem_allocated);
        trace_alloc.store(true);
    }
    return p;
}

inline void operatorDelete( void * p ) noexcept {
    using namespace BENCHMARK;
    auto it = mem_map.find(p);
    if(it == mem_map.end()) return;
    mem_allocated -= it->second;
    mem_map.erase(it);
    free(p);
}

void * operator new (size_t size)
{
    return operatorNew(size);
}

void * operator new[] (size_t size)
{
    return operatorNew(size);
}

inline void StartAllocTrace() {
    using namespace BENCHMARK;
    if (trace_alloc.exchange(true)) return; //Somebody is tracing already.

    mem_map.clear();
    mem_allocated = 0;
    max_mem_usage = 0;
    am_i_tracing_alloc = true;
}

inline bool StopAllocTrace() {
    using namespace BENCHMARK;
    if (!am_i_tracing_alloc) return false;
    am_i_tracing_alloc = false;
    trace_alloc.store(false);
    return true;
}

inline void BENCHMARK_START() {
    using namespace BENCHMARK;
    t_start = std::chrono::high_resolution_clock::now();
    c_start = std::clock();
    StartAllocTrace();
}

inline void BENCHMARK_FINISH( std::ostream& OUT_STREAM, const char NAME[] ) {
    using namespace BENCHMARK;
    c_end = std::clock();
    t_end = std::chrono::high_resolution_clock::now();
    OUT_STREAM   << "BENCHMARK \"" << NAME << "\" : "
                 << std::fixed << "CPU time: "
                 << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms; "
                 << "Wall time: "
                 << std::chrono::duration<double, std::milli>(t_end-t_start).count()
                 << " ms";
    if(StopAllocTrace())
        OUT_STREAM << " HEAP MEM : " << max_mem_usage << " bytes";
    OUT_STREAM << std::endl;
   }
#define BENCHMARK_STOP() BENCHMARK_FINISH( std::cerr, __PRETTY_FUNCTION__ );
#else
#define BENCHMARK_START()
#define BENCHMARK_STOP()
#define BENCHMARK_FINISH( OUT_STREAM, NAME )
#endif


#endif // BENCHMARK_H
