REM             GMT EXAMPLE 50
REM
REM Purpose:    Illustrate different statistical distributions in gmtmath
REM GMT progs:  gmtmath, gmtset, psxy, pstext

echo GMT EXAMPLE 50
set ps=example_50.ps

REM Left column have all the PDFs
gmt set FONT_ANNOT_PRIMARY 10p,Helvetica,black
REM Binomial distribution
gmt math -T0/8/1 0.25 8 T BPDF = p.txt
gmt psxy -R-0.6/8.6/0/0.35 -JX3i/0.5i -P -K -Glightgreen p.txt -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -X1i -Y0.8i > %ps%
REM Poisson distribution
gmt math -T0/8/1 T 2 PPDF = p.txt
gmt psxy -R-0.6/8.6/0/0.3 -J -O -K -Glightgreen p.txt -Sb0.8u -W0.5p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot normal distribution
gmt math -T-4/4/0.1 T ZPDF = p.txt
gmt psxy -R-4/4/0/0.4 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot t distribution
gmt psxy -R-4/4/0/0.4 -J -O -K p.txt -W1p,lightgray -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
gmt math -T-4/4/0.1 T 3 TPDF = p.txt
gmt psxy -R -J -O -K p.txt -L+yb -Glightgreen -W1p >> %ps%
REM Plot F distribution
gmt math -T0/6/0.02 T 20 12 FPDF = p.txt
gmt psxy -R0/6/0/1 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Laplace distribution
gmt math -T-4/4/0.1 T LPDF = p.txt
gmt psxy -R-4/4/0/0.5 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Exponential distribution
gmt math -T0/4/0.1 T 2 EPDF = p.txt
gmt psxy -R0/4/0/2.0 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Rayleigh distribution
gmt math -T0/6/0.1 T RPDF = p.txt
gmt psxy -R0/6/0/0.7 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Weibull distribution
gmt math -T0/6/0.1 T 1 1.5 WPDF = p.txt
gmt psxy -R0/6/0/0.8 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Chi-squared distribution
gmt math -T0/12/0.1 T 4 CHI2PDF = p.txt
gmt psxy -R0/12/0/0.20 -J -O -K p.txt -L+yb -Glightgreen -W1p -BWS -Bxa1 -Byaf -Y0.9i >> %ps%
REM Right column has all the CDF
REM Plot binomial cumulative distribution
gmt math -T0/8/1 0.25 8 T BCDF = p.txt
gmt psxy -R-0.6/8.6/0/1 -J -O -K -Glightred p.txt -Sb0.8u -W0.5p -BES -Bxa1 -Byaf -X3.5i -Y-8.1i >> %ps%
REM Plot Poisson cumulative distribution
gmt math -T0/8/1 T 2 PCDF = p.txt
gmt psxy -R-0.6/8.6/0/1 -J -O -K -Glightred p.txt -Sb0.8u -W0.5p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot normal cumulative distribution
gmt math -T-4/4/0.1 T ZCDF = p.txt
gmt psxy -R-4/4/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot t cumulative distribution
gmt psxy -R-4/4/0/1 -J -O -K p.txt -W1p,lightgray -BES -Bxa1 -Byaf -Y0.9i >> %ps%
gmt math -T-4/4/0.1 T 3 TCDF = p.txt
gmt psxy -R -J -O -K p.txt -L+yb -Glightred -W1p >> %ps%
REM Plot F cumulative distribution
gmt math -T0/6/0.02 T 20 12 FCDF = p.txt
gmt psxy -R0/6/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Laplace cumulative distribution
gmt math -T-4/4/0.1 T LCDF = p.txt
gmt psxy -R-4/4/0/1  -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Exponential cumulative distribution
gmt math -T0/4/0.1 T 2 ECDF = p.txt
gmt psxy -R0/4/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Rayleigh cumulative distribution
gmt math -T0/6/0.1 T RCDF = p.txt
gmt psxy -R0/6/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Weibull cumulative distribution
gmt math -T0/6/0.1 T 1 1.5 WCDF = p.txt
gmt psxy -R0/6/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
REM Plot Chi-squared cumulative distribution
gmt math -T0/12/0.1 T 4 CHI2CDF = p.txt
gmt psxy -R0/12/0/1 -J -O -K p.txt -L+yb -Glightred -W1p -BES -Bxa1 -Byaf -Y0.9i >> %ps%
echo "Probability @;lightgreen;Density@;; and @;lightred;Cumulative@;; Distribution Functions" | gmt pstext -R0/6.5/0/1.25 -Jx1i -N -X-3.5i -O -K -F+f18p+cTC >> %ps%
echo 3.25 0.9 Binomial P@-8,0.25@- > tmp
echo 3.25 1.8 Poisson P(@~l=2@~) >> tmp
echo 3.25 2.7 Normal P(z) >> tmp
echo 3.25 3.6 Student t(@~n=3@~) >> tmp
echo 3.25 4.5 F(@~n@-1@-=20, n@-2@- = 12@~) >> tmp
echo 3.25 5.4 Laplace P(z) >> tmp
echo 3.25 6.3 Exponential P(@~l=2@~) >> tmp
echo 3.25 7.2 Rayleigh P(z) >> tmp
echo 3.25 8.1 Weibull P(z,1,1.5) >> tmp
echo 3.25 9 @~c@~@+2@+(z,@~n=4@~) >> tmp
gmt pstext -R0/6.5/0/10 -J -O -F+f14p,Times-Italic+jTC -Dj0.35i -N -Y-8.1i tmp >> %ps%
del p.txt
