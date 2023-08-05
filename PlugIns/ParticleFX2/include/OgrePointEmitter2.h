/*
-----------------------------------------------------------------------------
This source file is part of OGRE-Next
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2023 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef OgrePointEmitter2_H
#define OgrePointEmitter2_H

#include "OgreParticleFX2Prerequisites.h"

#include "ParticleSystem/OgreEmitter2.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    OGRE_ASSUME_NONNULL_BEGIN

    class _OgreParticleFX2Export PointEmitter2 : public EmitterDefData
    {
    public:
        void initEmittedParticles( ParticleCpuData cpuData, const uint32 *newHandles,
                                   size_t numParticles ) override;
    };

    class _OgrePrivate PointEmitterFactory2 final : public ParticleEmitterDefDataFactory
    {
    public:
        const String &getName() const override;

        EmitterDefData *createEmitter() override;
    };

    OGRE_ASSUME_NONNULL_END
}  // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif