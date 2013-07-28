\name{canvasFontPaths}
\alias{canvasFontPaths}
\title{Get/Set font search paths}
\usage{
  canvasFontPaths(new)
}
\arguments{
  \item{new}{a character vector indicating the search paths
  to be prepended. If the argument is missing, the function
  will return the current search paths.}
}
\value{
  The updated search paths
}
\description{
  This function gets/sets the search paths for font files.
}
\details{
  Default search paths will be assigned when package is
  loaded: \itemize{ \item For Windows, it is
  \code{\%windir\%\\Fonts}

  \item For Mac OS, default paths are \code{/Library/Fonts}
  and \code{~/Library/Fonts} and their subdirectories

  \item For Linux, \code{/usr/share/fonts},
  \code{/usr/local/share/fonts}, \code{~/.fonts}, and their
  subdirectories }
}
\author{
  Yixuan Qiu <\url{http://yixuan.cos.name/}>
}
\seealso{
  See \code{\link{registerFontFamily}()} for details about
  how \pkg{canvasDevice} looks for font files. There is
  also a complete example showing the usage of these
  functions in the help page of
  \code{\link{registerFontFamily}()}
}

