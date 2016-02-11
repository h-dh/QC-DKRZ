.. _installation:

============
Installation
============

You can install `QA-DKRZ` either via the conda package manager or from source.


.. _conda-install:

Installing with Conda Package Manager
=====================================

Make sure that you have `conda <http://conda.pydata.org/docs/install/quick.html#linux-miniconda-install>`_ installed. The quick steps to install `miniconda` on Linux 64-bit are:

.. code-block:: bash

   $ wget https://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh
   $ bash Miniconda-latest-Linux-x86_64.sh

.. note:: Currently we only support the Linux 64-bit version of the QA tool on conda. The installation is tested on Centos 6 and Ubuntu/Debian 14.04 LTS.

Please check that the ``conda`` package manager is on your ``PATH``. For example you may set the ``PATH`` environment variable as following:

.. code-block:: bash

    $ export PATH=~/miniconda/bin:$PATH
    $ conda -h

The QA tool package is on the `birdhouse anaconda channel <https://anaconda.org/birdhouse/qa-dkrz>`_.
To install the QA tool just run the following command (all dependencies included):

.. code-block:: bash

   $ conda install -c http://conda.anaconda.org/birdhouse qa-dkrz


Check the installation by running the CF-Checker with a NetCDF file:

.. code-block:: bash

   $ dkrz-cf-checker -h
   $ dkrz-cf-checker my_tasmax.nc


Installing from Source
======================

Requirements
------------

The tool requires the BASH shell and a C/C++ compiler.

Building the QA tool
--------------------

A script ``install`` is provided to manage different installation/update modes.
``install`` runs on linux utilising Bash and C/C++ (AIX works, too).

Environmental variables CC, CXX, CFLAGS, and CXXFLAGS are accepted.

``install`` establishes access to libraries, which may be linked or built, as well
as updating sources and executables.

A file ``install_configure`` is installed during the first run of install.

If option ``--build`` is not applied, then the user is notified to edit ``install_configure``.

After the installation, compiler settings and paths to libraries are always
read from ``install_configure``.

Proceedings of installation/update are logged in file ``install.log``.

The full set of options is described by:

.. code-block:: bash

  $ ./install --help

Compile the executable for the project CORDEX: 

.. code-block:: bash

   $ ./install CORDEX

The following projects are supported: CORDEX (by default), CMIP5, CF, NONE.

A test-run is started automatically generating some results in the directory ``/package-path/example/test_I``.

Building libraries
------------------

.. code-block:: bash

  $ ./install --build [opts]

This downloads and installs the following libraries:

- zlib-1.2.8 from www.zlib.net,
- hdf5-1.8.9 from www.hdfgroup.org,
- netcdf-4.3.0 from www.unidata.ucar.edu (shared, no FORTRAN, non-parallel),
- udunits package from http://www.unidata.ucar.edu/packages/udunits (not for QC-0.4).

The libraries are built in sub-directory ``local/source``.
If libraries had been built previously, then the sources are updated and
the libraries are rebuilt.


Update you installation
------------------------

Updating the QA sources from the repository and re-compilation of executables is done
easiest by using the ``install`` script. There are two modes: automatic and manually.
Please note that the execution of ``/package-path/install [project]`` does
not call for any updates by default; this will only recompile locally changed
C/C++ programs.

**Manual Update**:

.. code-block:: bash

  $ /package-path/install --up[date] [opts]

This applies any changes in the repository. If C/C++ programs are affected,
then executables are remade. Please note that libraries are not updated.
If you want to do so, then you have to set option ``--build``.

**Automatic Update**:

.. code-block:: bash

  $ /package-path/install --auto-up [opts]

- Once ``--auto-up`` was set, the package will always be synchronised to the
  repository at the beginning of each QA session.
- This mode may be disabled by option ``--auto-up=disable``.
- Enabling/disabling the auto-mode works also during operational runs of the
  qa-DKRZ script.
- Daily search for updates of the required tables from
  http://www.cfconventions.org (done off-line for QC-0.4, which applies the standard-name table.).

  - ``area-type-table.xml``
  - ``cf-standard-name-table.xml``
  - ``standardized-region-names.html``
