.. _guide-intro:

============
Introduction
============

DKRZ Quality Assurance Tool
===========================

The Quality Assurance (QA) software developed at the DKRZ tests the conformance
of meta-data of climate simulations given in NetCDF format to conventions and
rules of projects. Additionally, the QA checks time sequences of data
and the technical state of data (i.e. occurrences of Inf or NaN, gaps,
replicated sub-sets, etc.)
for atomic data set entities, i.e. variable and frequency, like 'tas' and 'mon'
for monthly means of 'Near-Surface Air Temperature'. When atomic data sets
are subdivided into several files, then changes between these files in
terms of (auxiliary) coordinate variables will be detected as well as gaps or
overlapping time ranges. This may also apply to follow-up experiments.

At present, the QA checks data of the projects CMIP5 and CORDEX by consulting
tables based on a controlled vocabularies and requirements provided by the
projects. When such pre-defined information is given about directory structures,
format of file names, variables and attributes, geographical domain, etc.,
then any deviation from compliance will be annotated in the QA results.

Please, direct questions or comments to hollweg@dkrz.de


Versions
========

The tool is distributed in two versions:

- **QC-0.4 (stable)**

  Purpose is to operate checks of CMIP5 and CORDEX data.
  This QA software package is maintained with the version control tool
  ``subversion``. The installation is done by script. Please, click
  `install <http://svn-mad.zmaw.de/svn/mad/Model/QualCheck/QC/branches/QC-0.4/install>`_
  to download the script, which is also located in the QC-0.4 package. Note that
  depending on the browser, the script could be saved as ``install.txt`` and
  perhaps without execute permission. If so, run ``bash install.txt`` (or
  ``chmod u+x install``).
  After installation and configuration, QA checks are started on the command-line
  by:

  .. code-block:: bash

    /package-path/scripts/qcManager [opts]

- **QA-DKRZ (v.0.5-beta)**

  Development stage: the software resides on GitHub and is accessible by

  .. code-block:: bash

    git clone https://github.com/h-dh/QA-DKRZ

  The QA runs are started on the command-line by:

  .. code-block:: bash

    /package-path/scripts/qa_DKRZ [opts]

  While former versions took for granted that the NetCDF Climate and
  Forecast (CF) Meta Data Conventions were held when specified, the
  conformance is now tested. The CF check is both embedded in the QA tool
  itself and provided by a stand-alone tool including a test suite of files.

  - **CF Checker**

    The CF Conformance checker applies to conventions 1.4 - 1.7draft. Note
    that it takes a few seconds for the installation when it runs for the
    first time.

    Command-line for the stand-alone tool (generate executable by ``./install CF``):

    .. code-block:: bash

      /package-path/scripts/dkrz-cf-checker [opts] [path/]files

      -C str        CF conventions string; taken from global attributes by default.
      -F path       Find all NetCDF files in the sub-dir tree starting at path.
      -p str        Path to one or more NetCDF file; this is applied only to files.
      -R            Apply also recommendations given in the CF conventions.
      -x str        Path to QA-DKRZ/bin; required if the script was copied to the
                    outside of the package (note that a symbolic link works without
                    this option).
      --ts          Run the CF Test Suite located within the package (file names
                    may be given additionally).

  - **CF Test Suite**

    A collection of NetCDF files designed to cover all rules of the CF Conventions
    derived from the examples of the cf-conventions-1.x.pdf documents.
    There are two branches:

    PASS
      Proper meta-data and data sets, which should never output a failure. Number of files: 81

    FAIL
      Each file contains a single break of a rule and should never pass the test. Number of files: 187

Note that the name of the tool has changed over the years as well as the name of the starting script. At first, the acronym QC (Quality Control/Check) was used, but was changed to the more commonly used term QA (Quality Assurance). The names qcManager, qc_DKRZ, and qa_DKRZ start an identical script (also, the underscore may be replaced by a hyphen). Option names containing QC or qc are still usable for backward compatibility.


