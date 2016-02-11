.. _tools:

===============
Available Tools
===============

CF Checker
==========

The CF Conformance checker applies to conventions 1.4 -1.7draft. 

.. note:: The cf-checkers takes a few seconds for the installation when it runs for the first time.

Command-line for the stand-alone tool (generate executable by ``./install CF``):

.. code-block:: bash

    $ /package-path/scripts/dkrz-cf-checker [opts] [path/]files

      -C str        CF conventions string; taken from global attributes by default.
      -F path       Find all NetCDF files in the sub-dir tree starting at path.
      -p str        Path to one or more NetCDF file; this is applied only to files.
      -R            Apply also recommendations given in the CF conventions.
      -x str        Path to QA-DKRZ/bin; required if the script was copied to the
                    outside of the package (note that a symbolic link works without
                    this option).
      --ts          Run the CF Test Suite located within the package (file names
                    may be given additionally).

CF Test Suite
-------------

A collection of NetCDF files designed to cover all rules of the CF Conventions
derived from the examples of the cf-conventions-1.x.pdf documents. There are two branches:

   PASS
      Proper meta-data and data sets, which should never output a failure. Number of files: 81

   FAIL
      Each file contains a single break of a rule and should never pass the test. Number of files: 187

