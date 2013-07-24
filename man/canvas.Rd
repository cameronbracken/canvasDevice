\name{canvas}
\alias{canvas}
\title{Canvas Graphics Device}
\usage{
  canvas(file = "", width = 640, height = 480,
    bg = "transparent", ...)
}
\arguments{
  \item{file}{A character string indicating the desired
  path to the output file.}

  \item{width}{The width of the output figure, in
  \bold{pixels}.}

  \item{height}{The height of the output figure, in
  \bold{pixels}.}

  \item{bg}{The starting background color for the plot.}
}
\value{
  Returns NULL invisibly.
}
\description{
  R graphics device targetting the HTML canvas element
}
\details{
  Initializes a new graphics device that implements the
  CanvasRenderingContext2D javascript api.
}
\examples{
\dontrun{
	# very simple plot
canvas(600, 600, file="plot.js")
plot(rnorm(4000),rnorm(4000),col="#ff000018",pch=19,cex=2) # semi-transparent red
dev.off() # creates a file "plot.js" with the above plot
}
}
\keyword{device}

