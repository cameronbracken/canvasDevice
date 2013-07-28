#include "canvasFont.h"

// TODO wrapper to use a preloaded font by passing the pointer from R

/* Function called by R calculate metrics */
SEXP freetypeMetrics(SEXP fontObj_r, SEXP char_codes_r, SEXP num_chars_r, SEXP point_size_r)
{

    /* Retrieve information from arguments */
    //const char *filename = CHAR(STRING_ELT(filename_r, 0));
    const int *char_codes = INTEGER(char_codes_r);
    const int num_chars = INTEGER(num_chars_r)[0];
    const int point_size = INTEGER(point_size_r)[0]; 

    SEXP metrics_out, metrics_names;

    // Call the workhorse function
    StringMetrics m = getFreetypeMetrics(fontObj_r, char_codes, num_chars, point_size);
    
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
}

/* 
    Convenience wrapper (for calling from R) that loads a 
    font file and calculates metrics for a string
*/
StringMetrics getFreetypeMetrics(SEXP fontObj, const int *char_codes, 
    const int num_chars, const int point_size){

    SEXP fontExternalPointer;
    pFontInfo font;

    switch(TYPEOF(fontObj)) {
        case STRSXP:
            // We were given an unloaded font file name
            // So load it first, get a pointer to the loaded font obj
            // and then send that to the metrics calculator
            fontExternalPointer = canvasLoadFont(fontObj);
            font = (pFontInfo) R_ExternalPtrAddr(fontExternalPointer);
            //Rprintf("Passed string\n");
            break;
        case EXTPTRSXP:
            //Rprintf("Passed pointer\n");
            // If we are calling from C, assume the font is loaded
            // Get the font obj and pass it to metrics calculator
            font = (pFontInfo) R_ExternalPtrAddr(fontObj);
            break;
        default:
            Rprintf("Unknown R type, bailing out.\n");
            error("Invalid font file reference.");
    }

    return calcFreetypeMetrics(font, char_codes, num_chars, point_size);

}

/* 
    Workhorse function 
*/
StringMetrics calcFreetypeMetrics(pFontInfo font, const int *char_codes, 
    const int num_chars, const int point_size){

    FT_Error      e;

    //FT_Glyph_Metrics metrics;
    FT_BBox  bbox;

    FT_Glyph  glyph; /* a handle to the glyph image */

    int           n;
    int           width, height, ascent, descent;
    StringMetrics m;
    

    /* Set up character size, leave resolution blank for now  */
    e = FT_Set_Char_Size(
                    font->face,    /* handle to face object           */
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
            font->face,           /* handle to face object */
            point_size,     /* pixel_width           */
            point_size);   /* pixel_height          */
    if (e) { error ("Error setting pixel size.\n"); } 
    
    // Initilize   
    width = height = descent = ascent = 0;   

    for (n = 0; n < num_chars; n++)
    {

        /* FT_LOAD_NO_SCALE, FT_LOAD_DEFAULT */
        e = FT_Load_Char(font->face, char_codes[n], FT_LOAD_DEFAULT);
        if (e){ error ("Error loading glyph.\n"); }
        /* Possibley deal with kerning here later
            // retrieve kerning distance and move pen position 
            if (use_kerning && previous && glyph_index)
            {
              FT_Vector  delta;
              FT_Get_Kerning(face, previous, glyph_index,
                              FT_KERNING_DEFAULT, &delta);
              pen_x += delta.x >> 6;
            }
        */

        e = FT_Get_Glyph(font->face->glyph, &glyph);
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

        // Convert to pixels 
        // should we use width here? when considering the width 
        // of a string we should probably use the horizontal advance of a
        // character and not just the width of the glyph I think,
        // This will change when kerning is used. 
        width += font->face->glyph->metrics.horiAdvance / 64.0;
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

    m.height = height;
    m.width = width;
    m.ascent = ascent;
    m.descent = -descent;
    return m;
}

/* 
    This function loads a font using freetype and creates C object that 
    can be accessed from R as well as from other C functions. The reason 
    why we are going to the trouble is so that a font can be loaded from 
    an R function regardless of the device being open. One could 
    potentially switch the font while the device is open as well, perhaps
    placing a few text strings using different fonts. This would also 
    take some fiddling with the webpage code too to ensure fonts were 
    available. 
*/
SEXP canvasLoadFont(SEXP fontPath)
{   

    const char* file = CHAR(STRING_ELT(fontPath, 0));

    // initilize the font object holding the font face and the ft library info
    pFontInfo font = (pFontInfo) calloc(1, sizeof(FontInfo));

    FT_Error e;
        
    // Pointer to the font object that can be accessed outside of this function. 
    SEXP fontExternalPointer;

    e = FT_Init_FreeType(&(font->library));              /* initialize library */
    if (e) { 
        if(font) free(font);
        error ("Could not initilize FreeType library.\n"); 
    }

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
    e = FT_New_Face(font->library, file, 0, &(font->face));/* create face object */
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
        if(font->library) FT_Done_FreeType(font->library);
        if(font) free(font);
        switch(e)
        {
            case 0x01:
                Rf_error("freetype: cannot open resource, error code %d", e);
                break;
            case 0x02:
                Rf_error("freetype: unknown file format, error code %d", e);
                break;
            case 0x03:
                Rf_error("freetype: broken file, error code %d", e);
                break;
            default:
                Rf_error("freetype: unable to load font file, error code %d", e);
                break;

            /* //Freetype error codes 
                
                */
        }
    }

    /*
        Create a pointer to the loaded font object and register the routine that 
        will unload the font when there is no longer a corresponding R object
    */
    fontExternalPointer = R_MakeExternalPtr(font, R_NilValue, R_NilValue);
    PROTECT(fontExternalPointer);
    R_RegisterCFinalizerEx(fontExternalPointer, canvasUnloadFont, TRUE);
    UNPROTECT(1);

    return fontExternalPointer;
}

/* 
    Loaded font destructor. 

    This function is called by the garbage collector 
    when there is no longer an R object attached to the 
    C object that points to the loaded font.  

    No need to manually call this.   
*/
void canvasUnloadFont(SEXP fontExternalPointer)
{
    pFontInfo font = (pFontInfo) R_ExternalPtrAddr(fontExternalPointer);
    
    if(font->face) FT_Done_Face(font->face);
    if(font->library) FT_Done_FreeType(font->library);
    if(font) free(font);

    //Rprintf("Unloading font.\n");
    
}