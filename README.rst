=================================================
QA-DKRZ - Quality Assurance Tool for Climate Data
=================================================

|build-status| |conda-install|

:Version: 0.5.x

The QA tool is a quality assurance checker for metadata in climate data (NetCDF files). The tool checks
the `CF-Conventions`_ conformance (Climate and Forecast Conventions) and also projects conventions for CMIP5 and CORDEX.

.. note:: The QA tool is still in a testing stage.

.. _`CF-conventions`: http://cfconventions.org/

Getting Started
===============

The easiest way to install the QA tool is to use the conda package manager:

.. code-block:: bash

   $ conda install -c birdhouse qa-dkrz

See :ref:`installation` for details and how to install from source. Try the :ref:`examples` to get started.

Documentation
-------------

QA Tool is using Sphinx, and the latest documentation can be found on `ReadTheDocs`_:

    http://qa-dkrz.readthedocs.org

.. _`ReadTheDocs`:  http://qa-dkrz.readthedocs.org

Getting Help
============

Please, direct questions or comments to hollweg@dkrz.de

Mailing list
------------

Join the mailing list ...


Bug tracker
===========

If you have any suggestions, bug reports or annoyances please report them
to our issue tracker at https://github.com/IS-ENES-Data/QA-DKRZ/issues

Contributing
============

Development of `QA DKRZ` happens at Github: https://github.com/IS-ENES-Data/QA-DKRZ

You are highly encouraged to participate in the development.

License
=======

Please see the *Disclaimer of Warranty* (`DoW.txt`) file in the top distribution directory.

.. |build-status| image:: https://travis-ci.org/IS-ENES-Data/QA-DKRZ.svg?branch=master
   :target: https://travis-ci.org/IS-ENES-Data/QA-DKRZ
.. |conda-install| image:: https://anaconda.org/birdhouse/qa-dkrz/badges/installer/conda.svg
   :target: https://anaconda.org/birdhouse/qa-dkrz
