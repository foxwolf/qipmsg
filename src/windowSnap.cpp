// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "windowSnap.h"

#include <QtGlobal>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <CoreGraphics/CoreGraphics.h>
#elif defined(Q_OS_LINUX)
#include <dlfcn.h>
#endif

namespace {

QVector<QRect> platformRects(WId skip)
{
    QVector<QRect> rects;
#if defined(Q_OS_WIN)
    typedef HRESULT (WINAPI *DwmGetFn)(HWND, DWORD, PVOID, DWORD);
    struct EnumContext {
        QVector<QRect> *out;
        HWND skipHwnd;
        DwmGetFn dwmGet;
    };
    struct WindowEnum {
        static BOOL CALLBACK collect(HWND hwnd, LPARAM param)
        {
            EnumContext *ctx = reinterpret_cast<EnumContext *>(param);
            if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) {
                return TRUE;
            }
            if (ctx->skipHwnd && hwnd == ctx->skipHwnd) {
                return TRUE;
            }
            if (GetAncestor(hwnd, GA_ROOT) != hwnd) {
                return TRUE;
            }
            const LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            if ((style & WS_EX_TOOLWINDOW) != 0) {
                return TRUE;
            }
            if (ctx->dwmGet) {
                DWORD cloaked = 0;
                if (SUCCEEDED(ctx->dwmGet(hwnd, 14, &cloaked, sizeof(cloaked)))
                        && cloaked != 0) {
                    return TRUE;
                }
            }
            RECT wr;
            if (!GetWindowRect(hwnd, &wr)) {
                return TRUE;
            }
            const int width = wr.right - wr.left;
            const int height = wr.bottom - wr.top;
            if (width < 32 || height < 32) {
                return TRUE;
            }
            ctx->out->append(QRect(wr.left, wr.top, width, height));
            return TRUE;
        }
    };
    EnumContext ctx;
    ctx.out = &rects;
    ctx.skipHwnd = reinterpret_cast<HWND>(skip);
    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    ctx.dwmGet = dwm
            ? reinterpret_cast<DwmGetFn>(GetProcAddress(dwm, "DwmGetWindowAttribute"))
            : 0;
    EnumWindows(WindowEnum::collect, reinterpret_cast<LPARAM>(&ctx));
    if (dwm) {
        FreeLibrary(dwm);
    }
#elif defined(Q_OS_MACOS)
    const CGWindowID skipId = (CGWindowID)skip;
    CFArrayRef list = CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
            kCGNullWindowID);
    if (list) {
        const CFIndex count = CFArrayGetCount(list);
        for (CFIndex i = 0; i < count; ++i) {
            CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(list, i);
            if (!info) {
                continue;
            }
            int layer = 0;
            CFNumberRef layerNum = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowLayer);
            if (layerNum) {
                CFNumberGetValue(layerNum, kCFNumberIntType, &layer);
            }
            if (layer != 0) {
                continue;
            }
            int number = 0;
            CFNumberRef numberNum = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);
            if (numberNum) {
                CFNumberGetValue(numberNum, kCFNumberIntType, &number);
            }
            if (skipId != 0 && (CGWindowID)number == skipId) {
                continue;
            }
            CFDictionaryRef bounds = (CFDictionaryRef)CFDictionaryGetValue(info, kCGWindowBounds);
            CGRect cg;
            if (!bounds || !CGRectMakeWithDictionaryRepresentation(bounds, &cg)) {
                continue;
            }
            if (cg.size.width < 32 || cg.size.height < 32) {
                continue;
            }
            rects.append(QRect(qRound(cg.origin.x), qRound(cg.origin.y),
                               qRound(cg.size.width), qRound(cg.size.height)));
        }
        CFRelease(list);
    }
#elif defined(Q_OS_LINUX)
    Q_UNUSED(skip);
    void *lib = dlopen("libX11.so.6", RTLD_LAZY);
    if (!lib) {
        return rects;
    }
    typedef struct _XDisplay Display;
    typedef Display *(*OpenFn)(const char *);
    typedef int (*CloseFn)(Display *);
    typedef int (*ScreenFn)(Display *);
    typedef unsigned long (*RootFn)(Display *, int);
    typedef unsigned long (*AtomFn)(Display *, const char *, int);
    typedef int (*PropFn)(Display *, unsigned long, unsigned long, long, long, int,
                          unsigned long, unsigned long *, int *, unsigned long *,
                          unsigned long *, unsigned char **);
    typedef int (*FreeFn)(void *);
    typedef int (*GeomFn)(Display *, unsigned long, unsigned long *, int *, int *,
                          unsigned int *, unsigned int *, unsigned int *, unsigned int *);
    typedef int (*TranslateFn)(Display *, unsigned long, unsigned long, int, int,
                               int *, int *, unsigned long *);

    OpenFn openDisplay = reinterpret_cast<OpenFn>(dlsym(lib, "XOpenDisplay"));
    CloseFn closeDisplay = reinterpret_cast<CloseFn>(dlsym(lib, "XCloseDisplay"));
    ScreenFn defaultScreen = reinterpret_cast<ScreenFn>(dlsym(lib, "XDefaultScreen"));
    RootFn rootWindow = reinterpret_cast<RootFn>(dlsym(lib, "XRootWindow"));
    AtomFn internAtom = reinterpret_cast<AtomFn>(dlsym(lib, "XInternAtom"));
    PropFn getProperty = reinterpret_cast<PropFn>(dlsym(lib, "XGetWindowProperty"));
    FreeFn freeData = reinterpret_cast<FreeFn>(dlsym(lib, "XFree"));
    GeomFn getGeometry = reinterpret_cast<GeomFn>(dlsym(lib, "XGetGeometry"));
    TranslateFn translate = reinterpret_cast<TranslateFn>(dlsym(lib, "XTranslateCoordinates"));
    if (!openDisplay || !closeDisplay || !defaultScreen || !rootWindow || !internAtom
            || !getProperty || !freeData || !getGeometry || !translate) {
        dlclose(lib);
        return rects;
    }
    Display *display = openDisplay(0);
    if (!display) {
        dlclose(lib);
        return rects;
    }
    const int screen = defaultScreen(display);
    const unsigned long root = rootWindow(display, screen);
    const unsigned long atom = internAtom(display, "_NET_CLIENT_LIST", 0);
    unsigned char *data = 0;
    unsigned long actualType = 0;
    int actualFormat = 0;
    unsigned long count = 0;
    unsigned long after = 0;
    const int status = getProperty(display, root, atom, 0, 1024, 0, 33,
                                   &actualType, &actualFormat, &count, &after, &data);
    if (status == 0 && data && actualFormat == 32) {
        unsigned long *ids = reinterpret_cast<unsigned long *>(data);
        const unsigned long limit = count > 256 ? 256 : count;
        for (unsigned long i = 0; i < limit; ++i) {
            const unsigned long window = ids[i];
            if (window == 0) {
                continue;
            }
            int dx = 0;
            int dy = 0;
            unsigned long child = 0;
            if (!translate(display, window, root, 0, 0, &dx, &dy, &child)) {
                continue;
            }
            unsigned long geomRoot = 0;
            int gx = 0;
            int gy = 0;
            unsigned int width = 0;
            unsigned int height = 0;
            unsigned int border = 0;
            unsigned int depth = 0;
            if (!getGeometry(display, window, &geomRoot, &gx, &gy, &width, &height,
                             &border, &depth)) {
                continue;
            }
            if (width < 32 || height < 32) {
                continue;
            }
            rects.append(QRect(dx, dy, (int)width, (int)height));
        }
    }
    if (data) {
        freeData(data);
    }
    closeDisplay(display);
    dlclose(lib);
#else
    Q_UNUSED(skip);
#endif
    return rects;
}

int nearestEdge(int value, const QVector<int> &edges, int thresholdPx)
{
    int best = value;
    int bestDist = thresholdPx + 1;
    for (int i = 0; i < edges.size(); ++i) {
        const int dist = qAbs(value - edges.at(i));
        if (dist <= thresholdPx && dist < bestDist) {
            bestDist = dist;
            best = edges.at(i);
        }
    }
    return best;
}

} // namespace

QVector<QRect> WindowSnap::topLevelRects(const QRect &screenGlobal, WId skip)
{
    QVector<QRect> local;
    const QRect screenLocal(QPoint(0, 0), screenGlobal.size());
    if (screenGlobal.width() < 2 || screenGlobal.height() < 2) {
        return local;
    }
    const QVector<QRect> global = platformRects(skip);
    for (int i = 0; i < global.size(); ++i) {
        const QRect shifted = global.at(i).translated(-screenGlobal.topLeft());
        const QRect hit = shifted.intersected(screenLocal);
        if (hit.width() >= 32 && hit.height() >= 32) {
            local.append(hit);
        }
    }
    local.append(screenLocal);
    return local;
}

QRect WindowSnap::snapSelection(const QRect &raw, const QVector<QRect> &targets,
                                int thresholdPx, bool disabled)
{
    const QRect rect = raw.normalized();
    if (disabled || thresholdPx < 1 || rect.width() < 1 || rect.height() < 1) {
        return rect;
    }

    QVector<int> xs;
    QVector<int> ys;
    for (int i = 0; i < targets.size(); ++i) {
        const QRect target = targets.at(i);
        if (!target.isValid()) {
            continue;
        }
        xs << target.left() << target.right();
        ys << target.top() << target.bottom();
    }

    const int left = nearestEdge(rect.left(), xs, thresholdPx);
    const int right = nearestEdge(rect.right(), xs, thresholdPx);
    const int top = nearestEdge(rect.top(), ys, thresholdPx);
    const int bottom = nearestEdge(rect.bottom(), ys, thresholdPx);

    int useLeft = rect.left();
    int useRight = rect.right();
    int useTop = rect.top();
    int useBottom = rect.bottom();
    if (right - left >= 8) {
        useLeft = left;
        useRight = right;
    }
    if (bottom - top >= 8) {
        useTop = top;
        useBottom = bottom;
    }
    return QRect(QPoint(useLeft, useTop), QPoint(useRight, useBottom)).normalized();
}
