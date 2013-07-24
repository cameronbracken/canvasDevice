.onLoad <-
function(libname, pkgname) {

  # Print out version banner
  pkgInfo <- read.dcf(file.path( libname, pkgname, "DESCRIPTION"))
  packageStartupMessage(sprintf("%s: %s (v%s)",
    pkgInfo[, "Package"], pkgInfo[, "Title"], pkgInfo[, "Version"]))

}

# Any variables defined in here will be hidden
# from normal users.
.canvasInternal <- new.env()
