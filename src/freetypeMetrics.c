#include <ft2build.h>
#include FT_FREETYPE_H 
#include FT_GLYPH_H  

#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freetypeMetrics.h"

/* Function called by R calculate metrics */
SEXP freetypeMetrics(SEXP filename_r, SEXP char_codes_r, SEXP num_chars_r, SEXP point_size_r)
{

    /* Retrieve information from arguments */
    const char *filename = CHAR(STRING_ELT(filename_r, 0));
    const int *char_codes = INTEGER(char_codes_r);
    const int num_chars = INTEGER(num_chars_r)[0];
    const int point_size = INTEGER(point_size_r)[0]; 

    SEXP metrics_out, metrics_names;

    // Call the workhorse function
    StringMetrics m = getFreetypeMetrics(filename, char_codes, num_chars, point_size);
    
    /* 
        This tedious process is just to set up a 
        named list with four atomic elements.
    */
    PROTECT(metrics_out = allocVector(VECSXP, 4));
    PROTECT(metrics_names = allocVector(STRSXP, 4));
    
    SET_STRING_ELT(metrics_names, 0, mkChar("height"));
    SET_VECTOR_ELT(metrics_out, 0, allocVector(INTSXP, 1));
    INTEGER(VECTOR_ELT(metrics_out, 0))[0] = m.height;

    SET_STRING_ELT(metrics_names, 1, mkChar("ascent"));
    SET_VECTOR_ELT(metrics_out, 1, allocVector(INTSXP, 1));
    INTEGER(VECTOR_ELT(metrics_out, 1))[0] = m.ascent;

    SET_STRING_ELT(metrics_names, 2, mkChar("descent"));
    SET_VECTOR_ELT(metrics_out, 2, allocVector(INTSXP, 1));
    INTEGER(VECTOR_ELT(metrics_out, 2))[0] = m.descent;

    SET_STRING_ELT(metrics_names, 3, mkChar("width"));
    SET_VECTOR_ELT(metrics_out, 3, allocVector(INTSXP, 1));
    INTEGER(VECTOR_ELT(metrics_out, 3))[0] = m.width;

    setAttrib(metrics_out, R_NamesSymbol, metrics_names);

    UNPROTECT(2);
    return metrics_out;

    return R_NilValue;
}

StringMetrics getFreetypeMetrics(const char *filename, 
    const int *char_codes, const int num_chars, const int point_size){

    FT_Library    library;
    FT_Face       face;

    FT_Error      e;

    FT_Glyph_Metrics metrics;
    FT_BBox  bbox;

    FT_Glyph  glyph; /* a handle to the glyph image */

    int           n;
    int           width, height, ascent, descent;
    StringMetrics m;

    //Rprintf("%d\n",num_chars);

    e = FT_Init_FreeType(&library);              /* initialize library */
    if (e) { error ("Could not initilize FreeType library.\n"); }

    /*
        when a new face object is created, it will look for a Unicode charmap 
        and select it. The currently selected charmap is accessed via 
        face->charmap. This field is NULL when no charmap is selected, which 
        typically happens when you create a new FT_Face object from a font 
        file that doesn't contain a Unicode charmap (which is rather infrequent 
        today).

        For the time being we will just load the unicode character map and 
        assume thats what the user wants. 
    */
    e = FT_New_Face(library, filename, 0, &face);/* create face object */
    if (e == FT_Err_Unknown_File_Format)
    {
        /* 
            the font file could be opened and read, but it appears
            that its font format is unsupported 
        */
        error ("The font file could be opened and read, \
            but it appears that its font format is unsupported.\n");
    }
    else if (e)
    {
        /* 
            another error code means that the font file could not
            be opened or read, or simply that it is broken...
        */
        error ("The font file could not be opened or read, it may be corrupt.\n");
    }

    /* Set up character size, leave resolution blank for now  */
    e = FT_Set_Char_Size(
                    face,    /* handle to face object           */
                    0,       /* char_width in 1/64th of points  */
                    point_size * 64,   /* char_height in 1/64th of points */
                    0,     /* horizontal device resolution    */
                    0);   /* vertical device resolution      */
    if (e) { error ("Error setting FreeType character size.\n"); }

    /* 
        Normally, a point is not equivalent to a pixel but we are targetting a
        region on a webpage which is based on pixel width and height, so set
        the pixel width of the face to be the same as the point size, 
        effectively forcing freetype to output in units of 1/64th of a pixel.
    */
    e = FT_Set_Pixel_Sizes(
            face,           /* handle to face object */
            point_size,     /* pixel_width           */
            point_size);   /* pixel_height          */
    if (e) { error ("Error setting pixel size.\n"); } 
    
    // Initilize   
    width = height = descent = ascent = 0;   

    for (n = 0; n < num_chars; n++)
    {

        /* FT_LOAD_NO_SCALE, FT_LOAD_DEFAULT */
        e = FT_Load_Char(face, char_codes[n], FT_LOAD_DEFAULT);
        if (e){ error ("Error loading glyph.\n"); }
        ///* retrieve kerning distance and move pen position */
        //    if (use_kerning && previous && glyph_index)
        //    {
        //      FT_Vector  delta;
        //      FT_Get_Kerning(face, previous, glyph_index,
        //                      FT_KERNING_DEFAULT, &delta);
        //      pen_x += delta.x >> 6;
        //    }

        e = FT_Get_Glyph(face->glyph, &glyph);
        if (e) { error ("Error getting glyph.\n"); } 

        /* 
            If the glyph has been loaded with FT_LOAD_NO_SCALE, 
            bbox_mode must be set to FT_GLYPH_BBOX_UNSCALED to get 
            unscaled font units in 26.6 pixel format.
            FT_GLYPH_BBOX_UNSCALED
            FT_GLYPH_BBOX_GRIDFIT
            FT_GLYPH_BBOX_PIXELS
        */
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);

        metrics = face->glyph->metrics;

        // Convert to pixels
        width += metrics.width / 64.0;
        /*
            If ‘yMin’ is negative, this value gives the glyph's descender. 
            Otherwise, the glyph doesn't descend below the baseline. Similarly, 
            if ‘ymax’ is positive, this value gives the glyph's ascender.

            Also recall we are calcualting metrics for an entire string, so 
            the descent of the string is the lowest descender of all the 
            characters and ascent is the highest ascender. 
        */
        if(bbox.yMin < 0) {
            descent =  MIN(bbox.yMin, descent);
        } else {
            descent = MIN(0, descent);
        }
        if(bbox.yMax > 0) {
            ascent = MAX(bbox.yMax, ascent);
        } else {
            ascent = MAX(0, ascent);
        }
        
        //Rprintf("     Height: %d\n     Ascent: %d\n     Descent: %d\n     Width: %d\n", 
        //    height, ascent, descent, width);  
    }           

    // By definition
    height = ascent + abs(descent);

    FT_Done_Face    (face);
    FT_Done_FreeType(library);

    m.height = height;
    m.width = width;
    m.ascent = ascent;
    m.descent = descent;
    return m;
}

 R_CallMethodDef callMethods[]  = {
   {"freetypeMetrics", (DL_FUNC) &freetypeMetrics, 2},
   {NULL, NULL, 0}
 };