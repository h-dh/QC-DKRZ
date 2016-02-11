.. _tools:

===============
Available Tools
===============

CF Checker
==========

The CF Conformance checker applies to conventions 1.4 -1.7draft. 

.. note:: The cf-checkers takes a few seconds for the installation when it runs for the first time.

See :ref:`installation` on how to install this tool. See the available options:

.. code-block:: bash

    $ dkrz-cf-checker -h

    Usage: cf-checker [opts] netCDF-file[s]
    Purpose: Check for CF Conventions Compliance
    (http://cfconventions.org).
    The checker is part of the QA-DKRZ package and must have been compiled
    by '/your-path-to-QA-DKRZ/install CF'.
      -C str    CF Convention string; taken from global attributes by default.
      -F path   Find recursively all nc-files from starting point 'path'.
      -p str    Path to one or more netCDF Files; this is prefixed
                only to netCDF-files without any path component.
      -R        Apply also recommendations given by CF conventions.
      --debug   Show execution commands.
      --help
      --param   Only for program development.
      --ts      Run the files provided in the Test-Suite for CF conventions
                rules in QA-DKRZ/CF_TestSuite. If particular netCDF files are
                provided additionally, then only these are used.


CF Test Suite
-------------

A collection of NetCDF files designed to cover all rules of the CF Conventions
derived from the examples of the cf-conventions-1.x.pdf documents. There are two branches:

   PASS
      Proper meta-data and data sets, which should never output a failure. Number of files: 81

   FAIL
      Each file contains a single break of a rule and should never pass the test. Number of files: 187

