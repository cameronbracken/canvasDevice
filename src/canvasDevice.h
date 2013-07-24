#include <stdio.h>

#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#include "freetypeMetrics.h"

#if R_VERSION >= R_Version(2,8,0)
#ifndef NewDevDesc
#define NewDevDesc DevDesc
#endif
#endif

#define CANVAS_NAMESPACE R_FindNamespace(mkString("canvasDevice"))

#define CREDC(C) (((unsigned int)(C))&0xff)
#define CGREENC(C) ((((unsigned int)(C))&0xff00)>>8)
#define CBLUEC(C) ((((unsigned int)(C))&0xff0000)>>16)
#define CALPHA(C) ((((unsigned int)(C))&0xff000000)>>24)

#define canvasColor(fp, prop, col) { \
    if (CALPHA(col)==255) { \
        fprintf(fp, "ctx.%s = \"rgb(%d,%d,%d)\"; ",prop, CREDC(col), CGREENC(col), CBLUEC(col)); \
    } else { \
        fprintf(fp, "ctx.%s = \"rgba(%d,%d,%d,%f)\"; ", prop, CREDC(col), CGREENC(col), CBLUEC(col), \
            ((double)CALPHA(col))/255.); \
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

    FILE *fp;
    pGEDevDesc RGE;

#ifdef CANVASDEBUG
    Rboolean debug = TRUE;
#else
    Rboolean debug = FALSE;
#endif

} canvasDesc;