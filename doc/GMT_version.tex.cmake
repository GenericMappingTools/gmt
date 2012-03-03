%-----------------------------------------------------------------
% $Id$
%
% The GMT Documentation Project
% Copyright (c) 2000-2011.
% P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
%-----------------------------------------------------------------
%
\newcommand{\SVNVERSION}{@SVN_VERSION@}
\newcommand{\SVNVERSIONSTRING}{%
\ifx\SVNVERSION\@empty% no svn version
\else\ (r\SVNVERSION)% print svn version
\fi}
\newcommand{\GMTDOCVERSION}{@GMT_PACKAGE_VERSION@\SVNVERSIONSTRING}
\newcommand{\GMTDOCDATE}{@MONTHNAME@ @YEAR@}
\newcommand{\GMTDOCYEAR}{@YEAR@}
\newcommand{\GSHHSVERSION}{@GSHHS_VERSION@}
