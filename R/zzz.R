.onLoad <- function(libname, pkgname) {

    # Print out version banner
    pkgInfo <- read.dcf(file.path( libname, pkgname, "DESCRIPTION"))
    packageStartupMessage(sprintf("%s: %s (v%s)",
        pkgInfo[, "Package"], pkgInfo[, "Title"], pkgInfo[, "Version"]))

    .setDefaultCanvasFontPaths();
    .registerDefaultFonts();

}

.onUnload <- function(libpath) {
    #.unloadFonts();
    .canvasInternal <- NULL
    #library.dynam.unload("canvasDevice", libpath);
}
