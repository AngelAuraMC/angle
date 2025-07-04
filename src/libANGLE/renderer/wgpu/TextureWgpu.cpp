//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureWgpu.cpp:
//    Implements the class methods for TextureWgpu.
//

#include "libANGLE/renderer/wgpu/TextureWgpu.h"

#include "common/PackedGLEnums_autogen.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/ImageWgpu.h"
#include "libANGLE/renderer/wgpu/RenderTargetWgpu.h"

namespace rx
{

namespace
{

constexpr angle::SubjectIndex kTextureImageSubjectIndex = 0;

void GetRenderTargetLayerCountAndIndex(webgpu::ImageHelper *image,
                                       const gl::ImageIndex &index,
                                       GLuint *layerIndex,
                                       GLuint *layerCount,
                                       GLuint *imageLayerCount)
{
    *layerIndex = index.hasLayer() ? index.getLayerIndex() : 0;
    *layerCount = index.getLayerCount();

    switch (index.getType())
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::External:
            ASSERT(*layerIndex == 0 &&
                   (*layerCount == 1 ||
                    *layerCount == static_cast<GLuint>(gl::ImageIndex::kEntireLevel)));
            *imageLayerCount = 1;
            break;

        case gl::TextureType::CubeMap:
            ASSERT(!index.hasLayer() ||
                   *layerIndex == static_cast<GLuint>(index.cubeMapFaceIndex()));
            *imageLayerCount = gl::kCubeFaceCount;
            break;

        case gl::TextureType::_3D:
        {
            gl::LevelIndex levelGL(index.getLevelIndex());
            *imageLayerCount = image->getTextureDescriptor().size.depthOrArrayLayers;
            break;
        }

        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisampleArray:
        case gl::TextureType::CubeMapArray:
            // NOTE: Not yet supported, should set *imageLayerCount.
            UNIMPLEMENTED();
            break;

        default:
            UNREACHABLE();
    }

    if (*layerCount == static_cast<GLuint>(gl::ImageIndex::kEntireLevel))
    {
        ASSERT(*layerIndex == 0);
        *layerCount = *imageLayerCount;
    }
}

bool IsTextureLevelDefinitionCompatibleWithImage(webgpu::ImageHelper *image,
                                                 const gl::Extents &size,
                                                 const webgpu::Format &format)
{
    return size == wgpu_gl::GetExtents(image->getSize()) &&
           image->getIntendedFormatID() == format.getIntendedFormatID() &&
           image->getActualFormatID() == format.getActualImageFormatID();
}

}  // namespace

TextureWgpu::TextureWgpu(const gl::TextureState &state)
    : TextureImpl(state),
      mCurrentBaseLevel(state.getBaseLevel()),
      mCurrentMaxLevel(state.getMaxLevel()),
      mImageObserverBinding(this, kTextureImageSubjectIndex)
{
    setImageHelper(new webgpu::ImageHelper(), true);
}

TextureWgpu::~TextureWgpu() {}

void TextureWgpu::onDestroy(const gl::Context *context)
{
    setImageHelper(nullptr, true);
}

angle::Result TextureWgpu::setImage(const gl::Context *context,
                                    const gl::ImageIndex &index,
                                    GLenum internalFormat,
                                    const gl::Extents &size,
                                    GLenum format,
                                    GLenum type,
                                    const gl::PixelUnpackState &unpack,
                                    gl::Buffer *unpackBuffer,
                                    const uint8_t *pixels)
{
    return setImageImpl(context, internalFormat, type, index, size, unpack, pixels);
}

angle::Result TextureWgpu::setSubImage(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::Box &area,
                                       GLenum format,
                                       GLenum type,
                                       const gl::PixelUnpackState &unpack,
                                       gl::Buffer *unpackBuffer,
                                       const uint8_t *pixels)
{
    ContextWgpu *contextWgpu             = GetImplAs<ContextWgpu>(context);
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, type);
    return setSubImageImpl(context, contextWgpu->getFormat(formatInfo.sizedInternalFormat), type,
                           index, area, unpack, pixels);
}

angle::Result TextureWgpu::setCompressedImage(const gl::Context *context,
                                              const gl::ImageIndex &index,
                                              GLenum internalFormat,
                                              const gl::Extents &size,
                                              const gl::PixelUnpackState &unpack,
                                              size_t imageSize,
                                              const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setCompressedSubImage(const gl::Context *context,
                                                 const gl::ImageIndex &index,
                                                 const gl::Box &area,
                                                 GLenum format,
                                                 const gl::PixelUnpackState &unpack,
                                                 size_t imageSize,
                                                 const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Rectangle &sourceArea,
                                     GLenum internalFormat,
                                     gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copySubImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Offset &destOffset,
                                        const gl::Rectangle &sourceArea,
                                        gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyTexture(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       GLenum internalFormat,
                                       GLenum type,
                                       GLint sourceLevel,
                                       bool unpackFlipY,
                                       bool unpackPremultiplyAlpha,
                                       bool unpackUnmultiplyAlpha,
                                       const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copySubTexture(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &destOffset,
                                          GLint sourceLevel,
                                          const gl::Box &sourceBox,
                                          bool unpackFlipY,
                                          bool unpackPremultiplyAlpha,
                                          bool unpackUnmultiplyAlpha,
                                          const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyRenderbufferSubData(const gl::Context *context,
                                                   const gl::Renderbuffer *srcBuffer,
                                                   GLint srcLevel,
                                                   GLint srcX,
                                                   GLint srcY,
                                                   GLint srcZ,
                                                   GLint dstLevel,
                                                   GLint dstX,
                                                   GLint dstY,
                                                   GLint dstZ,
                                                   GLsizei srcWidth,
                                                   GLsizei srcHeight,
                                                   GLsizei srcDepth)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyTextureSubData(const gl::Context *context,
                                              const gl::Texture *srcTexture,
                                              GLint srcLevel,
                                              GLint srcX,
                                              GLint srcY,
                                              GLint srcZ,
                                              GLint dstLevel,
                                              GLint dstX,
                                              GLint dstY,
                                              GLint dstZ,
                                              GLsizei srcWidth,
                                              GLsizei srcHeight,
                                              GLsizei srcDepth)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyCompressedTexture(const gl::Context *context,
                                                 const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorage(const gl::Context *context,
                                      gl::TextureType type,
                                      size_t levels,
                                      GLenum internalFormat,
                                      const gl::Extents &size)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorageExternalMemory(const gl::Context *context,
                                                    gl::TextureType type,
                                                    size_t levels,
                                                    GLenum internalFormat,
                                                    const gl::Extents &size,
                                                    gl::MemoryObject *memoryObject,
                                                    GLuint64 offset,
                                                    GLbitfield createFlags,
                                                    GLbitfield usageFlags,
                                                    const void *imageCreateInfoPNext)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setEGLImageTarget(const gl::Context *context,
                                             gl::TextureType type,
                                             egl::Image *image)
{
    ImageWgpu *imageWgpu = webgpu::GetImpl(image);
    setImageHelper(imageWgpu->getImage(), false);
    ASSERT(mImage->isInitialized());

    return angle::Result::Continue;
}

angle::Result TextureWgpu::setImageExternal(const gl::Context *context,
                                            gl::TextureType type,
                                            egl::Stream *stream,
                                            const egl::Stream::GLTextureDescription &desc)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::generateMipmap(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::releaseTexImage(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::syncState(const gl::Context *context,
                                     const gl::Texture::DirtyBits &dirtyBits,
                                     gl::Command source)
{
    ContextWgpu *contextWgpu = GetImplAs<ContextWgpu>(context);
    ANGLE_TRY(respecifyImageStorageIfNecessary(contextWgpu, source));
    const bool isGenerateMipmap = source == gl::Command::GenerateMipmap;
    ANGLE_TRY(initializeImage(contextWgpu, isGenerateMipmap
                                               ? ImageMipLevels::FullMipChainForGenerateMipmap
                                               : ImageMipLevels::EnabledLevels));
    ANGLE_TRY(mImage->flushStagedUpdates(contextWgpu));
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorageMultisample(const gl::Context *context,
                                                 gl::TextureType type,
                                                 GLsizei samples,
                                                 GLint internalformat,
                                                 const gl::Extents &size,
                                                 bool fixedSampleLocations)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::initializeContents(const gl::Context *context,
                                              GLenum binding,
                                              const gl::ImageIndex &imageIndex)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::getAttachmentRenderTarget(const gl::Context *context,
                                                     GLenum binding,
                                                     const gl::ImageIndex &imageIndex,
                                                     GLsizei samples,
                                                     FramebufferAttachmentRenderTarget **rtOut)
{
    ContextWgpu *contextWgpu = GetImplAs<ContextWgpu>(context);
    ANGLE_TRY(respecifyImageStorageIfNecessary(contextWgpu, gl::Command::Draw));
    if (!mImage->isInitialized())
    {
        ANGLE_TRY(initializeImage(contextWgpu, ImageMipLevels::EnabledLevels));
    }

    GLuint layerIndex = 0, layerCount = 0, imageLayerCount = 0;
    GetRenderTargetLayerCountAndIndex(mImage, imageIndex, &layerIndex, &layerCount,
                                      &imageLayerCount);

    // NOTE: Multisampling not yet supported
    ASSERT(samples <= 1);
    const gl::RenderToTextureImageIndex renderToTextureIndex =
        gl::RenderToTextureImageIndex::Default;

    if (layerCount == 1)
    {
        ANGLE_TRY(initSingleLayerRenderTargets(contextWgpu, imageLayerCount,
                                               gl::LevelIndex(imageIndex.getLevelIndex()),
                                               renderToTextureIndex));

        std::vector<std::vector<RenderTargetWgpu>> &levelRenderTargets =
            mSingleLayerRenderTargets[renderToTextureIndex];
        ASSERT(imageIndex.getLevelIndex() < static_cast<int32_t>(levelRenderTargets.size()));

        std::vector<RenderTargetWgpu> &layerRenderTargets =
            levelRenderTargets[imageIndex.getLevelIndex()];
        ASSERT(imageIndex.getLayerIndex() < static_cast<int32_t>(layerRenderTargets.size()));

        *rtOut = &layerRenderTargets[layerIndex];
    }
    else
    {
        // Not yet supported.
        UNIMPLEMENTED();
    }

    return angle::Result::Continue;
}

void TextureWgpu::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    ASSERT(index == kTextureImageSubjectIndex &&
           (message == angle::SubjectMessage::SubjectChanged ||
            message == angle::SubjectMessage::InitializationComplete));

    // Forward the notification to the parent that the staging buffer changed.
    onStateChange(message);
}

angle::Result TextureWgpu::ensureImageInitialized(const gl::Context *context)
{
    return initializeImage(webgpu::GetImpl(context), ImageMipLevels::EnabledLevels);
}

void TextureWgpu::releaseOwnershipOfImage(const gl::Context *context)
{
    mOwnsImage = false;
    setImageHelper(nullptr, true);
}

angle::Result TextureWgpu::setImageImpl(const gl::Context *context,
                                        GLenum internalFormat,
                                        GLenum type,
                                        const gl::ImageIndex &index,
                                        const gl::Extents &size,
                                        const gl::PixelUnpackState &unpack,
                                        const uint8_t *pixels)
{
    ContextWgpu *contextWgpu           = GetImplAs<ContextWgpu>(context);
    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat, type);
    const webgpu::Format &webgpuFormat =
        contextWgpu->getFormat(internalFormatInfo.sizedInternalFormat);
    ANGLE_TRY(redefineLevel(context, webgpuFormat, index, size));
    return setSubImageImpl(context, webgpuFormat, type, index, gl::Box(gl::kOffsetZero, size),
                           unpack, pixels);
}

angle::Result TextureWgpu::setSubImageImpl(const gl::Context *context,
                                           const webgpu::Format &webgpuFormat,
                                           GLenum type,
                                           const gl::ImageIndex &index,
                                           const gl::Box &area,
                                           const gl::PixelUnpackState &unpack,
                                           const uint8_t *pixels)
{
    ContextWgpu *contextWgpu = GetImplAs<ContextWgpu>(context);

    if (!webgpuFormat.valid())
    {
        UNIMPLEMENTED();
        return angle::Result::Continue;
    }

    const gl::InternalFormat &inputInternalFormatInfo = webgpuFormat.getInternalFormatInfo(type);
    gl::Extents glExtents                 = gl::Extents(area.width, area.height, area.depth);

    GLuint inputRowPitch = 0;
    ANGLE_CHECK_GL_MATH(contextWgpu, inputInternalFormatInfo.computeRowPitch(
                                         type, glExtents.width, unpack.alignment, unpack.rowLength,
                                         &inputRowPitch));

    GLuint inputDepthPitch = 0;
    ANGLE_CHECK_GL_MATH(
        contextWgpu, inputInternalFormatInfo.computeDepthPitch(glExtents.height, unpack.imageHeight,
                                                               inputRowPitch, &inputDepthPitch));

    const angle::Format &actualFormat = webgpuFormat.getActualImageFormat();
    uint32_t outputRowPitch           = roundUp(actualFormat.pixelBytes * glExtents.width,
                                                static_cast<uint32_t>(webgpu::kTextureRowSizeAlignment));
    uint32_t outputDepthPitch         = outputRowPitch * glExtents.height;
    uint32_t allocationSize           = outputDepthPitch * glExtents.depth;

    // TODO(anglebug.com/389145696): ignores area.x|y|z
    ANGLE_TRY(mImage->stageTextureUpload(contextWgpu, webgpuFormat, type, glExtents, inputRowPitch,
                                         inputDepthPitch, outputRowPitch, outputDepthPitch,
                                         allocationSize, index, pixels));
    return angle::Result::Continue;
}

angle::Result TextureWgpu::initializeImage(ContextWgpu *contextWgpu, ImageMipLevels mipLevels)
{
    if (mImage->isInitialized())
    {
        return angle::Result::Continue;
    }

    const DawnProcTable *wgpu = webgpu::GetProcs(contextWgpu);

    const webgpu::Format &webgpuFormat      = getBaseLevelFormat(contextWgpu);
    DisplayWgpu *displayWgpu                = contextWgpu->getDisplay();
    const gl::ImageDesc *firstLevelDesc     = &mState.getBaseLevelDesc();
    uint32_t levelCount                     = getMipLevelCount(mipLevels);
    gl::LevelIndex firstLevel               = gl::LevelIndex(mState.getEffectiveBaseLevel());
    WGPUExtent3D wgpuExtents                = gl_wgpu::GetExtent3D(firstLevelDesc->size);
    WGPUTextureDimension textureDimension   = gl_wgpu::GetWgpuTextureDimension(mState.getType());
    WGPUTextureUsage textureUsage           = WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst |
                                    WGPUTextureUsage_RenderAttachment |
                                    WGPUTextureUsage_TextureBinding;

    if (mState.getType() == gl::TextureType::CubeMap)
    {
        ASSERT(wgpuExtents.depthOrArrayLayers == 1);
        ASSERT(wgpuExtents.width == wgpuExtents.height);
        wgpuExtents.depthOrArrayLayers = 6;
    }

    return mImage->initImage(
        wgpu, webgpuFormat.getIntendedFormatID(), webgpuFormat.getActualImageFormatID(),
        displayWgpu->getDevice(), firstLevel,
        mImage->createTextureDescriptor(
            textureUsage, textureDimension, wgpuExtents,
            webgpu::GetWgpuTextureFormatFromFormatID(webgpuFormat.getActualImageFormatID()),
            levelCount, 1));
}

angle::Result TextureWgpu::redefineLevel(const gl::Context *context,
                                         const webgpu::Format &webgpuFormat,
                                         const gl::ImageIndex &index,
                                         const gl::Extents &size)
{
    if (mImage != nullptr && mOwnsImage)
    {
        // If there are any staged changes for this index, we can remove them since we're going to
        // override them with this call.
        gl::LevelIndex levelIndexGL(index.getLevelIndex());
        const uint32_t layerIndex = index.hasLayer() ? index.getLayerIndex() : 0;

        if (index.hasLayer())
        {
            mImage->removeSingleSubresourceStagedUpdates(levelIndexGL, layerIndex,
                                                         index.getLayerCount());
        }
        else
        {
            mImage->removeStagedUpdates(levelIndexGL);
        }

        if (mImage->isInitialized())
        {
            TextureLevelAllocation levelAllocation =
                mImage->isTextureLevelInAllocatedImage(levelIndexGL)
                    ? TextureLevelAllocation::WithinAllocatedImage
                    : TextureLevelAllocation::OutsideAllocatedImage;
            TextureLevelDefinition levelDefinition =
                IsTextureLevelDefinitionCompatibleWithImage(mImage, size, webgpuFormat)
                    ? TextureLevelDefinition::Compatible
                    : TextureLevelDefinition::Incompatible;
            if (TextureRedefineLevel(levelAllocation, levelDefinition, mState.getImmutableFormat(),
                                     mImage->getLevelCount(), layerIndex, index,
                                     mImage->getFirstAllocatedLevel(), &mRedefinedLevels))
            {
                // TODO(anglebug.com/425449020): release any views or references to this image,
                // including RenderTargets.
                mImage->resetImage();
            }
        }
    }
    else
    {
        setImageHelper(new webgpu::ImageHelper, true);
    }

    return angle::Result::Continue;
}

uint32_t TextureWgpu::getMipLevelCount(ImageMipLevels mipLevels) const
{
    switch (mipLevels)
    {
        // Returns level count from base to max that has been specified, i.e, enabled.
        case ImageMipLevels::EnabledLevels:
            return mState.getEnabledLevelCount();
        // Returns all mipmap levels from base to max regardless if an image has been specified or
        // not.
        case ImageMipLevels::FullMipChainForGenerateMipmap:
            return getMaxLevelCount() - mState.getEffectiveBaseLevel();

        default:
            UNREACHABLE();
            return 0;
    }
}

uint32_t TextureWgpu::getMaxLevelCount() const
{
    // getMipmapMaxLevel will be 0 here if mipmaps are not used, so the levelCount is always +1.
    return mState.getMipmapMaxLevel() + 1;
}

angle::Result TextureWgpu::respecifyImageStorageIfNecessary(ContextWgpu *contextWgpu,
                                                            gl::Command source)
{
    ASSERT(mState.getBuffer().get() == nullptr);

    // Before redefining the image for any reason, check to see if it's about to go through mipmap
    // generation.  In that case, drop every staged change for the subsequent mips after base, and
    // make sure the image is created with the complete mip chain.
    const bool isGenerateMipmap = source == gl::Command::GenerateMipmap;
    if (isGenerateMipmap)
    {
        prepareForGenerateMipmap(contextWgpu);
    }

    // Set base and max level before initializing the image
    ANGLE_TRY(maybeUpdateBaseMaxLevels(contextWgpu));

    // It is possible for the image to have a single level (because it doesn't use mipmapping),
    // then have more levels defined in it and mipmapping enabled.  In that case, the image needs
    // to be recreated.
    bool isMipmapEnabledByMinFilter = false;
    if (!isGenerateMipmap && mImage && mImage->isInitialized())
    {
        isMipmapEnabledByMinFilter =
            mImage->getLevelCount() < getMipLevelCount(ImageMipLevels::EnabledLevels);
    }

    // If generating mipmaps and the image needs to be recreated (not full-mip already, or changed
    // usage flags), make sure it's recreated.
    if (isGenerateMipmap && mImage && mImage->isInitialized() &&
        (!mState.getImmutableFormat() &&
         mImage->getLevelCount() !=
             getMipLevelCount(ImageMipLevels::FullMipChainForGenerateMipmap)))
    {
        ANGLE_TRY(mImage->flushStagedUpdates(contextWgpu));

        mImage->resetImage();
    }

    // Also recreate the image if it's changed in usage, or if any of its levels are redefined and
    // no update to base/max levels were done (otherwise the above call would have already taken
    // care of this).
    // TODO(liza): Respecify the image once copying images is supported.
    if (TextureHasAnyRedefinedLevels(mRedefinedLevels) || isMipmapEnabledByMinFilter)
    {
        ANGLE_TRY(mImage->flushStagedUpdates(contextWgpu));

        mImage->resetImage();
    }

    return angle::Result::Continue;
}

void TextureWgpu::prepareForGenerateMipmap(ContextWgpu *contextWgpu)
{
    gl::LevelIndex baseLevel(mState.getEffectiveBaseLevel());
    gl::LevelIndex maxLevel(mState.getMipmapMaxLevel());

    // Remove staged updates to the range that's being respecified (which is all the mips except
    // baseLevel).
    gl::LevelIndex firstGeneratedLevel = baseLevel + 1;
    for (GLuint levelToRemove = mState.getEffectiveBaseLevel();
         levelToRemove < mState.getMipmapMaxLevel(); levelToRemove++)
    {
        mImage->removeStagedUpdates(gl::LevelIndex(levelToRemove));
    }

    TextureRedefineGenerateMipmapLevels(baseLevel, maxLevel, firstGeneratedLevel,
                                        &mRedefinedLevels);

    // If generating mipmap and base level is incompatibly redefined, the image is going to be
    // recreated.  Don't try to preserve the other mips.
    if (IsTextureLevelRedefined(mRedefinedLevels, mState.getType(), baseLevel))
    {
        ASSERT(!mState.getImmutableFormat());
        mImage->resetImage();
    }
}

angle::Result TextureWgpu::maybeUpdateBaseMaxLevels(ContextWgpu *contextWgpu)
{
    bool baseLevelChanged = mCurrentBaseLevel.get() != static_cast<GLint>(mState.getBaseLevel());
    bool maxLevelChanged  = mCurrentMaxLevel.get() != static_cast<GLint>(mState.getMaxLevel());

    if (!maxLevelChanged && !baseLevelChanged)
    {
        return angle::Result::Continue;
    }

    gl::LevelIndex newBaseLevel = gl::LevelIndex(mState.getEffectiveBaseLevel());
    gl::LevelIndex newMaxLevel  = gl::LevelIndex(mState.getEffectiveMaxLevel());
    ASSERT(newBaseLevel <= newMaxLevel);

    if (!mImage->isInitialized())
    {
        return angle::Result::Continue;
    }

    if (mState.getImmutableFormat())
    {
        // For immutable texture, baseLevel/maxLevel should be a subset of the texture's actual
        // number of mip levels. We don't need to respecify an image.
        ASSERT(!baseLevelChanged || newBaseLevel >= mImage->getFirstAllocatedLevel());
        ASSERT(!maxLevelChanged || newMaxLevel < gl::LevelIndex(mImage->getLevelCount()));
    }
    else if (!baseLevelChanged && (newMaxLevel <= mImage->getLastAllocatedLevel()))
    {
        // With a valid image, check if only changing the maxLevel to a subset of the texture's
        // actual number of mip levels
        ASSERT(maxLevelChanged);
    }
    else
    {
        // TODO(liza): Respecify the image once copying images is supported.
        mImage->resetImage();
        return angle::Result::Continue;
    }

    mCurrentBaseLevel = newBaseLevel;
    mCurrentMaxLevel  = newMaxLevel;

    return angle::Result::Continue;
}

angle::Result TextureWgpu::initSingleLayerRenderTargets(
    ContextWgpu *contextWgpu,
    GLuint layerCount,
    gl::LevelIndex levelIndex,
    gl::RenderToTextureImageIndex renderToTextureIndex)
{
    std::vector<std::vector<RenderTargetWgpu>> &allLevelsRenderTargets =
        mSingleLayerRenderTargets[renderToTextureIndex];

    if (allLevelsRenderTargets.size() <= static_cast<uint32_t>(levelIndex.get()))
    {
        allLevelsRenderTargets.resize(levelIndex.get() + 1);
    }

    std::vector<RenderTargetWgpu> &renderTargets = allLevelsRenderTargets[levelIndex.get()];

    // Lazy init. Check if already initialized.
    if (!renderTargets.empty())
    {
        return angle::Result::Continue;
    }

    // There are |layerCount| render targets, one for each layer
    renderTargets.resize(layerCount);

    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        webgpu::TextureViewHandle textureView;
        ANGLE_TRY(mImage->createTextureViewSingleLevel(levelIndex, layerIndex, textureView));

        renderTargets[layerIndex].set(mImage, textureView, mImage->toWgpuLevel(levelIndex),
                                      layerIndex, mImage->toWgpuTextureFormat());
    }

    return angle::Result::Continue;
}

const webgpu::Format &TextureWgpu::getBaseLevelFormat(ContextWgpu *contextWgpu) const
{
    const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
    return contextWgpu->getFormat(baseLevelDesc.format.info->sizedInternalFormat);
}

void TextureWgpu::setImageHelper(webgpu::ImageHelper *imageHelper, bool ownsImageHelper)
{
    if (mOwnsImage && mImage)
    {
        mImageObserverBinding.bind(nullptr);
        SafeDelete(mImage);
    }

    mImage     = imageHelper;
    mOwnsImage = ownsImageHelper;

    if (mImage)
    {
        mImageObserverBinding.bind(mImage);
    }

    onStateChange(angle::SubjectMessage::SubjectChanged);
}

}  // namespace rx
