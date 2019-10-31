REM               GMT EXAMPLE 50
REM
REM Purpose:      Illustrate different statistical distributions in gmtmath
REM GMT modules:  math, set, plot, text
REM

gmt begin ex50
	REM Left column have all the PDFs
	gmt set FONT_ANNOT_PRIMARY 10p,Helvetica,black
	REM Binomial distribution
	gmt math -T0/8/1 0.25 8 T BPDF = p.txt
	gmt plot -R-0.6/8.6/0/0.35 -JX3i/0.5i -Glightgreen p.txt -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -X1i -Y0.8i
	REM Poisson distribution
	gmt math -T0/8/1 T 2 PPDF = p.txt
	gmt plot -R-0.6/8.6/0/0.3 -Glightgreen p.txt -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot normal distribution
	gmt math -T-4/4/0.1 T ZPDF = p.txt
	gmt plot -R-4/4/0/0.4 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot t distribution
	gmt plot -R-4/4/0/0.4 p.txt -W1p,lightgray -BWS -Bxa1 -Byaf -Y0.9i
	gmt math -T-4/4/0.1 T 3 TPDF = p.txt
	gmt plot p.txt -L+yb -Glightgreen -W1p
	REM Plot F distribution
	gmt math -T0/6/0.02 T 20 12 FPDF = p.txt
	gmt plot -R0/6/0/1.02 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot Laplace distribution
	gmt math -T-4/4/0.1 T LPDF = p.txt
	gmt plot -R-4/4/0/0.5 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot Exponential distribution
	gmt math -T0/4/0.1 T 2 EPDF = p.txt
	gmt plot -R0/4/0/2.0 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot Rayleigh distribution
	gmt math -T0/6/0.1 T RPDF = p.txt
	gmt plot -R0/6/0/0.7 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot Weibull distribution
	gmt math -T0/6/0.1 T 1 1.5 WPDF = p.txt
	gmt plot -R0/6/0/0.8 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Plot Chi-squared distribution
	gmt math -T0/12/0.1 T 4 CHI2PDF = p.txt
	gmt plot -R0/12/0/0.20 p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i
	REM Right column has all the CDF
	REM Plot binomial cumulative distribution
	gmt math -T0/8/1 0.25 8 T BCDF = p.txt
	gmt plot -R-0.6/8.6/0/1.02 -Glightred p.txt -Sb0.8u -W0.5p -BES -Bxa1 -Byaf -X3.5i -Y-8.1i
	REM Plot Poisson cumulative distribution
	gmt math -T0/8/1 T 2 PCDF = p.txt
	gmt plot -R-0.6/8.6/0/1.02 -Glightred p.txt -Sb0.8u -W0.5p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot normal cumulative distribution
	gmt math -T-4/4/0.1 T ZCDF = p.txt
	gmt plot -R-4/4/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot t cumulative distribution
	gmt plot -R-4/4/0/1.02 p.txt -W1p,lightgray -BES -Bxa1 -Byaf -Y0.9i
	gmt math -T-4/4/0.1 T 3 TCDF = p.txt
	gmt plot p.txt -L+yb -Glightred -W1p
	REM Plot F cumulative distribution
	gmt math -T0/6/0.02 T 20 12 FCDF = p.txt
	gmt plot -R0/6/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot Laplace cumulative distribution
	gmt math -T-4/4/0.1 T LCDF = p.txt
	gmt plot -R-4/4/0/1.02  p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot Exponential cumulative distribution
	gmt math -T0/4/0.1 T 2 ECDF = p.txt
	gmt plot -R0/4/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot Rayleigh cumulative distribution
	gmt math -T0/6/0.1 T RCDF = p.txt
	gmt plot -R0/6/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot Weibull cumulative distribution
	gmt math -T0/6/0.1 T 1 1.5 WCDF = p.txt
	gmt plot -R0/6/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	REM Plot Chi-squared cumulative distribution
	gmt math -T0/12/0.1 T 4 CHI2CDF = p.txt
	gmt plot -R0/12/0/1.02 p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i
	gmt text -R0/6.5/0/1.25 -Jx1i -N -X-3.5i -F+f18p+cTC+t"Probability @;lightgreen;Density@;; and @;lightred;Cumulative@;; Distribution Functions"

	echo 3.25 0.9 Binomial P@-8,0.25@-	> tmp.txt
	echo 3.25 1.8 Poisson P(@~l=2@~)	>> tmp.txt
	echo 3.25 2.7 Normal P(z)			>> tmp.txt
	echo 3.25 3.6 Student t(@~n=3@~)	>> tmp.txt
	echo 3.25 4.5 F(@~n@-1@-=20, n@-2@- = 12@~)	>> tmp.txt
	echo 3.25 5.4 Laplace P(z)					>> tmp.txt
	echo 3.25 6.3 Exponential P(@~l=2@~)		>> tmp.txt
	echo 3.25 7.2 Rayleigh P(z)					>> tmp.txt
	echo 3.25 8.1 Weibull P(z,1,1.5)			>> tmp.txt
	echo 3.25 9 @~c@~@+2@+(z,@~n=4@~)			>> tmp.txt
	gmt text tmp.txt -R0/6.5/0/10 -F+f14p,Times-Italic+jTC -Dj0.35i -N -Y-8.1i
	del p.txt
gmt end show
