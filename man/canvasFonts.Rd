\name{canvasFonts}
\alias{canvasFonts}
\title{List available font families for canvas device}
\usage{
  canvasFonts()
}
\value{
  A character vector of available font family names
}
\description{
  This function lists font families currently available
  that can be used by canvas device through
  \code{par("family")}.
}
\details{
  By default there are three font families loaded
  automatically, i.e., "sans", "serif" and "mono". If you
  want to use other font families in canvas device, you
  need to call \code{\link{registerFontFamily}()} to
  register new fonts by specifying a family name and
  corresponding font file paths. See
  \code{\link{registerFontFamily}()} for details about
  what's the meaning of "family name" in this context, as
  well as a complete example of registering and using a new
  font.
}
\examples{
canvasFonts()
}
\author{
  Yixuan Qiu <\url{http://yixuan.cos.name/}>
}
\seealso{
  \code{\link{registerFontFamily}()}
}

