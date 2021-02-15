REM Blending the NASA day and night views from the Blue and Black Marble mosaic
REM images using a day-night mask that goes through a full 24-hour cycle.  In addition,
REM we adjust the colors using the intensities derived from the slopes of the earth
REM relief grid.  We spin around at 24 frames per second where each frame advances by 1 minute.
REM Because we are not plotting anything, just manipulating images via grids, we must instead
REM create the frames and the final movie via the batch module.
REM
REM DEM:    @earth_relief_04m
REM Images: @earth_day_HD.tif @earth_night_HD.tif from the GMT cache server
REM 
REM The finished movie is available in our YouTube channel as well:
REM https://youtu.be/X8TojLs0NYk
REM The movie took ~4 minutes to render on a 24-core MacPro 2013.
REM To make a UHD (4k) movie, comment/uncomment the SIZE, INC and FILT settings
set SIZE=HD
set INC=11.25
set FILT=21
REM set SIZE=UHD
REM set INC=5.625
REM set FILT=10.5

REM Set up working directory
set CURRENTDIR="%cd%"
set main=%CURRENTDIR%\main.bat
set pre=%CURRENTDIR%\pre.bat
set postf=%CURRENTDIR%\post.bat

REM 1. Create preflight script to build data files needed in the REM
echo gmt begin > %pre%
  REM Set view time for Feb 27-28, 2021 with steps of 1 minute
  echo. gmt math -T2021-02-27T/2021-02-28T/1 -o0 --TIME_UNIT=m  T = %CURRENTDIR%\times.txt >> %pre%
  REM Create a %SIZE% DEM at %INC%x%INC% arc minutes to yield %SIZE% dimensions
  echo. gmt grdfilter @earth_relief_04m -Fg%FILT% -I%INC%m -D1 -r -Gtopo_%SIZE%.grd >> %pre%
  REM Create an intensity grid based on the %SIZE% DEM so we can see structures in the oceans
  echo. gmt grdgradient topo_%SIZE%.grd -Nt0.5 -A45 -Gintens_%SIZE%.grd >> %pre%
  REM Make sure our remote files have been downloaded
  echo. gmt which -Ga @earth_day_%SIZE%.tif @earth_night_%SIZE%.tif >> %pre%
echo gmt end >> %pre%

REM 2. Set up main script
echo gmt begin > %main%
  REM Let HSV minimum value go to zero and faint map border
  echo. gmt set COLOR_HSV_MIN_V 0 MAP_FRAME_PEN=faint >> %main%
  REM Make global grid with a smooth 2-degree day/night transition for this time.
  echo. for /f "delims=*" %%%%a in ('gmt solar -C -o0:1 -I+d%%BATCH_COL0%%') do set sunsolar=%%%%a >> %main%
  echo. gmt grdmath -Rd -I%INC%m -r %%sunsolar%% 2 DAYNIGHT = daynight_%SIZE%.grd >> %main%
  REM Blend the day and night Earth images using the weights, so that when w is 1
  REM we get the daytime view, and then adjust colors based on the intensity.
  echo. gmt grdmix @earth_day_%SIZE%.tif @earth_night_%SIZE%.tif -Wdaynight_%SIZE%.grd -Iintens_%SIZE%.grd -G%%BATCH_NAME%%.png -Ve >> %main%
echo gmt end >> %main%

REM 3. Create postflight script to build movie from the images
echo gmt begin > %postf%
  echo. gmt math -Q %%BATCH_NJOBS%% LOG10 CEIL = %CURRENTDIR%\precs >> %postf%
  echo. set /P prec= ^< %CURRENTDIR%\precs >> %postf%
  echo. ffmpeg -loglevel warning -f image2 -framerate 24 -y -i %%BATCH_PREFIX%%_%%%%0%%prec%%d.png -vcodec libx264 -pix_fmt yuv420p %%BATCH_PREFIX%%_%SIZE%.mp4 >> %postf%
  echo. rd /q %%BATCH_PREFIX%%_*.png precs times.txt >> %postf%
echo gmt end >> %postf%

REM 4. Run the batch, requesting a fade in/out via white
gmt batch %main% -Sb%pre% -Sf%postf% -T%CURRENTDIR%\times.txt -Nanim12 -V -W%CURRENTDIR%\temp -Zs
