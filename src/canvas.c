#include <stdio.h>

#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/GraphicsEngine.h>
#include <R_ext/GraphicsDevice.h>

#if R_VERSION >= R_Version(2,8,0)
#ifndef NewDevDesc
#define NewDevDesc DevDesc
#endif
#endif

typedef struct _canvasDesc {
	/* device specific stuff */
	int col;
	short lwd;
	int lty;
	float miterLimit;
	FILE *fp;
	pGEDevDesc RGE;
} canvasDesc;

static void canvasSetLineType( canvasDesc *cGD, pGEcontext gc)
{
	/* Line end: par lend  */
	cGD->lty = 0;
	cGD->miterLimit = 2.0;
	switch(gc->lend){
		case GE_ROUND_CAP: cGD->lty =  0; break;
		case GE_BUTT_CAP: cGD->lty =   0; break;
		case GE_SQUARE_CAP: cGD->lty = 0; break;
	}

	/* Line join: par ljoin */
	switch(gc->ljoin){
		case GE_ROUND_JOIN: cGD->lty |= 0; break;
		case GE_MITRE_JOIN: cGD->lty |= 0; break;
		case GE_BEVEL_JOIN: cGD->lty |= 0; break;
	} 
}

static void canvasActivate(const pDevDesc RGD)
{ 
#ifdef CANVASDEBUG
	Rprintf("Activate(RGD=0x%x)\n",RGD);
#endif
}

static void canvasCircle(double x, double y, double r, const pGEcontext gc, pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;

	
	fprintf(cGD->fp,"ctx.beginPath(); ctx.arc(%f,%f,%f,0,Math.PI*2,true); ctx.stroke();\n", x, y, r);

#ifdef CANVASDEBUG
	Rprintf("Circle(x=%f,y=%f,r=%f,gc=0x%x,RGD=0x%x)\n",x,y,r,gc,RGD);
	/*Rprintf("\tuser coords: x=%f,y=%f\n",fromDeviceX(x,GE_NDC,MGD->RGE),fromDeviceY(y,GE_NDC,MGD->RGE));*/
#endif
}

static void canvasClip(double x0, double x1, double y0, double y1, pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;
/*	fprintf(cGD->fp,"ctx.beginPath(); ");
	fprintf(cGD->fp,"ctx.rect(%f,%f,%f,%f); ",x0,y0,x1-x0,y1-y0);
	fprintf(cGD->fp,"ctx.clip();\n");*/
#ifdef CANVASDEBUG
	Rprintf("Clip(x0=%f,y0=%f,x1=%f,y1=%f,RGD=0x%x)\n",x0,y0,x1,y1,RGD);
#endif
}

static void canvasClose(pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;

	/* Save plot */
	fclose(cGD->fp);
	free(cGD);
	RGD->deviceSpecific = NULL;
#ifdef CANVASDEBUG
	Rprintf("Close(RGD=0x%x)\n",RGD);
#endif
}

static void canvasDeactivate(pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Deactivate(RGD=0x%x)\n",RGD);
#endif
}
static Rboolean canvasLocator(double *x, double *y, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Locator(x=%f,y=%f,RGD=0x%x)\n",x,y,RGD);
#endif
}

static void canvasLine(double x1, double y1, double x2, double y2, const pGEcontext gc, pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;

	fprintf(cGD->fp,"ctx.beginPath(); ctx.moveTo(%f,%f); ctx.lineTo(%f,%f); ctx.stroke();\n",x1,y1,x2,y2);

#ifdef CANVASDEBUG
	Rprintf("Line(x0=%f,y0=%f,x1=%f,y1=%f,gc=0x%x,RGD=0x%x)\n",x1,y1,x2,y2,gc,RGD);
#endif
}

static void canvasMetricInfo(int c, const pGEcontext gc, double* ascent, double* descent, double* width, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("MetricInfo(c=%d,gc=0x%x,ascent=%f,descent=%f,width=%f,RGD=0x%x)\n",c,gc,ascent,descent,width,RGD);
#endif
}
static void canvasMode(int mode, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Mode(mode=%d,RGD=0x%x)\n",mode,RGD);
#endif
}

static void canvasNewPage(const pGEcontext gc, pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;
	fprintf(cGD->fp,"/* NewPage */\n");
#ifdef CANVASDEBUG
	Rprintf("NewPage(gc=0x%x,RGD=0x%x)\n",gc,RGD);
#endif
}

static void canvasPolygon(int n, double *x, double *y, const pGEcontext gc, pDevDesc RGD)
{
	int i;
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;
#ifdef CANVASDEBUG
	{ int i=0;
	Rprintf("Polygon(n=%d,x=0x%x,y=0x%x,gc=0x%x,RGD=0x%x)\n\tpoints: ",n,x,y,gc,RGD);
	while(i<n){ Rprintf("(%.2f,%.2f) ",x[i],y[i]); i++;} Rprintf("\n");
	}
#endif
}

static void canvasPolyline(int n, double *x, double *y, const pGEcontext gc, pDevDesc RGD)
{
	canvasDesc *cGD = (canvasDesc *)RGD->deviceSpecific;
	int i=1;
	fprintf(cGD->fp,"ctx.beginPath(); ctx.moveTo(%f,%f);\n",x[0],y[0]);
	while(i<n) {
		fprintf(cGD->fp,"ctx.lineTo(%f,%f);\n",x[i],y[i]);
		i++;
	}
	fprintf(cGD->fp,"ctx.stroke();\n");
#ifdef CANVASDEBUG
	{ int i=0;
	Rprintf("Polyline(n=%d,x=0x%x,y=0x%x,gc=0x%x,RGD=0x%x)\n\tpoints: ",n,x,y,gc,RGD);
	while(i<n){ Rprintf("(%.2f,%.2f) ",x[i],y[i]); i++;} Rprintf("\n");
	}
#endif
}

static void canvasRect(double x0, double y0, double x1, double y1, const pGEcontext gc, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Rect(x0=%f,y0=%f,x1=%f,y1=%f,gc=0x%x,RGD=0x%x)\n",x0,y0,x1,y1,gc,RGD);
#endif

}
static void canvasSize(double *left, double *right, double *bottom, double *top, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Size(left=%f,right=%f,bottom=%f,top=%f,RGD=0x%x)\n",*left,*right,*bottom,*top,RGD);
#endif
}

static double canvasStrWidth(const char *str, const pGEcontext gc, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("StrWidth(str=%s,gc=0x%x,RGD=0x%x)\n",str,gc,RGD);
#endif
}

static void canvasText(double x, double y, const char *str, double rot, double hadj, const pGEcontext gc, pDevDesc RGD)
{
#ifdef CANVASDEBUG
	Rprintf("Text(x=%f,y=%f,str=%s,rot=%f,hadj=%f,gc=0x%x,RGD=0x%x)\n",x,y,str,rot,hadj,gc,RGD);
#endif
}

SEXP canvas_new_device(SEXP args)
{
	/* R Graphics Device: in GraphicsDevice.h */
	pDevDesc RGD;

	/* R Graphics Engine: in GraphicsEngine.h */
	pGEDevDesc RGE;

	/* canvas Graphics Device */
	canvasDesc *cGD;

	FILE *fp = NULL;
	int width, height;

	SEXP v;
	args=CDR(args);
	v=CAR(args); args=CDR(args);
	if (isString(v)){
		PROTECT(v);
		fp = fopen(CHAR(STRING_ELT(v,0)),"w");
		UNPROTECT(1);
		if (fp == NULL)
			error("could not open file");
	} else {
		error("file must be a filename");
	}

	v=CAR(args); args=CDR(args);
	if (!isNumeric(v)) {fclose(fp); error("`width' must be a number");}
	width=asInteger(v);
	v=CAR(args); args=CDR(args);
	if (!isNumeric(v)) {fclose(fp); error("`height' must be a number");}
	height=asInteger(v);
#ifdef CANVASDEBUG
	Rprintf("canvas_new_device(width=%d,height=%d,fd=%x)\n", width, height, fp);
#endif
	
    R_CheckDeviceAvailable();

	if (!(RGD = (pDevDesc)calloc(1, sizeof(NewDevDesc)))){
		fclose(fp);
	    error("calloc failed for canvas device");
	}

    if (!(cGD = (canvasDesc *)calloc(1, sizeof(canvasDesc)))){
		free(RGD);
		fclose(fp);
	    error("calloc failed for canvas device");
	}

	cGD->fp = fp;

    RGD->deviceSpecific = (void *) cGD;

	/* Callbacks */
    RGD->close = canvasClose;
    RGD->activate = canvasActivate;
    RGD->deactivate = canvasDeactivate;
    RGD->size = canvasSize;
    RGD->newPage = canvasNewPage;
    RGD->clip = canvasClip;
    RGD->strWidth = canvasStrWidth;
    RGD->text = canvasText;
    RGD->rect = canvasRect;
    RGD->circle = canvasCircle;
    RGD->line = canvasLine;
    RGD->polyline = canvasPolyline;
    RGD->polygon = canvasPolygon;
    RGD->locator = canvasLocator;
    RGD->mode = canvasMode;
    RGD->metricInfo = canvasMetricInfo;
	RGD->hasTextUTF8 = TRUE;
    RGD->strWidthUTF8 = canvasStrWidth;
    RGD->textUTF8 = canvasText;
	RGD->wantSymbolUTF8 = TRUE;

	/* Initialise RGD */
	RGD->left = RGD->clipLeft = 0;
	RGD->top = RGD->clipTop = 0;
	RGD->right = RGD->clipRight = width;
	RGD->bottom = RGD->clipBottom = height;
	RGD->xCharOffset = 0.4900;
	RGD->yCharOffset = 0.3333;
	RGD->yLineBias = 0.1;
	RGD->ipr[0] = 1.0/72.0;
	RGD->ipr[1] = 1.0/72.0;
	RGD->cra[0] = 0.9 * 12;
	RGD->cra[1] = 1.2 * 12;
	RGD->gamma = 1.0;
	RGD->canClip = TRUE;
    RGD->canChangeGamma = FALSE;
    RGD->canHAdj = 2;
	RGD->startps = 12.0;
	RGD->startcol = R_RGB(0,0,0);
	RGD->startfill = 0xffffffff;
	RGD->startlty = LTY_SOLID;
	RGD->startfont = 1;
	RGD->startgamma = RGD->gamma;
    RGD->displayListOn = TRUE;

	/* Add to the device list */
	RGE = GEcreateDevDesc(RGD);
	cGD->RGE = RGE;
	GEaddDevice(RGE);
	GEinitDisplayList(RGE);

	/*return ScalarInteger(1 + GEdeviceNumber(RGE));*/
    return R_NilValue;
}

R_ExternalMethodDef canvas_externals[] = {
	{"canvas_new_device",(DL_FUNC) &canvas_new_device,3},
	{NULL,NULL,0}
};

void R_init_canvas(DllInfo *info){
	R_registerRoutines(info, NULL, NULL, NULL, canvas_externals);
}
