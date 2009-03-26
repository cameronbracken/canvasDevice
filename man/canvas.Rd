\name{canvas}
\alias{canvas}
\title{
  R graphics device targetting the HTML canvas element
}
\description{
  \code{canvas} initializes a new graphics device that implements
  the CanvasRenderingContext2D javascript api.
}
\usage{
canvas(width = 640, height = 480, file="",...)
}
\arguments{
  \item{width}{width of the plot area.}
  \item{height}{height of the plot area.}
  \item{file}{name of the file to be created.}
}
\value{
  Returns the NULL value invisibly.
}
\examples{
# very simple KDE
canvas(600, 600, file="plot.js")
plot(rnorm(4000),rnorm(4000),col="#ff000018",pch=19,cex=2) # semi-transparent red
dev.off() # creates a file "plot.png" with the above plot
}
\keyword{device}
