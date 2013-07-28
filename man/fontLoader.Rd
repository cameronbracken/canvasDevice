\name{fontLoader}
\alias{fontLoader}
\title{Font Metrics}
\usage{
  fontLoader(file = "")
}
\arguments{
  \item{file}{font file (must be readable by freetype)}

  \item{text}{string}
}
\value{
  A list containing metrics (width, height, ascent,
  descent) for the given string (in pixels).
}
\description{
  Get font metrics from freetype
}
\examples{
\dontrun{
   font_name <- '/Library/Fonts/Arial Regular.ttf'

   # one off usage on a multi-character string
   fontMetrics('A Text String', font_name)

   # get metrics for the whole alphabet!
   Vectorize(fontMetrics, 'string')(font_name, c(letters,LETTERS))
}
}
\keyword{device}

