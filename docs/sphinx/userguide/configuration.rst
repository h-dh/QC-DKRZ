.. _configuration:

===============
 Configuration
===============



QA Configuration
================

The QA would run basically on the command-line only with the specification of
the target(s) to be checked. However, using options facilitates a check in
particular for a given project.
Configuration options could be supplied on the command-line and by one or more
conf-files. Configuration options follow a specific syntax with case-insensitive
option names (e.g. ``KEY-WORD`` and ``kEy-WorD``).

.. code-block:: bash

  KEY-WORD                             enable key-word; equivalent to key-word=t
  KEY-WORD   =     value[,value ...]   assign key-word a [comma-separated] value; overwrite
  KEY-WORD   +=    value[,value ...]   same, but appended


Partitioning of data
--------------------

The specification of a path to a directory tree by option ``PROJECT_DATA`` results
in a check of every NetCDF files found within the entire tree. This can be further
customised by the key-words ``SELECT`` and ``LOCK``, which follow special rules.

.. code-block:: bash

  KEY-WORD                            var1[,var2,...]   specified variables for every path;
                                                        += mode for sub-paths
  KEY-WORD      path1[,path2,...] [=]                   all variables within the specified path;
                                                        += mode for sub-paths
  KEY-WORD      path1[,path2,...]  =  [var1[,var2,...]  specified variables within the given paths;
                                                        the  = char in-between is mandatory;
                                                        += mode for sub-paths
  KEY-WORD  =   path1[,path2,...]  =  [var1[,var2,...]  same, but overwrite mode
  KEY-WORD  +=  path1[,path2,...]  =  [var1[,var2,...]  append to a previous assignment

- Highest precedence is for options on the command-line.

- If path has no leading '/', then the search is relative to the path specified
  by option PROJECT_DATA.

- Special for options ``-S arg`` or an appended plain string on the command-line:
  cancellation of previous SELECTions in any configuration file.

- If SELECTions are specified on the command-line (options -S) with an absolute
  path, i.e. beginning with '/', then PROJECT_DATA specified in any other
  config-files is cancelled..

- All selections refer to the atomic data set of a given variable, i.e. all
  sub-atomic files; even if a file name is appended to a path.

- Locking gets the higher precedence over selection.

- Path and variable indicators are separated by the '=' character, which may be
  omitted when there is no variable (except the case that each of the paths
  contains no '/' character).

- Regular expressions may be applied for both path(s) and variable(s).

- If an expanded path points to a sub-dir tree, then this is searched for the
  variables.

- A variable is selected if the expanded variable part fits the beginning of the
  name, e.g. specifying 'tas' would select all tas, tasmax, and tasmin files.
  Note that every file name begins with ``variable_...`` for CMIP5/CORDEX, thus,
  use ``tas_`` for this alone.


Configuration files
===================

A description of the configuration options is given in the repository.


.. code-block:: bash

    QC-0.4:
    /package-path/tables/SVN_defaults/"project-name"_qc.conf


.. code-block:: bash

    QA-DKRZ:
    /package-path/tables/projects/"project-name"/"project-name"_qa.conf

Configuration files and options may be specified multiply following a given
precedence. This facilitates to have a file with short-term options (in a
file attached to the -f option on the command-line), another one with settings
to site-specific demands, which are robust against changes in the repository,
and long-term default settings from the repository. All options may be specified
on the command-line plus some more (
``/package-path/QA-DKRZ/scripts/qa_DKRZ --help``).
A sequence of configuration files is accomplished by ``QA_CONF=conf-file``
assignments embedded in the configuration files (nesting depth is unrestricted).
The precedence of configuration files/options is given below from highest to
lowest:

-  directly on the command-line
-  in the task-file (``-f file``) specified on the command-line.
-  QA_CONF assignments embedded (descending starts from the ``-f file``).
-  site-specific files provided by files located straight in ``/package-path/tables``.
-  defaults for the entire project:

   .. code-block:: bash

       QC-0.4:
       /package-path/tables/SVN_defaults

   .. code-block:: bash

       QA-DKRZ:
       /package-path/tables/projects/project-name
