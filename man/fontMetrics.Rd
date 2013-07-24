\name{fontMetrics}
\alias{fontMetrics}
\title{Freetype Metrics}
\usage{
  fontMetrics(file = "", string, ps = 12,
    encoding = "UTF-8")
}
\arguments{
  \item{file}{font file (must be readable by freetype)}

  \item{text}{string}
}
\value{
  A list containing metrics for the given string.
}
\description{
  Get font metrics from freetype
}
\examples{
\dontrun{
   fontMetrics('Ariel.ttf','stuff')
}
}
\keyword{device}

