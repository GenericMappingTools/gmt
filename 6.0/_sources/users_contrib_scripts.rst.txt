.. _users_gallery:

Users Script Contributions
--------------------------

.. cssclass:: gmtgallary

.. jinja::

    {% set scripts=['vertical_slice'] %}
    {% for i in scripts %}
    -  .. figure:: /users_contrib_scripts/images/{{i}}.*
          :target: ./users_contrib_scripts/{{i}}.html

          :ref:`{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   users_contrib_scripts/*
