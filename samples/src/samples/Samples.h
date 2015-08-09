/*
 * Copyright (c) 2014 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "WormBuncher.h"
#include "Quaternions.h"
#include "Repetition.h"
#include "BezierConstruction.h"
#include "SlideAndBounce.h"

using SampleRef = std::shared_ptr<pk::Scene>;
using SampleFn = std::function<SampleRef ()>;

const std::vector<std::pair<std::string, SampleFn>> SampleList =
{
  { "Slide", std::make_shared<SlideAndBounce> },
  { "Bezier", std::make_shared<BezierConstruction> },
  { "Repetition", std::make_shared<Repetition> },
  { "Worm Bunch", std::make_shared<WormBuncher> },
  { "Dev Junk", std::make_shared<Quaternions> }
};

const std::vector<std::string> SampleNames = ([]
{
  std::vector<std::string> names;
  for( auto &pair : SampleList )
  {
    names.push_back( pair.first );
  }
  return names;
})();
