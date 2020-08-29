.. _symbols_gallery:

Users Symbol Contributions
==========================

Biology symbols
---------------

These are symbols used to illustrate sightings of whales.

.. toctree::
   :maxdepth: 1

   users-contrib-symbols/biology/Cetacea.rst

.. cssclass:: gmtgallary

.. jinja::

    {% set symbols=[
        "atlantwhitesided",
        "atlantwhitesided_high",
        "atlantwhitesided_low",
        "beluga",
        "beluga_high",
        "beluga_low",
        "bottlenose",
        "bottlenose_high",
        "bottlenose_low",
        "bowhead",
        "bowhead_high",
        "bowhead_low",
        "burmeistersporpoise",
        "burmeistersporpoise_high",
        "burmeistersporpoise_low",
        "commondolphin",
        "commondolphin_high",
        "commondolphin_low",
        "commondolphin_midhigh",
        "commondolphin_midlow",
        "commonporpoise",
        "commonporpoise_high",
        "commonporpoise_low",
        "cuviersbeaked",
        "cuviersbeaked_high",
        "cuviersbeaked_low",
        "finwhale",
        "finwhale_high",
        "finwhale_low",
        "graywhale",
        "graywhale_high",
        "graywhale_low",
        "humpbacktail_one",
        "humpbacktail_one_low",
        "humpbacktail_two",
        "humpbacktail_two_low",
        "jumpback",
        "jumpback_high",
        "jumpback_low",
        "killerwhale",
        "killerwhale_high",
        "killerwhale_low",
        "longfinnedpilotwhale",
        "longfinnedpilotwhale_high",
        "longfinnedpilotwhale_low",
        "minkewhale",
        "minkewhale_high",
        "minkewhale_low",
        "northernrightwhale",
        "northernrightwhale_high",
        "northernrightwhale_low",
        "pigmyspermwhale",
        "pigmyspermwhale_high",
        "pigmyspermwhale_low",
        "rissosdolphin",
        "rissosdolphin_high",
        "rissosdolphin_low",
        "seiwhale",
        "seiwhale_high",
        "seiwhale_low",
        "shortfinnedpilotwhale",
        "shortfinnedpilotwhale_high",
        "shortfinnedpilotwhale_low",
        "southernrightwhale",
        "southernrightwhale_high",
        "southernrightwhale_low",
        "spectacledporpoise",
        "spectacledporpoise_high",
        "spectacledporpoise_low",
        "spermwhale",
        "spermwhale_high",
        "spermwhale_low",
        "spermwhaletail",
        "spermwhaletail_high",
        "spermwhaletail_low",
        "srightwhaledolphin",
        "srightwhaledolphin_high",
        "srightwhaledolphin_low",
        "stripeddolphin",
        "stripeddolphin_high",
        "stripeddolphin_low",
        "unidentifiedbeakedwhale",
        "unidentifiedbeakedwhale_high",
        "unidentifiedbeakedwhale_low",
        "unidentifieddolphin",
        "unidentifieddolphin_high",
        "unidentifieddolphin_low",
        "unidentifiedwhale",
        "unidentifiedwhale_high",
        "unidentifiedwhale_low"
        ]
    %}
    {% for i in symbols %}
    -  .. figure:: /users-contrib-symbols/biology/images/{{i}}.*
          :target: ./users-contrib-symbols/biology/{{i}}.html

          :ref:`{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   users-contrib-symbols/biology/*

Structural geology symbols
--------------------------

.. toctree::
   :maxdepth: 1

   users-contrib-symbols/geology/Geology.rst

These are symbols used in structural geology.

.. cssclass:: gmtgallary

.. jinja::

    {% set symbols=[
        "geo-cleavage",
        "geo-cleavage_hor",
        "geo-cleavage_vert",
        "geo-foliation-2",
        "geo-foliation-3",
        "geo-foliation",
        "geo-foliation_hor",
        "geo-foliation_vert",
        "geo-joint",
        "geo-joint_hor",
        "geo-joint_vert",
        "geo-lineation-2",
        "geo-lineation-3",
        "geo-lineation",
        "geo-lineation_hor",
        "geo-lineation_vert",
        "geo-plane",
        "geo-plane_gentle",
        "geo-plane_hor",
        "geo-plane_inv",
        "geo-plane_medium",
        "geo-plane_rake",
        "geo-plane_steep",
        "geo-plane_und",
        "geo-plane_vert"
        ]
    %}
    {% for i in symbols %}
    -  .. figure:: /users-contrib-symbols/geology/images/{{i}}.*
          :target: ./users-contrib-symbols/geology/{{i}}.html

          :ref:`{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   users-contrib-symbols/geology/*

Miscellaneous symbols
---------------------

These are general-purpose symbols you may find useful.

.. cssclass:: gmtgallary

.. jinja::

    {% set symbols=['pirata'] %}
    {% for i in symbols %}
    -  .. figure:: /users-contrib-symbols/misc/images/{{i}}.*
          :target: ./users-contrib-symbols/misc/{{i}}.html

          :ref:`{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   users-contrib-symbols/misc/*
