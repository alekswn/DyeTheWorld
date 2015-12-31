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
    static std::chrono::high_resolution_clock::time_point t_start, t_end;
    static std::clock_t c_start, c_end;
inline void BENCHMARK_START() {
    t_start = std::chrono::high_resolution_clock::now(); \
    c_start = std::clock();
}
inline void BENCHMARK_FINISH( std::ostream& OUT_STREAM, const char NAME[] ) {
    c_end = std::clock(); \
    t_end = std::chrono::high_resolution_clock::now(); \
    OUT_STREAM   << "BENCHMARK \"" << NAME << "\" : " \
                 << std::fixed << "CPU time: " \
                 << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms; " \
                 << "Wall time: " \
                 << std::chrono::duration<double, std::milli>(t_end-t_start).count() \
                 << " ms\n";
   }
#define BENCHMARK_STOP() BENCHMARK_FINISH( std::cerr, __PRETTY_FUNCTION__ );
#else
#define BENCHMARK_START()
#define BENCHMARK_STOP()
#define BENCHMARK_FINISH( OUT_STREAM, NAME )
#endif


#endif // BENCHMARK_H
