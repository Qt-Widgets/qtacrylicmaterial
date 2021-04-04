/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QtCore/qglobal.h>

#ifndef QTACRYLICHELPER_API
#ifdef QTACRYLICHELPER_STATIC
#define QTACRYLICHELPER_API
#else
#ifdef QTACRYLICHELPER_BUILD_LIBRARY
#define QTACRYLICHELPER_API Q_DECL_EXPORT
#else
#define QTACRYLICHELPER_API Q_DECL_IMPORT
#endif
#endif
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_WINDOWS)
#define Q_OS_WINDOWS
#endif

#ifndef Q_DISABLE_MOVE
#define Q_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Q_DISABLE_MOVE(Class)
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#define qAsConst(i) std::as_const(i)
#endif

namespace _qam::Global {

[[maybe_unused]] const char _qam_blurEnabled_flag[] = "_QTACRYLICMATERIAL_BLUR_ENABLED";
[[maybe_unused]] const char _qam_blurMode_flag[] = "_QTACRYLICMATERIAL_BLUR_MODE";
[[maybe_unused]] const char _qam_gradientColor_flag[] = "_QTACRYLICMATERIAL_BLUR_GRADIENT_COLOR";
[[maybe_unused]] const char _qam_disableExtraProcess[] = "_QTACRYLICMATERIAL_DISABLE_EXTRA_PROCESS_FOR_BLUR";
[[maybe_unused]] const char _qam_forceEnableOfficialMSWin10AcrylicBlur_flag[] = "_QTACRYLICMATERIAL_FORCE_ENABLE_MSWIN10_OFFICIAL_ACRYLIC_BLUR";
[[maybe_unused]] const char _qam_forceEnableTraditionalBlur_flag[] = "_QTACRYLICMATERIAL_FORCE_ENABLE_TRADITIONAL_BLUR";
[[maybe_unused]] const char _qam_forceDisableWallpaperBlur_flag[] = "_QTACRYLICMATERIAL_FORCE_DISABLE_WALLPAPER_BLUR";

}
