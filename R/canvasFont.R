# Any variables defined in here will be hidden
# from normal users.
.canvasInternal <- new.env()
# list of font pointers separated into families 
.canvasInternal$.fontList = list();
# list of pointers to C font objects
.canvasInternal$.fontPointerList = list();
.canvasInternal$.fontSearchPaths = character(0);

# NOTE: functions starting with a '.' are internal functions

.setDefaultCanvasFontPaths = function()
{
    path = switch(Sys.info()[["sysname"]],
           Windows = normalizePath(file.path(Sys.getenv("windir"), "Fonts")),
           Linux = list.dirs(c("/usr/share/fonts",
                               "/usr/local/share/fonts",
                               "~/.fonts")),
           Darwin = list.dirs(c("/Library/Fonts",
                                "~/Library/Fonts")));
    .canvasInternal$.fontSearchPaths = path;
}

#' Get/Set font search paths
#' 
#' This function gets/sets the search paths for font files.
#' 
#' @param new a character vector indicating the search paths to be
#'        prepended. If the argument is missing, the function will
#'        return the current search paths.
#' @return The updated search paths
#' 
#' @details Default search paths will be assigned when package is loaded:
#' \itemize{
#' \item For Windows, it is \code{\%windir\%\\Fonts}
#'
#' \item For Mac OS, default paths are \code{/Library/Fonts}
#'       and \code{~/Library/Fonts} and their subdirectories
#'
#' \item For Linux, \code{/usr/share/fonts},
#'       \code{/usr/local/share/fonts}, \code{~/.fonts}, and
#'       their subdirectories
#' }
#' 
#' @seealso See \code{\link{registerFontFamily}()} for details about how
#'          \pkg{canvasDevice} looks for font files. There is also a
#'          complete example showing the usage of these functions
#'          in the help page of \code{\link{registerFontFamily}()}
#' 
#' @export
#' 
#' @author Yixuan Qiu <\url{http://yixuan.cos.name/}>
canvasFontPaths = function(new)
{
    if(!missing(new))
    {
        new = path.expand(new);
        paths = unique(normalizePath(c(new, .canvasInternal$.fontSearchPaths)));
        .canvasInternal$.fontSearchPaths = paths;
    }
    return(.canvasInternal$.fontSearchPaths);
}

#' List available font families for canvas device
#' 
#' This function lists font families currently available that can be
#' used by canvas device through \code{par("family")}.
#' 
#' @return A character vector of available font family names
#' 
#' @details By default there are three font families loaded automatically,
#' i.e., "sans", "serif" and "mono". If you want to use other font
#' families in canvas device, you need to call \code{\link{registerFontFamily}()}
#' to register new fonts by specifying a family name and corresponding
#' font file paths. See \code{\link{registerFontFamily}()} for details about
#' what's the meaning of "family name" in this context, as well as
#' a complete example of registering and using a new font.
#' 
#' @seealso \code{\link{registerFontFamily}()}
#' 
#' @export
#' 
#' @author Yixuan Qiu <\url{http://yixuan.cos.name/}>
#' 
#' @examples canvasFonts()
#' 
canvasFonts = function()
{
    return(names(.canvasInternal$.fontList));
}

.checkFontPath = function(path, type)
{
    if(file.exists(path))
    {
        if(file.info(path)$isdir) {
            stop(sprintf("file path for '%s' shouldn't be a directory", type));
        } else return(path);
    }
    
    filename = basename(path);
    search.paths = canvasFontPaths();
    found = FALSE;
    for(dir in search.paths)
    {
        path = file.path(dir, filename);
        if(file.exists(path) & !file.info(path)$isdir)
        {
            found = TRUE;
            break;
        }
    }
    if(!found) stop(sprintf("font file not found for '%s' type", type));
    
    return(normalizePath(path));
}

#' Add new font families for canvas device
#' 
#' This function registers new font families that can be used by canvas
#' device. Currently supported formats are ttf/ttc fonts.
#' 
#' @param family a character string of maximum 200-byte size,
#'               indicating the family name of the fonts you want to add.
#'               See "Details" for further explanation.
#' @param regular path of the font file for "regular" font face.
#'                This argument must be specified as a character string
#'                and cannot be missing.
#' @param bold path of the font file for "bold" font face.
#'             If it is \code{NULL}, the function will use the value of
#'             argument \code{regular}.
#' @param italic,bolditalic,symbol ditto
#' 
#' @return A character vector (invisible) of current available
#'         font family names
#' 
#' @details In R graphics device, there are two parameters combined together
#' to select a font to show texts. \code{par("family")} is a character
#' string giving a name to a \strong{series} of font faces. Here
#' \strong{series} implies that there may be different fonts with the
#' same family name, and actually they are distinguished by the parameter
#' \code{par("font")}, indicating whether it is regular, bold or italic,
#' etc. In R, \code{par("font")} is an integer from 1 to 5 representing
#' regular, bold, italic, bold italic and symbol respectively.
#' 
#' In canvas device, there are three default font families, sans, serif and mono,
#' along with those 5 font faces, that can be used immediately. If you want
#' to use other font families, you could call \code{registerFontFamily()} to register
#' new fonts. Notice that the \code{family} argument in this function can be
#' an arbitrary string which doesn't need to be the real font name. You will
#' use the specified family name in functions like \code{par(family = "myfont")}
#' and \code{text("Some text", family = "myfont")}. The "Examples" section
#' shows a complete demonstration of the usage.
#' 
#' To find the font file of argument \code{regular} (and the same for
#' other font faces), this function will first check the existence
#' of the specified path. If not found, file will be searched in the
#' directories returned by \code{\link{canvasFontPaths}()} in turn. If the
#' file cannot be found in any of the locations,
#' an error will be issued.
#' 
#' @seealso See \code{\link[graphics]{par}()} for explanation of
#'          the parameters \code{family} and \code{font}
#' 
#' @export
#' 
#' @author Yixuan Qiu <\url{http://yixuan.cos.name/}>
#' 
#' @examples \dontrun{
#' # Example: download the font file of WenQuanYi Micro Hei,
#' #          add it to canvas device, and use it to draw text in canvas()
#' # WenQuanYi Micro Hei is an open source and high quality Chinese (and CJKV) font
#' wd = setwd(tempdir());
#' ft.url = "http://sourceforge.net/projects/wqy/files/wqy-microhei/0.2.0-beta/wqy-microhei-0.2.0-beta.tar.gz";
#' download.file(ft.url, basename(ft.url));
#' # Extract it into canvasDevice/fonts and add the directory to search path
#' ft.dir = system.file("fonts", package = "canvasDevice");
#' untar(basename(ft.url), exdir = ft.dir, compressed = "gzip");
#' canvasFontPaths(file.path(ft.dir, "wqy-microhei"));
#' # Register this font file and assign the family name "wqy".
#' # Other font faces will be the same with regular by default
#' registerFontFamily("wqy", regular = "wqy-microhei.ttc");
#' 
#' # A more concise way to add font is using absolute path,
#' # without calling canvasFontPaths()
#' # registerFontFamily("wqy", file.path(ft.dir, "wqy-microhei/wqy-microhei.ttc"));
#' 
#' # List available font families
#' canvasFonts();
#' # Now it shows that we can use the family "wqy" in canvas()
#' canvas("testfont.canvas");
#' # Select font family globally
#' op = par(family = "serif", font.lab = 2);
#' # Inline selecting font
#' plot(1, type = "n");
#' text(1, 1, intToUtf8(c(20013, 25991)), family = "wqy", font = 1, cex = 2);
#' dev.off();
#' canvas2html("testfont.canvas");
#' setwd(wd);
#' }
#' 
registerFontFamily = function(family,
                     regular,
                     bold = NULL,
                     italic = NULL,
                     bolditalic = NULL,
                     symbol = NULL)
{
    family = as.character(family)[1];
    # Shouldn't modify default fonts
    if(family %in% c("sans", "serif", "mono") &
           all(c("sans", "serif", "mono") %in% canvasFonts()))
        stop("default font families('sans', 'serif', 'mono') cannot be modified");
    # The maximum length for font family name is 200 bytes
    
    if(nchar(family, type = "bytes") > 200)
        stop("family name is too long (max 200 bytes)");
    
    r = .Call("canvasLoadFont", .checkFontPath(regular, "regular"),
              PACKAGE = "canvasDevice");
    
    # If other font faces are not specified, use the regular one
    b = if(is.null(bold)) r
        else .Call("canvasLoadFont", .checkFontPath(bold, "bold"),
                   PACKAGE = "canvasDevice");
    
    i = if(is.null(italic)) r
    else .Call("canvasLoadFont", .checkFontPath(italic, "italic"),
               PACKAGE = "canvasDevice");
    
    bi = if(is.null(bolditalic)) r
    else .Call("canvasLoadFont", .checkFontPath(bolditalic, "bolditalic"),
               PACKAGE = "canvasDevice");
    
    s = if(is.null(symbol)) r
    else .Call("canvasLoadFont", .checkFontPath(symbol, "symbol"),
               PACKAGE = "canvasDevice");
    
    lst = .canvasInternal$.fontList;
    newfamily = list(regular = r, bold = b,
                     italic = i, bolditalic = bi, symbol = s);
    # list of font pointers separated into families 
    lst[[family]] = newfamily;
    .canvasInternal$.fontList = lst;
    .canvasInternal$.fontPointerList = c(.canvasInternal$.fontPointerList, newfamily);
    
    invisible(canvasFonts());
}

.registerDefaultFonts = function()
{
    packageStartupMessage("Loading fonts...");

    sans.r = system.file("fonts", "Lato-Reg.ttf", package = "canvasDevice");
    sans.b = system.file("fonts", "Lato-Bol.ttf", package = "canvasDevice");
    sans.i = system.file("fonts", "Lato-RegIta.ttf", package = "canvasDevice");
    sans.bi = system.file("fonts", "Lato-BolIta.ttf", package = "canvasDevice");
    
    serif.r = system.file("fonts", "VeraSerif.ttf", package = "canvasDevice");
    serif.b = system.file("fonts", "VeraSerif-Bold.ttf", package = "canvasDevice");
    serif.i = system.file("fonts", "DroidSerif-Italic.ttf", package = "canvasDevice");
    serif.bi = system.file("fonts", "DroidSerif-BoldItalic.ttf", package = "canvasDevice");
    
    mono.r = system.file("fonts", "VeraMono.ttf", package = "canvasDevice");
    mono.b = system.file("fonts", "VeraMoBd.ttf", package = "canvasDevice");
    mono.i = system.file("fonts", "VeraMoIt.ttf", package = "canvasDevice");
    mono.bi = system.file("fonts", "VeraMoBI.ttf", package = "canvasDevice");
    
    registerFontFamily("sans", sans.r, sans.b, sans.i, sans.bi, NULL);
    registerFontFamily("serif", serif.r, serif.b, serif.i, serif.bi, NULL);
    registerFontFamily("mono", mono.r, mono.b, mono.i, mono.bi, NULL);
    
    # We do some "hacks" here. For default families(sans, serif, mono),
    # we want to set their symbol fonts to be serif-italic
    lst = .canvasInternal$.fontList;
    lst[["sans"]][["symbol"]] = lst[["serif"]][["italic"]];
    lst[["serif"]][["symbol"]] = lst[["serif"]][["italic"]];
    lst[["mono"]][["symbol"]] = lst[["serif"]][["italic"]];
    .canvasInternal$.fontList = lst;
    
    packageStartupMessage("Loading fonts finished.");
    
    invisible(NULL);
}

# list of font pointers separated into families 
.unloadFonts = function()
{
    lst = unique(unlist(.canvasInternal$.fontPointerList));
    for(i in seq_along(lst))
    {
        .Call("canvasUnloadFont", lst[[i]], PACKAGE = "canvasDevice");
        # list of font pointers separated into families 
    }
    .canvasInternal$.fontList = list();
    .canvasInternal$.fontPointerList = list();
    gc();
    invisible(NULL);
}
