#ifndef CANVASDEVICE_H_INCLUDED
#define CANVASDEVICE_H_INCLUDED

#include <stdio.h>

#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#define R_USE_PROTOTYPES 1

#include "canvasFont.h"

#if R_VERSION >= R_Version(2,8,0)
#ifndef NewDevDesc
#define NewDevDesc DevDesc
#endif
#endif

#define CANVAS_NAMESPACE R_FindNamespace(mkString("canvasDevice"))

#define canvasColor(fp, prop, col) { \
    if (R_ALPHA(col)==0) { \
        fprintf(fp, "ctx.%s = \"rgb(%d,%d,%d)\"; ",prop, R_RED(col), R_GREEN(col), R_BLUE(col)); \
    } else { \
        fprintf(fp, "ctx.%s = \"rgba(%d,%d,%d,%f)\"; ", prop, R_RED(col), R_GREEN(col), R_BLUE(col), \
            ((double)(R_ALPHA(col)))/255.); \
    }; \
}

typedef struct _canvasDesc {
    /* device specific stuff */
    int col;
    int fill;

    /* Line characteristics */
    double lwd;
    int lty;
    R_GE_lineend lend;
    R_GE_linejoin ljoin;
    double lmitre;

    FILE *outputFile;
    pGEDevDesc graphicsEngine;

    Rboolean debug;

    pFontInfo font;
    SEXP pkgEnv;         /* environment storing objects in R,
                            used to retrieve font objects,
                            defined in font.R */

} canvasDesc;

typedef canvasDesc* pCanvasDesc;

SEXP canvasNewDevice(SEXP filename_r, SEXP width_r, SEXP height_r, SEXP bg_r, SEXP fg_r, SEXP env_r);

static void canvasClose(pDevDesc dev);
static void canvasNewPage(const pGEcontext gc, pDevDesc dev);
static void canvasClip(double x0, double x1, double y0, double y1, pDevDesc dev);
static void canvasSize(double *left, double *right, double *bottom, double *top, pDevDesc dev);
static void canvasMetricInfo(int c, const pGEcontext gc, double* ascent, double* descent, double* width, pDevDesc dev);
static double canvasStrWidth(const char *str, const pGEcontext gc, pDevDesc dev);
static void canvasText(double x, double y, const char *str, double rot, double hadj, const pGEcontext gc, pDevDesc dev);
static void canvasCircle(double x, double y, double r, const pGEcontext gc, pDevDesc dev);
static void canvasRect(double x0, double y0, double x1, double y1, const pGEcontext gc, pDevDesc dev);
static void canvasLine(double x1, double y1, double x2, double y2, const pGEcontext gc, pDevDesc dev);
static void canvasPolyline(int n, double *x, double *y, const pGEcontext gc, pDevDesc dev);
static void canvasPolygon(int n, double *x, double *y, const pGEcontext gc, pDevDesc dev);
static void canvasMode(int mode, pDevDesc dev);
static void canvasActivate(const pDevDesc dev);
static void canvasDeactivate(pDevDesc dev);
static void canvasSetLineType( canvasDesc *cGD, pGEcontext gc);

static size_t utf8toucs(wchar_t *wc, const char *s);
static int utf8towcs(wchar_t *wc, const char *s, int n);
static pFontInfo canvasGetFontDesc(pCanvasDesc canvasInfo, const pGEcontext gc);


#endif