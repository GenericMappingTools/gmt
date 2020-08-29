GMT symbols for Whale-watchers and marine biologists
====================================================

What's this?
------------

This is a collection of custom symbols for the Generic Mapping Tools (GMT) software that may
be of interest mainly to whale-watchers or whale researchers. With this, you can show your 
data in a map created with GMT as fine and detailed marine mammal's icons instead the 
usual circles or squares. GMT is a very complete GIS software so you could add also 
bathymetric and coastal lines, the locations of the main cities and ports in your area, 
or other useful data like plankton concentration or water temperatures in your map.
 
The collection currently comprises symbols for 8 species of Baleen Whales, 16 of Toothed Whales, 
2 species of seals and 4 more for "unidentified" seals, beaked whales, dolphins or whales. 
Several versions (low/normal/high) of most symbols are also available, and you can 
change easily between color or gray symbols in 57 of the 90 symbols provided, therefore
you can choose really between more than 150 different symbols.  

Author of the  symbols: Pablo Valdés <marine.biol@arsystel.com> (Spanish preferred, but you can use also
English or French). In the future I hope to create gradually more symbols for other species. 
I greatly appreciate your suggestions and feedback.

These symbols are freely available under a GNU Library General Public Licence (version 2, 
or later).

 
How to use the symbols?
-----------------------

Before to start, think in the type of map you want to obtain and prepare your data. 
If you want to create a 2D map (most common situation) you need :doc:`plot </plot>`, if you want a 3D
map you should use :doc:`plot3d </plot3d>` instead. Think also in how many different symbols you want to 
show in each individual map. You should have at least a different .xy file for each
species that you want to show. You could want also to show separately males and females 
or adults/youngs/calfs. In this case, you will either need to give a different size to each 
group or place the data for each group in a different .xy (or .xyz) file and provide different symbols. 
 
A valid input datafile.xy is simply a text file containing several lines like this:

.. code-block:: none

    #common dolphin data, file: cdolphin.xy
    #longitude latitude (symbolsize) (symboltype) (...others)
    -5:40:44   30:30:01  0.10	 k
    6:45.70   -43       0.12	 k
    355.707   42.7543   0.17	 k
    ...

plot can deal with several lon/lat formats. All of this are accepted.
The symbolsize field is optional, but if not provided you must specify a common size 
(used for all observations in this file) in the script with -Sksymbolname/simbolsize  

The symboltype field is also optional, k means custom symbol. Currently I can't pass 
different symbols to :doc:`plot </plot>` in the same file (I need split first the file) 
and must provide a unique symbolname in the script for all lines, so the interest 
of having this field is reduced. Probably I'm missing something. You can also add other fields
like font, angle, position or a last field with a short remembering note in a text line, they
are ignored by :doc:`plot </plot>` but you could pass it to :doc:`text </text>`.

Take a look at the pics (.png) and choose the symbols that you want. No installation needed, 
simply browse the directory symbols and copy the files with extension .def having the same 
name as the desired pic to your working directory. This is the list of symbols available:

**English name (scientific name) - symbols available**

Cetacea
  Toothed Whales, SubO. Odontoceti
     #. Common dolphin (Delphinus delphis) 
        - ddelphis_low.def  
        - ddelphis_midlow.def
        - ddelphis.def
        - ddelphis_midhigh.def
        - ddelphis_high.def

     #. Stripped dolphin (Stenella coeruleoalba) 
        - stripped_low.def, stripped.def, stripped_high.def

     #. Bottlenose dolphin (Tursiops truncatus) 
        - bottlenose_low.def, bottlenose.def, bottlenose_high.def

     #. Atlantic White-sided dolphin (Lagenorhynchus acutus) 
        - atlanticwhitesided_low.def, atlanticwhitesided.def, atlanticwhitesided_high.def

     #. Killer whale (Orcinus orca) 
        - killerwhale_low.def, killerwhale.def, killerwhale_high.def

     #. Risso's dolphin (Grampus griseus) 
        - rissosdolphin_low.def, rissosdolphin.def, rissosdolphin_high.def

     #. Short-Finned Pilot whale (Globicephala macrorhynchus) 
        - shortfinnnedpilotwhale_low.def,  shortfinnnedpilotwhale.def
        - shortfinnnedpilotwhale_low.def

     #. Long-Finned Pilot whale (Globicephala melaena) 
        - longfinnedpilotwhale_low.def, longfinnedpilotwhale.def
        - longfinnedpilotwhale_low.def

     #. Southern Rightwhale Dolphin (Lagenodelphis peronii)
	      - srightwhaledolphin_low.def, srightwhaledolphin.def
	      - srightwhaledolphin_high.def

     #. Common porpoise (Phocoena phocoena) 
        - commonporpoise_low.def, commonporpoise.def, commonporpoise_high.def

     #.	Burmeister's porpoise (Phocoena spinipinnis)
        - burmeistersporpoise_low.def, burmeistersporpoise.def, burmeistersporpoise_high.def

     #. Spectacled porpoise (Australophocaena dioptrica)
        - spectacledporpoise_low.def, spectacledporpoise.def, spectacledporpoise_high.def
	
     #. Beluga (Delphinaterus leucas) 
        - beluga_low.def,  beluga.def,  beluga_high.def

     #. Cuvier's beaked whale (Ziphius cavirostris) 
        - cuviersbeaked_low.def,  cuviersbeaked.def, cuviersbeaked_high.def

     #. Unidentified beaked whale (Mesoplodon spp.) 
        - unidentifiedbeakedwhale_low.def, unidentifiedbeakedwhale.def, unidentifiedbeakedwhale_high.def

     #. Sperm whale (Physeter macrocephalus) 
        - spermwhale_low.def, spermwhale.def, spermwhale_high.def
        - spermwhaletail_low.def, spermwhaletail.def, spermwhaletail_high.def

     #. Pygmy sperm whale (Kogia breviceps)
        - pigmyspermwhale_low.def, pigmyspermwhale.def, pigmyspermwhale_high.def

     #. A dolphin (gen. unknown)
       	- unidentifieddolphin_low.def, unidentifieddolphin.def, unidentifieddolphin_high.def

  Baleen Whales, SubO. Misticeti:
     #. Minke whale (Balaenoptera acutorostrata)
     	  - minkewhale.def, minkewhale_low.def, minkewhale_high.def

     #. Fin Whale (Balaenoptera physalus)
        - finwhale.def,  finwhale_low.def,  finwhale_high.def

     #.	Sei Whale (Balaenoptera borealis)
        - seiwhale_low.def,  seiwhale.def,  seiwhale_high.def

     #. Humpback Whale (Megaptera novaeangliae)
       	- humpbacktail_one_low.def, humpbacktail_one.def
        - humpbacktail_two_low.def, humpbacktail_two.def
        - jumpback_low.def (yes, with j, look at the pic ;-))
        - jumpback.def, jumpback_high.def

     #. Gray Whale (Eschrichtius robustus)
        - graywhale_low.def,  graywhale.def,  graywhale_high.def

     #.	Right Whales (Eubalaena glacialis, Eubalaena australis)
        - southernrightwhale_low.def, southernrightwhale.def, southernrightwhale_high.def
        - northernrightwhale_low.def, northernrightwhale.def, northernrightwhale_high.def

     #. A whale (unknown species)
       	- unidentifiedwhale_low.def, unidentifiedwhale.def, unidentifiedwhale_high.def


3: Call them including the corresponding :doc:`plot </plot>` or :doc:`plot3d </plot3d>` lines in a GMT script like this: 

.. code-block:: none

    #!/usr/bin/env bash
    gmt coast -JM20c -R-10/6/33/36 -K -W0.5pt/0 -P -Gblack > myfile.ps
    gmt plot cdolphin.xy -Skcommondolphin/ -JM20c -R-10/6/33/36 -P -K -O >> myfile.ps
    gmt plot bottlenose_dolphin.xy -Skbottlenose_high/0.5 -K -O ...etc >> myfile.ps
    gmt plot killerwhale_data.xy   -Skkillerwhale_low/0.5 -O ...etc >> myfile.ps

In our examples we will place all the .xy and .def files in our working directory, 
but you can find more convenient to move them to several subdirectories named, for
instance, data and symbols:

   ::

    gmt plot data/killer_whale.xy -Sksymbols/Cetacea/killerwhale/0.5 -O ...etc >> myfile.ps

In this case, please read also the points 1.2-1.3 of the file FAQ.txt
  
4: Run the bash script, print/open the output postscript file myfile.ps, or convert to PDF or rasters with gmt psconvert.


FAQ and Troubleshoting
----------------------

The symbols are not drawn
~~~~~~~~~~~~~~~~~~~~~~~~~

#. When In run the script I obtain GMT ERROR: plot:

Could not find custom symbol symbolname.def!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Probably you had wrote something like: -Sksymbolname.def/0.5 
    Please note that this is incorrect, you should remove .def and use 
    -Sksymbolname/0.5 instead in your script.

Could not find custom symbol mydirsymbolname!, Cannot open file mydirmyfile.xy!
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Probably you had placed your symbols in a subdirectory and are using the wrong 
    directory notation for your operative system. Please note that Linux use "/" 
    for directories whereas Microsoft Windows use "\", so check if you had wrote 
    something like -Skmydir\killerwhale when you want to say -Skmydir/killerwhale 
    or mydir\killerwhale.xy instead mydir/killerwhale.xy
    This error could also occur if you try to run a script created in Windows 
    in a Linux OS or vice versa. Try to intercange / and \ and run again the script.


Symbol customization
~~~~~~~~~~~~~~~~~~~~

The symbols are to much big!, What size should I use?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   In your script you'll need to reduce the normal size of the symbols. A range 
   of sizes between 0.12 and 0.18, (rarely more than 0.2) should be OK. Some symbols 
   are a little bigger than others, so play with the size in the script until you 
   obtain the right for you. Remember that you can easily modify the size of the 
   symbol directly in your GMT script (-Skoorca/0.8  -Skoorca/0.2) or in your file xy.  
   I recommend to use different sizes for males, females and calfs.  

I don't want color symbols!
^^^^^^^^^^^^^^^^^^^^^^^^^^^

   You can easily obtain the same symbol in graytones editing the def file with your 
   favourite text editor. Follow the instructions you will find inside the .def files. 
   Some symbols like the killerwhale have only a b/w version for obvious reasons.

How can I change the colour of the symbols?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   The colour of each area is specified inside the def file, so you can't 
   simply specify a colour directly in your GMT script or you will obtain 
   strange results. You should open and edit the -W and -G in the def file.  

After editing the def file I obtain strange polygonal patches instead the desired symbol but all points 
are the same than in the original .def!

   Check that you don't have deleted the pt specification in a line with -W. 
   This (-W100) is erroneous, (width line 100 pt) while this (-W025.pt/100) is ok. 

Other questions
~~~~~~~~~~~~~~~

Why they are so may similar symbols low, high, etc... for the same species?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Sometimes some symbols will overlap severely with their neighbors. Specially with 
    the most common species like Delphinus dolphins. I think that this looks ugly, so you 
    will obtain a nicer map if you use a little more tall or short symbol for these 
    specific animals. Try with the different versions of the same symbol until you 
    obtain a satisfactory presentation. Remember that you must place this problematic 
    specimens in a different xy file first.

    The representation of multiple strandings or sightings in the same point can be also 
    problematic and sometimes you will need to obtain more complex symbols to show a multiple 
    and heterogeneous stranding, for instance a mother/calf stranding or two different species 
    sighted in exactly the same point. You can deal with those cases if you stack several 
    low/high symbols until you obtain the complex symbol desired. You will need duplicate 
    or triplicate the :doc:`plot </plot>` lines in the script and perhaps play also with the size and color 
    of the symbols. For instance if you see a killer whale harassing two dolphins and you want
    to show all in the same map:

       plot a_killer_whale_data.xy             -Skkillerwhale_high/0.8 ...  etc 
       plot a_common_dolphin_mother_data.xy    -Skcommondolphin_midlow/0.7 ... etc
       plot and_its_calf_data.xy               -Skcommondolphin_low/0.3 ... etc

    For a better result place the lines calling the taller symbols first and the shorter 
    symbols at the end. 


Where fall the real coordinates in the figure?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    In the small black circle below the symbol. I think is more precise this way 
    so probably most if not all symbols in the future will have a line and a small 
    circle in the 0,0 point. 
