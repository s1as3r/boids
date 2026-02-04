/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */
#ifndef _H_EROSION_RAND
#define _H_EROSION_RAND

// clang-format off
#include "base.h"
// clang-format on

typedef struct {
  u64 state;
  u64 inc;
} PCG32RandomState;

PCG32RandomState g_pcg32_state = {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL};

u32 pcg32_random_r(PCG32RandomState *rng);
u32 pcg32_random(void);

f32 pcg32_randomf(void);
f32 pcg32_randomf_r(PCG32RandomState *rng);

u32 pcg32_boundedrand_r(PCG32RandomState *rng, u32 bound);
u32 pcg32_boundedrand(u32 bound);
#endif // _H_EROSION_RAND
