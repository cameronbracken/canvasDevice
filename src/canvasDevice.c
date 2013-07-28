#include "canvasDevice.h"

SEXP canvasNewDevice(SEXP filename_r, SEXP width_r, SEXP height_r,
    SEXP bg_r, SEXP fg_r, SEXP env_r)
{

    /*
        Make sure the version number of the R running this
        routine is compatible with the version number of 
        the R that compiled this routine.
    */
    R_GE_checkVersionOrDie(R_GE_version);

    /* R Graphics Device: in GraphicsDevice.h */
    pDevDesc dev;

    /* R Graphics Engine: in GraphicsEngine.h */
    pGEDevDesc graphicsEngine;

    /* canvas Graphics Device */
    canvasDesc *canvasInfo;

    FILE *outputFile;
    const char *fileName = CHAR(STRING_ELT(filename_r, 0));
    const int width = INTEGER(width_r)[0];
    const int height = INTEGER(height_r)[0];
    const char *bg = CHAR(STRING_ELT(bg_r, 0));
    const char *fg = CHAR(STRING_ELT(fg_r, 0));

    
    R_CheckDeviceAvailable();

    // Allocate device 
    if (!(dev = (pDevDesc)calloc(1, sizeof(NewDevDesc)))){
        fclose(outputFile);
        error("calloc failed for canvas device");
    }

    // Allocate device specific into
    if (!(canvasInfo = (canvasDesc *)calloc(1, sizeof(canvasDesc)))){
        free(dev);
        fclose(outputFile);
        error("calloc failed for canvas device");
    }

    // Set up output file pointer
    canvasInfo->outputFile = fopen(R_ExpandFileName(fileName), "w");
    canvasInfo->pkgEnv = env_r;

#ifdef CANVASDEBUG
    canvasInfo->debug = TRUE;
#else
    canvasInfo->debug = FALSE;
#endif

    if( canvasInfo->debug == TRUE )
        Rprintf("canvasNewDevice(width=%d,height=%d,fd=%x)\n", width, height, outputFile);

    dev->deviceSpecific = (void *) canvasInfo;

    /********************************************************
    * Device procedures.
    ********************************************************/
    dev->activate = canvasActivate;
    dev->circle = canvasCircle;
    dev->clip = canvasClip;
    dev->close = canvasClose;
    dev->deactivate = canvasDeactivate;
    dev->locator = NULL;
    dev->line = canvasLine;
    dev->metricInfo = canvasMetricInfo;
    dev->mode = canvasMode;
    dev->newPage = canvasNewPage;
    dev->polygon = canvasPolygon;
    dev->polyline = canvasPolyline;
    dev->rect = canvasRect;
    dev->path = NULL;
    dev->raster = NULL;
    dev->cap = NULL;
    dev->size = canvasSize;
    dev->strWidth = canvasStrWidth;
    dev->text = canvasText;
    dev->onExit = NULL;
    dev->getEvent = NULL;
    dev->newFrameConfirm = NULL;
    dev->hasTextUTF8 = TRUE;
    dev->textUTF8 = canvasText;
    dev->strWidthUTF8 = canvasStrWidth;
    dev->wantSymbolUTF8 = TRUE;
    dev->useRotatedTextInContour = TRUE;
    dev->eventEnv = NULL;
    dev->eventHelper = NULL;
    dev->holdflush = NULL;

    dev->haveTransparency = 2;       /* 1 = no, 2 = yes */
    dev->haveTransparentBg = 1;      /* 1 = no, 2 = fully, 3 = semi */
    dev->haveRaster = 1;             /* 1 = no, 2 = yes, 3 = except for missing values */
    dev->haveCapture = 1;            /* 1 = no, 2 = yes */
    dev->haveLocator = 1;            /* 1 = no, 2 = yes */

    /* Initialise dev */
    dev->left = dev->clipLeft = 0;
    dev->top = dev->clipTop = 0;
    dev->right = dev->clipRight = width;
    dev->bottom = dev->clipBottom = height;
    dev->xCharOffset = 0.4900;
    dev->yCharOffset = 0.3333;
    dev->yLineBias = 0.1;
    dev->ipr[0] = 1.0/72.0;
    dev->ipr[1] = 1.0/72.0;
    dev->cra[0] = 0.9 * 10;
    dev->cra[1] = 1.2 * 10;
    dev->gamma = 1.0;
    dev->canClip = FALSE;
    dev->canChangeGamma = FALSE;
    dev->canHAdj = 2;
    dev->startps = 10.0;
    dev->startfill = R_GE_str2col( bg );
    dev->startcol = R_GE_str2col( fg );
    dev->startlty = LTY_SOLID;
    dev->startfont = 1;
    dev->startgamma = dev->gamma;
    dev->displayListOn = FALSE;

    /* Add to the device list */
    graphicsEngine = GEcreateDevDesc(dev);
    canvasInfo->graphicsEngine = graphicsEngine;
    GEaddDevice2(graphicsEngine, "HTML Canvas Output");
    GEinitDisplayList(graphicsEngine);

    return R_NilValue;
}

/*==============================================================================
                            Core Graphics Routines
             Implementaion of an R Graphics Device as Defined by:
                               GraphicsDevice.h
==============================================================================*/

/*
 * Routines for handling device state:
 *

 * - Close
 * - Newpage
 * - Clip
 * - Size
 */

static void canvasClose(pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    /* Save plot */
    fclose(canvasInfo->outputFile);
    free(canvasInfo);
    dev->deviceSpecific = NULL;
    if( canvasInfo->debug == TRUE )
       Rprintf("Close(dev=0x%x)\n",dev);
}

static void canvasNewPage(const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;
    fprintf(canvasInfo->outputFile,"//NewPage\n");

    /* Set background only if we have a color */
    if (!R_TRANSPARENT(gc->fill)){
        canvasColor(canvasInfo->outputFile,"fillStyle",gc->fill);
        fprintf(canvasInfo->outputFile,"ctx.fillRect(0,0,%f,%f);\n",dev->right,dev->bottom);
    }

    if( canvasInfo->debug == TRUE )
       Rprintf("NewPage(gc=0x%x,dev=0x%x)\n",gc,dev);
}

static void canvasClip(double x0, double x1, double y0, double y1, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if( canvasInfo->debug == TRUE )
       Rprintf("Clip(");

    /* Too complicated to implement at the moment. The jist is that the context 
     * save()/restore() functions save not only the clip region but the current
     * transformation matrix and fill and stroke styles.
     */
/*    if (x1<x0) { double h=x1; x1=x0; x0=h; };
    if (y1<y0) { double h=y1; y1=y0; y0=h; };

    if (dev->left == x0 && dev->right == x1 && dev->top == y0 && dev->bottom == y1){
        fprintf(canvasInfo->outputFile,"ctx.restore();\n");
    } else {
        fprintf(canvasInfo->outputFile,"ctx.rect(%f,%f,%f,%f); ",x0,y0,x1-x0,y1-y0);
        fprintf(canvasInfo->outputFile,"ctx.clip();\n");
    }*/
    if( canvasInfo->debug == TRUE )
       Rprintf("x0=%f,y0=%f,x1=%f,y1=%f)\n",x0,y0,x1,y1);
}

static void canvasSize(double *left, double *right, double *bottom, double *top, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    *left = *top = 0.0;
    *right = dev->right;
    *bottom = dev->bottom;

    if( canvasInfo->debug == TRUE )
        Rprintf("Size(left=%f,right=%f,bottom=%f,top=%f,dev=0x%x)\n",*left,*right,*bottom,*top,dev);

}

/*
 * Routines for calculating text metrics:
 *
 * - MetricInfo
 * - StrWidth
 */

static void canvasMetricInfo(int c, const pGEcontext gc, double* ascent, double* descent, double* width, pDevDesc dev)
{

    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if( canvasInfo->debug == TRUE ){
        Rprintf("MetricInfo(");
    }

    const int char_code = { c };

    pFontInfo font = canvasGetFontDesc(canvasInfo, gc);

    int fontSize = gc->ps * gc->cex;

    StringMetrics m = calcFreetypeMetrics(font, &char_code, 1, fontSize);

    *ascent = (double)m.ascent;
    *descent = (double)m.descent;
    *width = (double)m.width;

    if( canvasInfo->debug == TRUE ){
        Rprintf("c=%d, gc=0x%x, ascent=%f, descent=%f, width=%f)\n",c,gc,*ascent,*descent,*width);
        Rprintf("** family= %s, c= %d, fontSize= %d, \n", gc->fontfamily, c, fontSize);
    }
}

static double canvasStrWidth(const char *str, const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if( canvasInfo->debug == TRUE ){
        Rprintf("StrWidth(");
    }

    int fontSize = gc->ps * gc->cex;

    int maxLen = strlen(str);
    wchar_t *unicodes = (wchar_t *) calloc(maxLen + 1, sizeof(wchar_t));
    int len = utf8towcs(unicodes, str, maxLen);

    pFontInfo font = canvasGetFontDesc(canvasInfo, gc);

    StringMetrics m = calcFreetypeMetrics(font, unicodes, len, fontSize);
    /* 10px sans-serif is default, however 7px provides a better guess. */
    if( canvasInfo->debug == TRUE ){
        Rprintf("str=%s, gc=0x%x, dev=0x%x)\n",str,gc,dev);
        Rprintf("** family = %s, str[0] = %c:%d, width = %d\n", 
            gc->fontfamily, str[0], str[0], m.width);
    }
    
    return m.width;

}

 /*
 * Output routines
 *
 * - Text
 * - Circle
 * - Rectangle
 * - Line
 * - Polyline
 * - Polygon
 * - Path
 * - Raster
 */

static void canvasText(double x, double y, const char *str, 
    double rot, double hadj, const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if (hadj!=0. || rot != 0.){
        double strextent = strlen(str) * 7; /* wild guess that each char is 10px wide */
        if (rot!=0.){
            fprintf(canvasInfo->outputFile,"ctx.save(); ");
            canvasColor(canvasInfo->outputFile,"fillStyle",gc->col);
            fprintf(canvasInfo->outputFile,"ctx.translate(%f,%f); ",x,y);
            fprintf(canvasInfo->outputFile,"ctx.rotate(-%f / 180 * Math.PI); ",rot);
            fprintf(canvasInfo->outputFile,"ctx.fillText(\"%s\",%f,0); ",str,-strextent*hadj);
            fprintf(canvasInfo->outputFile,"ctx.restore();\n");
        } else {
            canvasColor(canvasInfo->outputFile,"fillStyle",gc->col);
            fprintf(canvasInfo->outputFile,"ctx.fillText(\"%s\",%f,%f); ",str,x - strextent*hadj,y);
        }
    } else {
        canvasColor(canvasInfo->outputFile,"fillStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.fillText(\"%s\",%f,%f); ",str,x,y);
    }

    if( canvasInfo->debug == TRUE )
        fprintf(canvasInfo->outputFile,"//Text(x=%f,y=%f,str=%s,rot=%f,hadj=%f)\n",x,y,str,rot,hadj);
}

static void canvasCircle(double x, double y, double r, const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    fprintf(canvasInfo->outputFile,"ctx.beginPath();");
    fprintf(canvasInfo->outputFile,"ctx.arc(%f,%f,%f,0,Math.PI*2,true); ", x, y, r);
    if (!R_TRANSPARENT(gc->fill)){
        canvasColor(canvasInfo->outputFile,"fillStyle",gc->fill);
        fprintf(canvasInfo->outputFile,"ctx.fill(); ");
    }
    if (!R_TRANSPARENT(gc->col) && gc->lty!=-1){
        canvasSetLineType(canvasInfo,gc);
        canvasColor(canvasInfo->outputFile,"strokeStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.stroke();");
    }
    fprintf(canvasInfo->outputFile,"\n");

    if( canvasInfo->debug == TRUE )
       fprintf(canvasInfo->outputFile,"//Circle(x=%f,y=%f,r=%f)\n",x,y,r);
    /*Rprintf("\tuser coords: x=%f,y=%f\n",fromDeviceX(x,GE_NDC,MGD->RGE),fromDeviceY(y,GE_NDC,MGD->RGE));*/
}

static void canvasRect(double x0, double y0, double x1, double y1, const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;
    if (!R_TRANSPARENT(gc->fill)){
        canvasColor(canvasInfo->outputFile,"fillStyle",gc->fill);
        fprintf(canvasInfo->outputFile,"ctx.fillRect(%f,%f,%f,%f); ",x0,y0,x1-x0,y1-y0);
    }
    if (!R_TRANSPARENT(gc->col) && gc->lty!=-1){
        canvasSetLineType(canvasInfo,gc);
        canvasColor(canvasInfo->outputFile,"strokeStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.strokeRect(%f,%f,%f,%f); ",x0,y0,x1-x0,y1-y0);
    }
    if( canvasInfo->debug == TRUE )
        fprintf(canvasInfo->outputFile,"//Rect(x0=%f,y0=%f,x1=%f,y1=%f)\n",x0,y0,x1,y1);

}

static void canvasLine(double x1, double y1, double x2, double y2, const pGEcontext gc, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    Rprintf("%d, %d, %d, %d, %d, %d, %d\n", R_TRANSPARENT(gc->col), R_OPAQUE(gc->col), R_ALPHA(gc->col), 
        R_RED(gc->col), R_BLUE(gc->col), R_GREEN(gc->col), gc->lty);

    if (!R_TRANSPARENT(gc->col) && gc->lty!=-1){
        Rprintf(">>>>>>>>>>>>>>>>> Here <<<<<<<<<<<<<<<\n");
        canvasSetLineType(canvasInfo,gc);
        canvasColor(canvasInfo->outputFile,"strokeStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.beginPath(); ctx.moveTo(%f,%f); ctx.lineTo(%f,%f); ctx.stroke();\n",x1,y1,x2,y2);
    }

    if( canvasInfo->debug == TRUE )
       fprintf(canvasInfo->outputFile,"//Line(x0=%f,y0=%f,x1=%f,y1=%f)\n",x1,y1,x2,y2);
    
}

static void canvasPolyline(int n, double *x, double *y, const pGEcontext gc, pDevDesc dev)
{
    int i=1;
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if (n<2) return;

    if (!R_TRANSPARENT(gc->col) && gc->lty!=-1) {
        fprintf(canvasInfo->outputFile,"ctx.beginPath(); ctx.moveTo(%f,%f);\n",x[0],y[0]);
        while(i<n) { 
            fprintf(canvasInfo->outputFile,"ctx.lineTo(%f,%f);\n",x[i],y[i]); 
            i++; 
        }
        canvasSetLineType(canvasInfo,gc);
        canvasColor(canvasInfo->outputFile,"strokeStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.stroke();\n");
    }
    
    if( canvasInfo->debug == TRUE )
    { 
        int i=0;
        fprintf(canvasInfo->outputFile,"//Polyline(n=%d)\n\t//points: ",n);
        while(i<n){ 
            fprintf(canvasInfo->outputFile,"(%.2f,%.2f) ",x[i],y[i]); 
            i++;
        };
        fprintf(canvasInfo->outputFile,"\n");
    }
}


static void canvasPolygon(int n, double *x, double *y, const pGEcontext gc, pDevDesc dev)
{
    int i=1;
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if(n<2) return;

    canvasSetLineType(canvasInfo,gc);

    fprintf(canvasInfo->outputFile,"ctx.beginPath(); ctx.moveTo(%f,%f);\n",x[0],y[0]);
    while (i<n) { fprintf(canvasInfo->outputFile,"ctx.lineTo(%f,%f);", x[i], y[i]); i++; }
    fprintf(canvasInfo->outputFile,"ctx.closePath(); ");
    if (!R_TRANSPARENT(gc->fill)) {
        canvasColor(canvasInfo->outputFile,"fillStyle",gc->fill);
        fprintf(canvasInfo->outputFile,"ctx.fill(); ");
    }
    if (!R_TRANSPARENT(gc->col) && gc->lty!=-1) {
        canvasColor(canvasInfo->outputFile,"strokeStyle",gc->col);
        fprintf(canvasInfo->outputFile,"ctx.stroke(); ");
    }
    fprintf(canvasInfo->outputFile,"\n");

    if( canvasInfo->debug == TRUE ){ 
        int i=0;
        fprintf(canvasInfo->outputFile,"//Polygon(n=%d)\n\t//points: ",n);
        while(i<n){ 
            fprintf(canvasInfo->outputFile,"(%.2f,%.2f) ",x[i],y[i]); i++;
        }; 
        fprintf(canvasInfo->outputFile,"\n");
    }

}

/*==============================================================================

                           Stub Functions (unimplemented or unused)

==============================================================================*/

static void canvasMode(int mode, pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;
    if( canvasInfo->debug == TRUE )
       Rprintf("Mode(mode=%d,dev=0x%x)\n",mode,dev);
}

static void canvasActivate(const pDevDesc dev)
{ 
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if( canvasInfo->debug == TRUE )
        fprintf(canvasInfo->outputFile,"//Activate()\n");
}

static void canvasDeactivate(pDevDesc dev)
{
    canvasDesc *canvasInfo = (canvasDesc *)dev->deviceSpecific;

    if( canvasInfo->debug == TRUE )
       Rprintf("Deactivate(dev=0x%x)\n",dev);
}

/*==============================================================================

                           Style Definition Routines

==============================================================================*/

static void canvasSetLineType( canvasDesc *canvasInfo, pGEcontext gc)
{
    /* Line width */
    if (canvasInfo->lwd != gc->lwd){
        canvasInfo->lwd = gc->lwd;
        fprintf(canvasInfo->outputFile,"ctx.lineWidth = %f; ",canvasInfo->lwd);
    }

    /* Line end: par lend  */
    if (canvasInfo->lend != gc->lend){
        canvasInfo->lend = gc->lend;
        if (canvasInfo->lend == GE_ROUND_CAP)
            fprintf(canvasInfo->outputFile,"ctx.lineCap = \"round\"; ");
        if (canvasInfo->lend == GE_BUTT_CAP)
            fprintf(canvasInfo->outputFile,"ctx.lineCap = \"butt\"; ");
        if (canvasInfo->lend == GE_SQUARE_CAP)
            fprintf(canvasInfo->outputFile,"ctx.lineCap = \"square\"; ");
    }

    /* Line join: par ljoin */
    if (canvasInfo->ljoin != gc->ljoin){
        canvasInfo->ljoin = gc->ljoin;
        if (canvasInfo->ljoin == GE_ROUND_JOIN)
            fprintf(canvasInfo->outputFile,"ctx.lineJoin = \"round\"; ");
        if (canvasInfo->ljoin == GE_MITRE_JOIN)
            fprintf(canvasInfo->outputFile,"ctx.lineJoin = \"miter\"; ");
        if (canvasInfo->ljoin == GE_BEVEL_JOIN)
            fprintf(canvasInfo->outputFile,"ctx.lineJoin = \"bevel\"; ");
    }

    /* Miter limit */
    if (canvasInfo->lmitre != gc->lmitre){
        canvasInfo->lmitre = gc->lmitre;
        fprintf(canvasInfo->outputFile,"ctx.miterLimit = %f; ",canvasInfo->lmitre);
    }
    fprintf(canvasInfo->outputFile,"\n");
}

/*==============================================================================

                               Utility Routines

==============================================================================*/
/* Get the font description object defined in canvasFont.h */
static pFontInfo canvasGetFontDesc(pCanvasDesc canvasInfo, const pGEcontext gc)
{
    int gcfontface = gc->fontface;
    pFontInfo font;
    
    SEXP fontList;
    SEXP fontNames;
    SEXP extPtr;
    int i, listLen;
    
    /* Font list is .canvasInfo$.fontList, defined in font.R */
    PROTECT(fontList = Rf_findVar(install(".fontList"), canvasInfo->pkgEnv));
    UNPROTECT(1);
    fontNames = GET_NAMES(fontList);
    listLen = Rf_length(fontList);
    // See if the fontface loaded in the font list in R
    for(i = 0; i < listLen; i++)
    {
        if(strcmp(gc->fontfamily, CHAR(STRING_ELT(fontNames, i))) == 0)
        {
            break;
        }
    }
    // If the font is not loaded, default to the package fonts
    if(i == listLen) i = 0;
    if(gcfontface < 1 || gcfontface > 5) gcfontface = 1;
    
    // return a pointer to the loaded font object
    extPtr = VECTOR_ELT(VECTOR_ELT(fontList, i), gcfontface - 1);
    font = (FontInfo *) R_ExternalPtrAddr(extPtr);
    
    return font;
}


/* The following two functions are copied from R/src/main/util.c */
static size_t utf8toucs(wchar_t *wc, const char *s)
{
    unsigned int byte;
    wchar_t local, *w;
    byte = *((unsigned char *)s);
    w = wc ? wc: &local;

    if (byte == 0)
    {
        *w = (wchar_t) 0;
        return 0;
    }
    else if (byte < 0xC0)
    {
        *w = (wchar_t) byte;
        return 1;
    }
    else if (byte < 0xE0)
    {
        if(strlen(s) < 2) return (size_t)-2;
        if ((s[1] & 0xC0) == 0x80)
        {
            *w = (wchar_t) (((byte & 0x1F) << 6) | (s[1] & 0x3F));
            return 2;
        }
        else return (size_t)-1;
    }
    else if (byte < 0xF0)
    {
        if(strlen(s) < 3) return (size_t)-2;
        if (((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80))
        {
            *w = (wchar_t) (((byte & 0x0F) << 12)
                            | (unsigned int) ((s[1] & 0x3F) << 6)
                            | (s[2] & 0x3F));
            byte = (unsigned int) *w;
            /* Surrogates range */
            if(byte >= 0xD800 && byte <= 0xDFFF) return (size_t)-1;
            if(byte == 0xFFFE || byte == 0xFFFF) return (size_t)-1;
            return 3;
        }
        else return (size_t)-1;
    }
    if(sizeof(wchar_t) < 4) return (size_t)-2;
    /* So now handle 4,5.6 byte sequences with no testing */
    if (byte < 0xf8)
    {
        if(strlen(s) < 4) return (size_t)-2;
        *w = (wchar_t) (((byte & 0x0F) << 18)
                        | (unsigned int) ((s[1] & 0x3F) << 12)
                        | (unsigned int) ((s[2] & 0x3F) << 6)
                        | (s[3] & 0x3F));
        return 4;
    }
    else if (byte < 0xFC)
    {
        if(strlen(s) < 5) return (size_t)-2;
        *w = (wchar_t) (((byte & 0x0F) << 24)
                        | (unsigned int) ((s[1] & 0x3F) << 12)
                        | (unsigned int) ((s[2] & 0x3F) << 12)
                        | (unsigned int) ((s[3] & 0x3F) << 6)
                        | (s[4] & 0x3F));
        return 5;
    }
    else
    {
        if(strlen(s) < 6) return (size_t)-2;
        *w = (wchar_t) (((byte & 0x0F) << 30)
                        | (unsigned int) ((s[1] & 0x3F) << 24)
                        | (unsigned int) ((s[2] & 0x3F) << 18)
                        | (unsigned int) ((s[3] & 0x3F) << 12)
                        | (unsigned int) ((s[4] & 0x3F) << 6)
                        | (s[5] & 0x3F));
        return 6;
    }
}

static int utf8towcs(wchar_t *wc, const char *s, int n)
{
    ssize_t m, res = 0;
    const char *t;
    wchar_t *p;
    wchar_t local;

    if(wc)
        for(p = wc, t = s; ; p++, t += m)
        {
            m  = (ssize_t) utf8toucs(p, t);
            if (m < 0) Rf_error("invalid input '%s' in 'utf8towcs'", s);
            if (m == 0) break;
            res ++;
            if (res >= n) break;
        }
    else
        for(t = s; ; res++, t += m)
        {
            m  = (ssize_t) utf8toucs(&local, t);
            if (m < 0) Rf_error("invalid input '%s' in 'utf8towcs'", s);
            if (m == 0) break;
        }
    return (int) res;
}