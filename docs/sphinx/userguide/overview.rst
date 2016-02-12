.. _overview:

========
Overview
========

What is checked?
================

The Quality Assurance (QA) tool developed at DKRZ tests the conformance
of meta-data of climate simulations given in `NetCDF` format to conventions and
rules of projects. Additionally, the QA checks time sequences of data
and the technical state of data (i.e. occurrences of `Inf` or `NaN`, gaps,
replicated sub-sets, etc.) for atomic data set entities, i.e. variable and frequency, like `tas` and `mon`
for monthly means of `Near-Surface Air Temperature`. When atomic data sets
are subdivided into several files, then changes between these files in
terms of (auxiliary) coordinate variables will be detected as well as gaps or
overlapping time ranges. This may also apply to follow-up experiments.

At present, the QA checks data of the projects `CMIP5` and `CORDEX` by consulting
tables based on a controlled vocabularies and requirements provided by the
projects. When such pre-defined information is given about directory structures,
format of file names, variables and attributes, geographical domain, etc.,
then any deviation from compliance will be annotated in the QA results.


Available Versions
==================

The tool is available in two versions.

.. note:: The name of the tool has changed over the years as well as the name of the starting script. At first, the acronym QC (Quality Control/Check) was used, but was changed to the more commonly used term QA (Quality Assurance). The names qcManager, qc_DKRZ, and qa_DKRZ start an identical script (also, the underscore may be replaced by a hyphen). Option names containing QC or qc are still usable for backward compatibility.


QA-DKRZ (current >=0.5 beta stage)
----------------------------------

Development stage: the software resides on GitHub and is accessible by:

.. code-block:: bash

   $ git clone  https://github.com/IS-ENES-Data/QA-DKRZ

The QA runs are started on the command-line by:

.. code-block:: bash

   $ /package-path/scripts/qa_DKRZ [opts]

While former versions took for granted that the NetCDF Climate and Forecast (CF) Meta Data Conventions were held when specified, 
the conformance is now tested. The CF check is both embedded in the QA tool itself and provided by a stand-alone tool 
including a test suite of files.


QC-0.4 (stable)
---------------

The purpose of the tool is to operate checks of CMIP5 and CORDEX data.  
This QA software package is maintained with the version control tool ``subversion``. 
The installation is done by script. Please download the `install script`_.

This script is also located in the QC-0.4 package. Note that depending on the browser, the script could be
saved as ``install.txt`` and perhaps without execute permission. If so, run ``bash install.txt`` (or ``chmod u+x install``).  

After installation and configuration, QA checks are started on the command-line by:

.. code-block:: bash

  $ /package-path/scripts/qcManager [opts]

.. _`install script`:  http://svn-mad.zmaw.de/svn/mad/Model/QualCheck/QC/branches/QC-0.4/install




