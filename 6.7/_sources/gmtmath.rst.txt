.. index:: ! gmtmath
.. include:: module_core_purpose.rst_

****
math
****

|gmtmath_purpose|

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**gmt math** [ |-A|\ *t_f(t)*\ [**+e**]\ [**+r**]\ [**+s**\|\ **w**] ]
[ |-C|\ *cols* ]
[ |-E|\ *eigen* ]
[ |-I| ]
[ |-N|\ *n\_col*\ [/*t_col*] ]
[ |-Q|\ [**c**\|\ **i**\|\ **p**\|\ **n**] ]
[ |-S|\ [**f**\|\ **l**] ]
[ |-T|\ [*min*/*max*/*inc*\ [**+b**\|\ **i**\|\ **l**\|\ **n**]\|\ *file*\|\ *list*] ]
[ |SYN_OPT-V| ]
[ |SYN_OPT-b| ]
[ |SYN_OPT-d| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-f| ]
[ |SYN_OPT-g| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-o| ]
[ |SYN_OPT-q| ]
[ |SYN_OPT-s| ]
[ |SYN_OPT-w| ]
[ |SYN_OPT--| ]
*operand* [ *operand* ] **OPERATOR** [ *operand* ] **OPERATOR** ...
**=** [ *outfile* ]

|No-spaces|

Description
-----------

**math** will perform operations like add, subtract, multiply, and
numerous other operands on one or more table data files or constants using
`Reverse Polish Notation (RPN) <https://en.wikipedia.org/wiki/Reverse_Polish_notation>`_
syntax.  Arbitrarily complicated expressions may therefore be evaluated; the
final result is written to an output file [or standard output]. Data
operations are element-by-element, not matrix manipulations (except
where noted). Some operators only require one operand (see below). If no
data tables are used in the expression then options |-T|, |-N| can
be set (and optionally **-bo** to indicate the
data type for binary tables). If STDIN is given, the standard input will
be read and placed on the stack as if a file with that content had been
given on the command line. By default, all columns except the "time"
column are operated on, but this can be changed (see |-C|).
Complicated or frequently occurring expressions may be coded as a macro
for future use or stored and recalled via named memory locations.

.. include:: RPN_info.rst_

Required Arguments
------------------

*operand*
    If *operand* can be opened as a file it will be read as an ASCII (or
    binary, see **-bi**) table data file. If not
    a file, it is interpreted as a numerical constant or a special
    symbol (see below). The special argument STDIN means that standard input
    will be read and placed on the stack; STDIN can appear more than
    once if necessary.
*outfile*
    The name of a table data file that will hold the final result. If
    not given then the output is sent to standard output.

Optional Arguments
------------------

.. _-A:

**-A**\ *t_f(t)*\ [**+e**]\ [**+r**]\ [**+s**\|\ **w**]
    Requires |-N| and will partially initialize a table with values
    from the given file *t_f(t)* containing *t* and *f(t)* only. The *t* is
    placed in column *t_col* while *f(t)* goes into column *n_col - 1*
    (see |-N|) and is called **b**.  The stack table is then the :math:`m \times (n+1)`
    augmented matrix [ **A** \| **b** ] and you want to solve for the least squares solution
    **x** to the matrix equation **Ax** = **b**. Usually, you will need
    to fill in the remaining columns in **A** using the various functions
    that defines the linear model you are trying to fit. If used with operators
    **LSQFIT** and **SVDFIT** you can optionally append some modifiers:

    - **+e** - Evaluate the solution and write a data set with four columns:
      *t*, *f(t)*, the model solution and residuals at *t*, respectively
      [Default writes one column with model coefficients **x**].
    - **+r** - Only place f(t) (i.e., **b**) and leave the **A** part of the
      augmented matrix equation alone.
    - **+s** - Your *t_f(t)* has a third column with 1-sigma uncertainties, or
    - **+w** - Your *t_f(t* table has a third column with weights.

    **Note**: If either **+s** or **+w** are used we find the weighted solution. The weights
    (or sigmas) will be output as the last column if **+e** is in effect.

.. _-C:

**-C**\ *cols*
    Select the columns that will be operated on until next occurrence of
    |-C|. List columns separated by commas; ranges like 1,3-5,7 are
    allowed, plus **-Cx** can be used for **-C**\ 0 and **-Cy** can be used for **-C**\ 1.
    |-C| (no arguments) resets the default action of using
    all columns except time column (see |-N|). **-Ca** selects all
    columns, including time column, while **-Cr** reverses (toggles) the
    current choices.  When |-C| is in effect it also controls which
    columns from a file will be placed on the stack.

.. _-E:

**-E**\ *eigen*
    Sets the minimum eigenvalue used by operators **LSQFIT** and **SVDFIT** [1e-7].
    Smaller eigenvalues are set to zero and will not be considered in the
    solution.

.. _-I:

**-I**
    Reverses the output row sequence from ascending time to descending [ascending].

.. _-N:

**-N**\ *n_col*\ [/*t_col*]
    Select the number of columns and optionally the column number that
    contains the "time" variable [0]. Columns are numbered starting at 0
    [2/0]. If input files are specified then |-N| will add any missing
    columns.

.. _-Q:

**-Q**\ [**c**\|\ **i**\|\ **p**\|\ **n**]
    Quick mode for scalar calculation. Internally sets the equivalent of **-Ca** **-N**\ 1/0 **-T**\ 1.
    In this mode, constants may have dimensional units (i.e., **c**, **i**, or **p**),
    and will be converted to internal *inches* before computing. If one or more constants
    with units are encountered then the final answer will be reported in the unit set by
    :term:`PROJ_LENGTH_UNIT`, unless overridden by appending another unit. Alternatively,
    append **n** for a non-dimensional result, meaning no unit conversion during output.
    To avoid any unit conversion on input, just do not use units.

.. _-S:

**-S**\ [**f**\|\ **l**]
    Only report the first or last row of the results [Default outputs all
    rows]. This is useful if you have computed a statistic (say the
    **MODE**) and only want to report a single number instead of
    numerous records with identical values. Append **l** to get the last
    row and **f** to get the first row only [Default].

.. _-T:

**-T**\ [*min*/*max*/*inc*\ [**+b**\|\ **i**\|\ **l**\|\ **n**]\|\ *file*\|\ *list*]
    Required when no input files are given. Builds an array for
    the "time" column (see |-N|). If there is no time column
    (i.e., your input has only data columns), give |-T| with
    no arguments; this also implies **-Ca**.
    For details on array creation, see `Generate 1-D Array`_.

.. |Add_-V| replace:: |Add_-V_links|
.. include:: explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-bi| unicode:: 0x20 .. just an invisible code
.. include:: explain_-bi.rst_

.. |Add_-bo| replace:: [Default is same as input, but see **-o**]
.. include:: explain_-bo.rst_

.. |Add_-d| unicode:: 0x20 .. just an invisible code
.. include:: explain_-d.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: explain_-e.rst_

.. |Add_-f| unicode:: 0x20 .. just an invisible code
.. include:: explain_-f.rst_

.. |Add_-g| unicode:: 0x20 .. just an invisible code
.. include:: explain_-g.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: explain_-h.rst_

.. include:: explain_-icols.rst_

.. include:: explain_-ocols.rst_

.. include:: explain_-q.rst_

.. include:: explain_-s.rst_

.. include:: explain_-w.rst_

.. include:: explain_help.rst_

.. include:: explain_array.rst_

Operators
---------

Choose among the following operators. Here, "args" are the number of input
and output arguments.

=============== ======= ============================================================================================= ===================== 
 Operator        args    Returns                                                                                       Type of Function   
=============== ======= ============================================================================================= ===================== 
  **ABS**         1 1     Absolute value of A                                                                           Arithmetic         
  **ACOS**        1 1     Inverse cosine (result in radians)                                                            Calculus           
  **ACOSD**       1 1     Inverse cosine (result in degrees)                                                            Calculus           
  **ACOSH**       1 1     Inverse hyperbolic cosine                                                                     Calculus           
  **ACOT**        1 1     Inverse of cotangent (result in radians)                                                      Calculus           
  **ACOTD**       1 1     Inverse of cotangent (result in degrees)                                                      Calculus           
  **ACSC**        1 1     Inverse of cosecant (result in radians)                                                       Calculus           
  **ACSCD**       1 1     Inverse of cosecant (result in degrees)                                                       Calculus           
  **ADD**         2 1     A + B (addition)                                                                              Arithmetic         
  **AND**         2 1     B if A equals NaN, else A                                                                     Logic              
  **ASEC**        1 1     Inverse of secant (result in radians)                                                         Calculus           
  **ASECD**       1 1     Inverse of secant (result in degrees)                                                         Calculus           
  **ASIN**        1 1     Inverse of sine (result in radians)                                                           Calculus           
  **ASIND**       1 1     Inverse of sine (result in degrees)                                                           Calculus           
  **ASINH**       1 1     Inverse of hyperbolic sine                                                                    Calculus           
  **ATAN**        1 1     Inverse of tangent (result in radians)                                                        Calculus           
  **ATAN2**       2 1     Inverse of tangent of A/B  (result in radians)                                                Calculus           
  **ATAN2D**      2 1     Inverse of tangent of A/B (result in degrees)                                                 Calculus           
  **ATAND**       1 1     Inverse of tangent (result in degrees)                                                        Calculus           
  **ATANH**       1 1     Inverse of hyperbolic tangent                                                                 Calculus           
  **BCDF**        3 1     Binomial cumulative distribution function for p = A, n = B, and x = C                         Probability        
  **BEI**         1 1     Kelvin function bei (A)                                                                       Special Functions  
  **BER**         1 1     Kelvin function ber (A)                                                                       Special Functions  
  **BITAND**      2 1     A & B (bitwise AND operator)                                                                  Logic              
  **BITLEFT**     2 1     A << B (bitwise left-shift operator)                                                          Arithmetic         
  **BITNOT**      1 1     ~A (bitwise NOT operator, i.e., return two’s complement)                                      Logic              
  **BITOR**       2 1     A | B (bitwise OR operator)                                                                   Logic              
  **BITRIGHT**    2 1     A >> B (bitwise right-shift operator)                                                         Arithmetic         
  **BITTEST**     2 1     1 if bit B of A is set, else 0 (bitwise TEST operator) n                                      Logic              
  **BITXOR**      2 1     A ^ B (bitwise XOR operator)                                                                  Logic              
  **BPDF**        3 1     Binomial probability density function for p = A, n = B, and x = C                             Probability        
  **CEIL**        1 1     ceil (A) (smallest integer >= A)                                                              Logic              
  **CHICDF**      2 1     Chi-squared cumulative distribution function for chi2 = A and nu = B                          Probability        
  **CHICRIT**     2 1     Chi-squared distribution critical value for alpha = A and nu = B                              Probability        
  **CHIPDF**      2 1     Chi-squared probability density function for chi2 = A and nu = B                              Probability        
  **COL**         1 1     Places column A on the stack                                                                  Special Operators  
  **COMB**        2 1     Combinations n_C_r, with n = A and r = B                                                      Probability        
  **CORRCOEFF**   2 1     Correlation coefficient r(A, B)                                                               Probability        
  **COS**         1 1     Cosine of A (A in radians)                                                                    Calculus           
  **COSD**        1 1     Cosine of A (A in degrees)                                                                    Calculus           
  **COSH**        1 1     Hyperbolic cosine of A                                                                        Calculus           
  **COT**         1 1     Cotangent of A (A in radians)                                                                 Calculus           
  **COTD**        1 1     Cotangent of A (A in degrees)                                                                 Calculus           
  **CSC**         1 1     Cosecant of A (A in radians)                                                                  Calculus           
  **CSCD**        1 1     Cosecant of A (A in degrees)                                                                  Calculus           
  **D2DT2**       1 1     d\ :sup:`2`\ (A)/dt\ :sup:`2` Central 2nd derivative                                          Calculus           
  **D2R**         1 1     Converts degrees to radians                                                                   Special Operators  
  **DDT**         1 1     d(A)/dt Central 1st derivative                                                                Calculus           
  **DEG2KM**      1 1     Converts spherical degrees to kilometers                                                      Special Operators  
  **DENAN**       2 1     Replace NaNs in A with values from B                                                          Logic              
  **DIFF**        1 1     Forward difference between adjacent elements of A (A[1]-A[0], A[2]-A[1], …, NaN)              Arithmetic         
  **DILOG**       1 1     Dilogarithm (Spence's) function                                                               Special Functions  
  **DIV**         2 1     A / B (division)                                                                              Arithmetic         
  **DUP**         1 2     Places duplicate of A on the stack                                                            Special Operators  
  **ECDF**        2 1     Exponential cumulative distribution function for x = A and lambda = B                         Probability        
  **ECRIT**       2 1     Exponential distribution critical value for alpha = A and lambda = B                          Probability        
  **EPDF**        2 1     Exponential probability density function for x = A and lambda = B                             Probability        
  **EQ**          2 1     1 if A equals B, else 0                                                                       Logic              
  **ERF**         1 1     Error function erf (A)                                                                        Probability        
  **ERFC**        1 1     Complementary Error function erfc (A)                                                         Probability        
  **ERFINV**      1 1     Inverse error function of A                                                                   Probability        
  **EXCH**        2 2     Exchanges A and B on the stack                                                                Special Operators  
  **EXP**         1 1     E raised to a power.                                                                          Arithmetic         
  **FACT**        1 1     A! (A factorial)                                                                              Arithmetic         
  **FCDF**        3 1     F cumulative distribution function for F = A, nu1 = B, and nu2 = C                            Probability        
  **FCRIT**       3 1     F distribution critical value for alpha = A, nu1 = B, and nu2 = C                             Probability        
  **FLIPUD**      1 1     Reverse order of each column                                                                  Special Operators  
  **FLOOR**       1 1     greatest integer less than or equal to A                                                      Logic              
  **FMOD**        2 1     A % B (remainder after truncated division)                                                    Arithmetic         
  **FPDF**        3 1     F probability density function for F = A, nu1 = B, and nu2 = C                                Probability        
  **GE**          2 1     1 if A >= (greater or equal than) B, else 0                                                   Logic              
  **GT**          2 1     1 if A > (greater than) B, else 0                                                             Logic              
  **HSV2LAB**     3 3     Convert *h,s,v* triplets to *l,a,b* triplets, with h = A (0-360), s = B and v = C (0-1)       Special Operators  
  **HSV2RGB**     3 3     Convert *h,s,v* triplets to *r,g,b* triplets, with h = A (0-360), s = B and v = C (0-1)       Special Operators  
  **HSV2XYZ**     3 3     Convert *h,s,v* triplets to *x,t,z* triplets, with h = A (0-360), s = B and v = C (0-1)       Special Operators  
  **HYPOT**       2 1     Hypotenuse of a right triangle of sides A and B (= sqrt (A\ :sup:`2` + B\ :sup:`2`))          Calculus           
  **I0**          1 1     Modified Bessel function of A (1st kind, order 0)                                             Special Functions  
  **I1**          1 1     Modified Bessel function of A (1st kind, order 1)                                             Special Functions  
  **IFELSE**      3 1     B if A is not equal to 0, else C                                                              Logic              
  **IN**          2 1     Modified Bessel function of A (1st kind, order B)                                             Special Functions  
  **INRANGE**     3 1     1 if B <= A <= C, else 0                                                                      Logic              
  **INT**         1 1     Numerically integrate A                                                                       Calculus           
  **INV**         1 1     Invert (1/A)                                                                                  Arithmetic         
  **ISFINITE**    1 1     1 if A is finite, else 0                                                                      Logic              
  **ISNAN**       1 1     1 if A equals NaN, else 0                                                                     Logic              
  **J0**          1 1     Bessel function of A (1st kind, order 0)                                                      Special Functions  
  **J1**          1 1     Bessel function of A (1st kind, order 1)                                                      Special Functions  
  **JN**          2 1     Bessel function of A (1st kind, order B)                                                      Special Functions  
  **K0**          1 1     Modified Kelvin function of A (2nd kind, order 0)                                             Special Functions  
  **K1**          1 1     Modified Bessel function of A (2nd kind, order 1)                                             Special Functions  
  **KEI**         1 1     Kelvin function kei (A)                                                                       Special Functions  
  **KER**         1 1     Kelvin function ker (A)                                                                       Special Functions  
  **KM2DEG**      1 1     Converts kilometers to spherical degrees                                                      Special Operators  
  **KN**          2 1     Modified Bessel function of A (2nd kind, order B)                                             Special Functions  
  **KURT**        1 1     Kurtosis of A                                                                                 Probability        
  **LAB2HSV**     3 3     Convert *l,a,b* triplets to *h,s,v* triplets                                                  Special Operators  
  **LAB2RGB**     3 3     Convert *l,a,b* triplets to *r,g,b* triplets                                                  Special Operators  
  **LAB2XYZ**     3 3     Convert *l,a,b* triplets to *x,y,z* triplets                                                  Special Operators  
  **LCDF**        1 1     Laplace cumulative distribution function for z = A                                            Probability        
  **LCRIT**       1 1     Laplace distribution critical value for alpha = A                                             Probability        
  **LE**          2 1     1 if A <= (equal or smaller than) B, else 0                                                   Logic              
  **LMSSCL**      1 1     LMS (Least Median of Squares) scale estimate (LMS STD) of A                                   Probability        
  **LMSSCLW**     2 1     Weighted LMS scale estimate (LMS STD) of A for weights in B                                   Probability        
  **LOG**         1 1     log (A) (natural logarithm)                                                                   Arithmetic         
  **LOG10**       1 1     log\ :sub:`10` (A) (logarithm base 10)                                                        Arithmetic         
  **LOG1P**       1 1     log (1+A) (natural logarithm, accurate for small A)                                           Arithmetic         
  **LOG2**        1 1     log\ :sub:`2` (A) (logarithm base 2)                                                          Arithmetic         
  **LOWER**       1 1     The lowest (minimum) value of A                                                               Arithmetic         
  **LPDF**        1 1     Laplace probability density function for z = A                                                Probability        
  **LRAND**       2 1     Laplace random noise with mean A and std. deviation B                                         Probability        
  **LSQFIT**      1 0     Stack is [**A** | **b**]; return least squares solution **x** = **A** \\ **b**                Special Operators  
  **LT**          2 1     1 if A < (smaller than) B, else 0                                                             Logic              
  **MAD**         1 1     Median Absolute Deviation (L1 STD) of A                                                       Probability        
  **MADW**        2 1     Weighted Median Absolute Deviation (L1 STD) of A for weights in B                             Probability        
  **MAX**         2 1     Maximum of A and B                                                                            Probability        
  **MEAN**        1 1     Mean value of A                                                                               Probability        
  **MEANW**       2 1     Weighted mean value of A for weights in B                                                     Probability        
  **MEDIAN**      1 1     Median value of A                                                                             Probability        
  **MEDIANW**     2 1     Weighted median value of A for weights in B                                                   Probability        
  **MIN**         2 1     Minimum of A and B                                                                            Probability        
  **MOD**         2 1     A mod B (remainder after floored division)                                                    Arithmetic         
  **MODE**        1 1     Mode value (Least Median of Squares) of A                                                     Probability        
  **MODEW**       2 1     Weighted mode value (Least Median of Squares) of A for weights in B                           Probability        
  **MUL**         2 1     A x B (multiplication)                                                                        Arithmetic         
  **NAN**         2 1     NaN if A equals B, else A                                                                     Logic              
  **NEG**         1 1     Negative (-A)                                                                                 Arithmetic         
  **NEQ**         2 1     1 If A  is not equal to B, else 0                                                             Logic              
  **NORM**        1 1     Normalize (A) so min(A) = 0 and max(A) = 1                                                    Probability        
  **NOT**         1 1     NaN ia A is equal NaN, 1 if A is equal to 0, else 0                                           Logic              
  **NRAND**       2 1     Normal, random values with mean A and std. deviation B                                        Probability        
  **OR**          2 1     NaN if B equals NaN, else A                                                                   Logic              
  **PCDF**        2 1     Poisson cumulative distribution function for x = A and lambda = B                             Probability        
  **PERM**        2 1     Permutations n_P_r, with n = A and r = B                                                      Probability        
  **PLM**         3 1     Associated Legendre polynomial P(A) degree B order C                                          Special Functions  
  **PLMg**        3 1     Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention)      Special Functions  
  **POP**         1 0     Delete top element from the stack                                                             Special Operators  
  **POW**         2 1     A to the power of B                                                                           Arithmetic         
  **PPDF**        2 1     Poisson distribution P(x,lambda), with x = A and lambda = B                                   Probability        
  **PQUANT**      2 1     The B’th quantile (0-100%) of A                                                               Probability        
  **PQUANTW**     3 1     The C’th weighted quantile (0-100%) of A for weights in B                                     Probability        
  **PSI**         1 1     Psi (or Digamma) of A                                                                         Special Functions  
  **PV**          3 1     Legendre function Pv(A) of degree v = real(B) + imag(C)                                       Special Functions  
  **QV**          3 1     Legendre function Qv(A) of degree v = real(B) + imag(C)                                       Special Functions  
  **R2**          2 1     Hypotenuse squared (= A\ :sup:`2` + B\ :sup:`2`)                                              Calculus           
  **R2D**         1 1     Convert radians to degrees                                                                    Special Operators  
  **RAND**        2 1     Uniform random values between A and B                                                         Probability        
  **RCDF**        1 1     Rayleigh cumulative distribution function for z = A                                           Probability        
  **RCRIT**       1 1     Rayleigh distribution critical value for alpha = A                                            Probability        
  **RGB2HSV**     3 3     Convert *r,g,b* triplets to *h,s,v* triplets, with r = A, g = B, and b = C (in 0-255 range)   Special Operators  
  **RGB2LAB**     3 3     Convert *r,g,b* triplets to *l,a,b* triplets, with r = A, g = B, and b = C (in 0-255 range)   Special Operators  
  **RGB2XYZ**     3 3     Convert *r,g,b* triplets to *x,y,z* triplets, with r = A, g = B, and b = C (in 0-255 range)   Special Operators  
  **RINT**        1 1     Rint (A) (round to integral value nearest to A)                                               Arithmetic         
  **RMS**         1 1     Root-mean-square of A                                                                         Arithmetic         
  **RMSW**        1 1     Weighted root-mean-square of A for weights in B                                               Arithmetic         
  **ROLL**        2 0     Cyclically shifts the top A stack items by an amount B                                        Special Operators  
  **ROOTS**       2 1     Treats col A as f(t) = 0 and returns its roots                                                Special Operators  
  **ROTT**        2 1     Rotate A by the (constant) shift B in the t-direction                                         Arithmetic         
  **RPDF**        1 1     Rayleigh probability density function for z = A                                               Probability        
  **SEC**         1 1     Secant of A (A in radians)                                                                    Calculus           
  **SECD**        1 1     Secant of A (A in degrees)                                                                    Calculus           
  **SIGN**        1 1     Sign (+1 or -1) of A                                                                          Logic              
  **SIN**         1 1     Sine of A (A in radians)                                                                      Calculus           
  **SINC**        1 1     Normalized sinc function.                                                                     Special Functions  
  **SIND**        1 1     Sine of A (A in degrees)                                                                      Calculus           
  **SINH**        1 1     Hiperbolic sine of A                                                                          Calculus           
  **SKEW**        1 1     Skewness of A                                                                                 Probability        
  **SQR**         1 1     Square (to the power of 2)                                                                    Arithmetic         
  **SQRT**        1 1     Square root                                                                                   Arithmetic         
  **STD**         1 1     Standard deviation of A                                                                       Probability        
  **STDW**        2 1     Weighted standard deviation of A for weights in B                                             Probability        
  **STEP**        1 1     Heaviside step function H(A)                                                                  Special Functions  
  **STEPT**       1 1     Heaviside step function H(t-A)                                                                Special Functions  
  **SUB**         2 1     A - B (subtraction)                                                                           Arithmetic         
  **SUM**         1 1     Cumulative sum of A                                                                           Arithmetic         
  **SVDFIT**      1 0     Stack is [**A** | **b**]; return **x** = **A** \\ **b** via SVD decomposition (see |-E)|      Special Operators  
  **TAN**         1 1     Tangent of A (A in radians)                                                                   Calculus           
  **TAND**        1 1     Tangent of A (A in degrees)                                                                   Calculus           
  **TANH**        1 1     Hyperbolic tangent of A                                                                       Calculus           
  **TAPER**       1 1     Unit weights cosine-tapered to zero within A of end margins                                   Special Operators  
  **TCDF**        2 1     Student’s t cumulative distribution function for t = A, and nu = B                            Probability        
  **TCRIT**       2 1     Student’s t distribution critical value for alpha = A and nu = B                              Probability        
  **TN**          2 1     Chebyshev polynomial Tn(-1<A<+1) of degree B                                                  Special Functions  
  **TPDF**        2 1      Student’s t probability density function for t = A, and nu = B                               Probability        
  **UPPER**       1 1     The highest (maximum) value of A                                                              Arithmetic         
  **VAR**         1 1     Variance of A                                                                                 Probability        
  **VARW**        2 1     Weighted variance of A for weights in B                                                       Probability        
  **VPDF**        3 1     Von Mises density distribution V(x,mu,kappa), with angles = A, mu = B, and kappa = C          Probability        
  **WCDF**        3 1     Weibull cumulative distribution function for x = A, scale = B, and shape = C                  Probability        
  **WCRIT**       3 1     Weibull distribution critical value for alpha = A, scale = B, and shape = C                   Probability        
  **WPDF**        3 1     Weibull density distribution P(x,scale,shape), with x = A, scale = B, and shape = C           Probability        
  **XOR**         2 1     B if A equals NaN, else A                                                                     Logic              
  **XYZ2HSV**     3 3     Convert *x,y,z* triplets to *h,s,v* triplets                                                  Special Operators  
  **XYZ2LAB**     3 3     Convert *x,y,z* triplets to *l,a,b* triplets                                                  Special Operators  
  **XYZ2RGB**     3 3     Convert *x,y,z* triplets to *r,g,b* triplets                                                  Special Operators  
  **Y0**          1 1     Bessel function of A (2nd kind, order 0)                                                      Special Functions  
  **Y1**          1 1     Bessel function of A (2nd kind, order 1)                                                      Special Functions  
  **YN**          2 1     Bessel function of A (2nd kind, order B)                                                      Special Functions  
  **ZCDF**        1 1     Normal cumulative distribution function for z = A                                             Probability        
  **ZCRIT**       1 1     Normal distribution critical value for alpha = A                                              Probability        
  **ZPDF**        1 1     Normal probability density function for z = A                                                 Probability        
=============== ======= ============================================================================================= =====================

Symbols
-------

The following symbols have special meaning:

+-------------+-----------------------------------------+
| **PI**      | 3.1415926...                            |
+-------------+-----------------------------------------+
| **E**       | 2.7182818...                            |
+-------------+-----------------------------------------+
| **EULER**   | 0.5772156...                            |
+-------------+-----------------------------------------+
| **PHI**     | 1.6180339... (golden ratio)             |
+-------------+-----------------------------------------+
| **EPS_F**   | 1.192092896e-07 (sgl. prec. eps)        |
+-------------+-----------------------------------------+
| **EPS_D**   | 2.2204460492503131e-16 (dbl. prec. eps) |
+-------------+-----------------------------------------+
| **TMIN**    | Minimum *t* value                       |
+-------------+-----------------------------------------+
| **TMAX**    | Maximum *t* value                       |
+-------------+-----------------------------------------+
| **TRANGE**  | Range of *t* values                     |
+-------------+-----------------------------------------+
| **TINC**    | *t* increment                           |
+-------------+-----------------------------------------+
| **N**       | The number of records                   |
+-------------+-----------------------------------------+
| **T**       | Table with *t*-coordinates              |
+-------------+-----------------------------------------+
| **TNORM**   | Table with normalized *t*-coordinates   |
+-------------+-----------------------------------------+
| **TROW**    | Table with row numbers 1, 2, ..., N-1   |
+-------------+-----------------------------------------+

.. include:: explain_precision.rst_


Notes On Operators
------------------

#. The operators **PLM** and **PLMg** calculate the associated Legendre
   polynomial of degree *L* and order *M* in *x* which must satisfy :math:`-1 \leq x \leq +1`
   and :math:`0 \leq M \leq L`. Here, *x*, *L*, and *M* are the three arguments preceding the
   operator. **PLM** is not normalized and includes the Condon-Shortley
   phase :math:`(-1)^M`. **PLMg** is normalized in the way that is most commonly
   used in geophysics. The Condon-Shortley phase can be added by using *-M* as argument.
   **PLM** will overflow at higher degrees, whereas **PLMg** is stable
   until ultra high degrees (at least 3000).

#. Files that have the same names as some operators, e.g., **ADD**,
   **SIGN**, **=**, etc. should be identified by prepending the current
   directory (i.e., ./).

#. The stack depth limit is hard-wired to 100.

#. All functions expecting a positive radius (e.g., **LOG**, **KEI**,
   etc.) are passed the absolute value of their argument.

#. The **DDT** and **D2DT2** functions only work on regularly spaced data.

#. All derivatives are based on central finite differences, with
   natural boundary conditions.

#. **ROOTS** must be the last operator on the stack, only followed by **=**.

.. include:: explain_sto_rcl_clr.rst_


#. The bitwise operators
   (**BITAND**, **BITLEFT**, **BITNOT**, **BITOR**, **BITRIGHT**,
   **BITTEST**, and **BITXOR**) convert a tables's double precision values
   to unsigned 64-bit ints to perform the bitwise operations. Consequently,
   the largest whole integer value that can be stored in a double precision
   value is 2\ :sup:`53` or 9,007,199,254,740,992. Any higher result will be masked
   to fit in the lower 54 bits.  Thus, bit operations are effectively limited
   to 54 bits.  All bitwise operators return NaN if given NaN arguments or
   bit-settings <= 0.

#. **TAPER** will interpret its argument to be a width in the same units as
   the time-axis, but if no time is provided (i.e., plain data tables) then
   the width is taken to be given in number of rows.

#. The color-triplet conversion functions (**RGB2HSV**, etc.) includes not
   only *r,g,b* and *h,s,v* triplet conversions, but also *l,a,b* (CIE L a b ) and
   sRGB (*x,y,z*) conversions between all four color spaces.  These functions
   behave differently whether |-Q| is used or not.  With |-Q| we expect
   three input constants and we place three output results on the stack.  Since
   only the top stack item is printed, you must use operators such as **POP** and
   **ROLL** to get to the item of interest.  Without |-Q|, these operators work
   across the three columns and modify the three column entries, returning their
   result as a single three-column item on the stack.
#. The **VPDF** operator expects angles in degrees.

Macros
------

Users may save their favorite operator combinations as macros via the
file *gmtmath.macros* in their current or user directory. The file may contain
any number of macros (one per record); comment lines starting with # are
skipped. The format for the macros is **name** = **arg1 arg2 ... arg2**
[ : *comment*] where **name** is how the macro will be used. When this
operator appears on the command line we simply replace it with the
listed argument list. No macro may call another macro. As an example,
the following macro expects that the time-column contains seafloor ages
in Myr and computes the predicted half-space bathymetry:

**DEPTH** = **SQRT** 350 **MUL** 2500 **ADD NEG** : *usage: DEPTH to return
half-space seafloor depths*

**Note**: Because geographic or time constants may be present in a macro, it
is required that the optional comment flag (:) must be followed by a space.
As another example, we show a macro **GPSWEEK** which determines which GPS week
a timestamp belongs to:

**GPSWEEK** = 1980-01-06T00:00:00 **SUB** 86400 **DIV** 7 **DIV FLOOR** : *usage: GPS week without rollover*

Active Column Selection
-----------------------

When **-C**\ *cols* is set then any operation, including loading of data from files, will
restrict which columns are affected.
To avoid unexpected results, note that if you issue a **-C**\ *cols* option *before* you load
in the data then only those columns will be updated, hence the unspecified columns will be zero.
On the other hand, if you load the file *first* and then issue **-C**\ *cols* then the unspecified
columns will have been loaded but are then ignored until you undo the effect of |-C|.

Absolute Time Column(s)
-----------------------

If input data have more than one column and the "time" column (set via |-N| [0])
contains absolute time, then the default output format for any *other* columns containing
absolute time will be reset to relative time.  Likewise, in scalar mode (|-Q|) the
time column will be operated on and hence it also will be formatted as relative
time.  Finally, if |-C| is used to include "time" in the columns operated on then
we likewise will reset that column's format to relative time. The user can override this behavior with a
suitable **-f** or **-fo** setting.  **Note**: We cannot guess what your operations on the
time column will do, hence this default behavior.  As examples, if you are computing time differences
then clearly relative time formatting is required, while if you are computing new absolute times
by, say, adding an interval to absolute times then you will need to use **-fo** to set
the output format for such columns to absolute time.

Scalar Math with Units
----------------------

If you use |-Q| to do simple calculations, please note that the support for dimensional units is
limited to converting a number ending in **c**, **i**, or **p** to internal *inches*.  Thus, while you can run
"gmt -Qc 1c 1c MUL =", you may be surprised that the output area is not 1 cm squared.  The reason is
that **gmt math** cannot keep track of what unit any particular item on the stack might be so it will
assume it is internally in inches and then scale the final output to cm.  In this particular case,
the unit is in inches squared and scaling by 2.54 once will give 0.3937 inch times cm as the unit.
Thus, conversions only work for linear unit calculations, such as gmt math -Qp 1c 0.5i ADD =, which
will return the result as 64.34 points.

Trailing Text
-------------

Any trailing text in the first input file will be passed to the output data set.  You can turn off the
output of text with **-on**.

Examples
--------

.. include:: explain_example.rst_

To add two plot dimensions of different units, we can run

::

    length=$(gmt math -Q 15c 2i ADD =)

To compute the ratio of two plot dimensions of different units, we select *non-dimensional* output and run

::

    ratio=$(gmt math -Qn 15c 2i DIV =)

To take the square root of the content of the second data column being
piped through **gmtmath** by process1 and pipe it through a 3rd process, use

::

    process1 | gmt math STDIN SQRT = | process3

To take :math:`\log_{10}` of the average of 2 data files, use

::

    gmt math file1.txt file2.txt ADD 0.5 MUL LOG10 = file3.txt

Given the file samples.txt, which holds seafloor ages in m.y. and seafloor
depth in m, use the relation depth(in m) = 2500 + 350 \* sqrt (age) to
print the depth anomalies:

::

    gmt math samples.txt T SQRT 350 MUL 2500 ADD SUB = | lpr

To take the average of columns 1 and 4-6 in the three data sets sizes.1,
sizes.2, and sizes.3, use

::

    gmt math -C1,4-6 sizes.1 sizes.2 ADD sizes.3 ADD 3 DIV = ave.txt

To take the 1-column data set ages.txt and calculate the modal value and
assign it to a variable, try

::

    mode_age=$(gmt math -S -T ages.txt MODE =)

To evaluate the dilog(x) function for coordinates given in the file t.txt:

::

    gmt math -Tt.txt T DILOG = dilog.txt

To demonstrate the use of stored variables, consider this sum of the
first 3 cosine harmonics where we store and repeatedly recall the
trigonometric argument (2\*pi\*T/360):

::

    gmt math -T0/360/1 2 PI MUL 360 DIV T MUL STO@kT COS @kT 2 MUL COS ADD @kT 3 MUL COS ADD = harmonics.txt

To use **gmtmath** as a RPN Hewlett-Packard calculator on scalars (i.e., no
input files) and calculate arbitrary expressions, use the |-Q| option.
As an example, we will calculate the value of Kei (((1 + 1.75)/2.2) +
cos (60)) and store the result in the shell variable z:

::

    z=$(gmt math -Q 1 1.75 ADD 2.2 DIV 60 COSD ADD KEI =)

To convert the r,g,b value for yellow to h,s,v and save the hue, try

::

    hue=$(gmt math -Q 255 255 0 RGB2HSV POP POP =)


To use **gmtmath** as a general least squares equation solver, imagine
that the current table is the augmented matrix [ **A** \| **b** ] and you want
the least squares solution **x** to the matrix equation **A** \* **x** = **b**. The
operator **LSQFIT** does this; it is your job to populate the matrix
correctly first. The |-A| option will facilitate this. Suppose you
have a 2-column file ty.txt with *t* and *y* and you would like to fit
a the model *y(t) = a + b\*t + c\*H(t-t0)*, where *H(t)* is the Heaviside step
function for a given t0 = 1.55. Then, you need a 4-column augmented
table loaded with *t* in column 1 and your observed *y* in column 3. The
calculation becomes

::

    gmt math -N4/1 -Aty.txt -C0 1 ADD -C2 1.55 STEPT ADD -Ca LSQFIT = solution.txt

Note we use the |-C| option to select which columns we are working on,
then make active all the columns we need (here all of them, with
**-Ca**) before calling **LSQFIT**. The second and fourth columns (col
numbers 1 and 3) are preloaded with *t* and *y*, respectively, the other
columns are zero. If you already have a pre-calculated table with the
augmented matrix [ **A** \| **b** ] in a file (say lsqsys.txt), the least squares
solution is simply

::

    gmt math -T lsqsys.txt LSQFIT = solution.txt

Users must be aware that when |-C| controls which columns are to be
active the control extends to placing columns from files as well.
Contrast the different result obtained by these very similar commands:

::

    echo 1 2 3 4 | gmt math STDIN -C3 1 ADD =
    1    2    3    5

versus

::

    echo 1 2 3 4 | gmt math -C3 STDIN 1 ADD =
    0    0    0    5

To calculate how many days there were in the 80s decade:

::

    gmt math -Q 1991-01-01T 1981-01-01T SUB --TIME_UNIT=d =

To determine the 1000th day since the beginning of the millennium:

::

    gmt math -Q 2001-01-01T 1000 ADD --TIME_UNIT=d -fT =


References
----------

Abramowitz, M., and I. A. Stegun, 1964, *Handbook of Mathematical
Functions*, Applied Mathematics Series, vol. 55, Dover, New York.

Holmes, S. A., and W. E. Featherstone, 2002, A unified approach to the
Clenshaw summation and the recursive computation of very high degree and
order normalized associated Legendre functions. *Journal of Geodesy*,
76, 279-299.

Press, W. H., S. A. Teukolsky, W. T. Vetterling, and B. P. Flannery,
1992, *Numerical Recipes*, 2nd edition, Cambridge Univ., New York.

Spanier, J., and K. B. Oldman, 1987, *An Atlas of Functions*, Hemisphere
Publishing Corp.

.. include:: RPN_MoreOn.rst_

See Also
--------

:doc:`gmt`,
:doc:`grdmath`
