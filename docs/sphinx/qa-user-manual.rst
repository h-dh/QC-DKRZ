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


Requirements
============

The tool requires the BASH shell and a C/C++ compiler.


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

    git clone https://github.com/IS-ENES-Data/QA-DKRZ

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


Installation
============

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

The full set of options is described by ``./install --help``.

| Compilation of executables: ``./install project_name``
| Supported projects: CORDEX (by default), CMIP5, CF, NONE.

A test-run is started automatically generating some results in the directory
``/package-path/example/test_I``.

Building libraries
------------------
.. code-block:: bash

  Command-line: ./install --build [opts]

Download and install libraries

- zlib-1.2.8 from www.zlib.net,

- hdf5-1.8.9 from www.hdfgroup.org,

- netcdf-4.3.0 from www.unidata.ucar.edu (shared, no FORTRAN, non-parallel),

- udunits package from http://www.unidata.ucar.edu/packages/udunits (not for QC-0.4).

The libraries are built in sub-directory ``local/source``.
If libraries had been built previously, then the sources are updated and
the libraries are rebuilt.


Update
======

Updating the QA sources from the repository and re-compilation of executables is done
easiest by using the ``install`` script. There are two modes: automatic and manually.
Please note that the execution of ``/package-path/install [project]`` does
not call for any updates by default; this will only recompile locally changed
C/C++ programs.

Manual Update
-------------

.. code-block:: bash

  Command-line: /package-path/install --up[date] [opts]

This applies any changes in the repository. If C/C++ programs are affected,
then executables are remade. Please note that libraries are not updated.
If you want to do so, then you have to set option ``--build``.

Automatic Update
----------------

.. code-block:: bash

  Command-line: /package-path/install --auto-up [opts]

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
lowest.

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


Running the QA
==============

A QA session is launched on the command-line or in the back-ground.

.. code-block:: bash

  /package-path/scripts/qa-DKRZ [-m] [-f task.file] [opts]

Configuration options are provided on the command-line with the prefix ``-E_`` or ``-e_``.

If a session is resumed, then the QA runs only over the part which had not
been checked previously or which is indicated for a clearance of previous check
results. Usually, atomic files causing annotations of a pre-defined degree of
severity are locked and will not be touched again. There are different ways
to clear results, which may be combined to a comma-separated list:

.. code-block:: bash

  E_CLEAR        All atomic variables of the current selection, i.e. redo
  E_CLEAR=note   Variables with annotations of minor severity
  E_CLEAR=lock   Variables with annotations causing a lock
  E_CLEAR=var    The acronym of a variable, e.g. 'tas'


Annotation
==========

Any failed QA test raises an annotation. In fact, testing and issuing annotations
are different processes. The way of testing is done always by the program;
the user has no influence. However, issuing annotations is controlled by the user
by means of configuration options and the check-list table. Location:

.. code-block:: bash

    QC-0.4:
    /package-path/tables/SVN_defaults/project-name_check-list.conf

.. code-block:: bash

    QA-DKRZ:
    /package-path/tables/projects/*project-name*/project-name_check-list.conf
    /package-path/tables/projects/CF/CF_check-list.conf

A check may-be discarded entirely or for specified variables. Also, specific data
values of given variables may be excluded from annotating. Details are explained
in the check-list files.


QA Results
==========

Log-files and annotations are written to the location specified by the
configuration option QA_RESULTS. Check results of data files are grouped in
directories or files based on the options beginning with the token EXP.
The purpose of such an experiment-like name is to tag a larger set of various
checks with a name that corresponds to a certain volume of netCDF files.
Please choose unique names among the respective components .

There
are several ways to define a name with the precedence of the following options:

.. code-block:: text

    EXP_NAME=string                      Explicit name
    EXP_FNAME_PATTERN=comma-sep-indices  Pattern common to file names
    EXP_PATH_INDEX=camma-sep-indices     Components of paths common to the directory structure.
                                       'all-scope' by default

*Example for an experiment-like-name*:

.. code-block:: text

    Option: EXP_PATH_INDEX=9,8,7,6,5,3
    Path:   /path(10)/AFR-44(9)/Inst(8)/driver(7)/historical(6)/EM(5)/model(4)/vers(3)/day(2)/var(1)
    Name:   AFR-44_Inst_driver_historical_EM_vers
    Note:   Index given in parantheses is only for illustration.


In general, five sub-directories are created:

.. code-block:: text
   :caption: check_logs (directory)

    - experiment-like-name.log (file)

        The files are written in the human-readable YAML format (optional for QC-0.4
        by YAML in the configuration or --yaml on the command-line; there are
        tools, which transform YAML coding into XML). Each entry of a log-file
        represents the check of a (sub-atomic) data file regardless of whether
        annotations were issued or not.

    - Annotations.yaml (directory; for QC-0.4: _Notes)

        Only log-file entries having annotations are extracted.
        Not created when there are no annotations.

    - Periods.yaml (directory; for QC-0.4: _Periods)

        CORDEX for instance, does not prescribe the periods of atomic data sets,
        i.e. the time interval from the beginning of the first sub-atomic file
        to the end of the last one. The period of each variable, grouped by
        frequencies, is tabled in files with experiment-like names.

    - Summary (directory)

        A directory with sub-directories of exp-like names containing
        human-readable files. All annotations are listed in the file
        annotations.txt. Existence of a file 'failed_periods.txt' indicates
        differences in the periods of variables. Also, files are created
        for each annotation type.

.. code-block:: text
   :caption: cs_table (directory; optional)

        This directory is created if the option CHECKSUM is enabled. Files with
        experiment-like names contain entries for each checked file consisting of
        a checksum (md5 by default, but any other system may be bound),
        creation_date, and tracking_id; the latter two only if
        corresponding global attributes exist. This information is used to raise
        an annotation, if later versions of the same
        file name apply identical creation_date or tracking_id attributes.

.. code-block:: text
   :caption: data (directory tree)

        Mostly for internal use. The directory structure of the data file ensemble is
        reproduced containing lock-files or atomic NetCDF files with the checksum
        of the original data for each time value.

.. code-block:: text
   :caption: session_logs (directory)

        Internal use

.. code-block:: text
   :caption: tables (directory)

        All tables and configuration files used for the given check.


Best Practise
=============

**Installation**

.. code-block:: text

    - Get sources
      QC-0.4: download the script install
      QA-DKRZ: git clone https://github.com/IS-ENES-Data/QA-DKRZ

    - Creation of the 'install_configure' file
        if the NetCDF-4 library with HDF5 support is available:
        ./install

        if supported libs should be protected (by hard-links) against deletion:
        ./install --link=path

        if NetCDF libs have to be built:
        ./install --build

    - Edit the 'install_configure' file

    - Verify installation success: ./install --show-inst
      Note that a verification is already done by the next step which eventually
      generates results in /package-path/example/test_I

    - Make executables: ./install [PROJECT]

**Configuration**

.. code-block:: text

    Most of the configuration options have a useful default and some are only
    for very specific conditions occurring at different sites.
    Note that all (specified and/or by default) config-opts are recorded
    in the log-files.

    - Use configuration options on the command-line only for testing or
      re-checking a small set of data.

    - Use a task-file for frequently modified directives. This should contain
      the QA_CONF option naming a configuration file with more static options.

    - PROJECT_DATA should only contain a site specific path to the data set,
      e.g. PROJECT_DATA=/path/data/CORDEX .

    - Use the SELECT option to partition the entire data set. E.g. for CORDEX,
        SELECT AFR-44=   # (may-be with prefixed '.*/' depending on PROJECT_DATA)
      would check the data of every model available for the specified domain.
      On the other hand, SELECT '.*/historical=orog' would find any orography
      file in all historical in the given PROJECT_DATA=sub-dir-tree.

    - Similarly to EXP_PATH_INDEX, option PT_PATH_INDEX defines the name of a
      project table, which is created and utilised to check consistency across
      sub-atomic files of a given variable. The project table is consulted for
      all experiments and versions matching the same name.
      Note: if many annotations about a changed layout are thrown, then chances
      are high that systematically different file selections are checked against
      the same project file.

    - The CORDEX_check-list.conf file provides directives, how to issue annotations.
      If a level of severity was rated higher than the L1 level, then a QA session
      may stop to process the given atomic data set any further.
      The option NOTE_LEVEL_LIMIT=[L]1 would prevent this.

**Operational Mode**

.. code-block:: text

     Before starting to check data, please make sure that everything was set
     properly:

     - Command-line: /package-path/scripts/qa_DKRZ -f file -e_show_conf

       Inspect the configuration options displayed on the screen.

     - Command-line: /package-path/scripts/qa_DKRZ -f file -e_show_exp

       Path and and filename of every SELECTed item will be displayed below
       the executed command-line call. Searching the data base may take a
       somewhat long time, depending on the number of data files attached
       by option PROJECT_DATA without any SELECT search.

     - Command-line: /package-path/scripts/qa_DKRZ -f file -E_next

       Only the first file of an atomic data set resulting from the SELECT evaluation
       will be checked. If everything appears fine in folder QA_RESULTS/check_logs,
       then restart the call without -e_next. This will resume the session.

     - Use nohup for long-term execution in the background. If the script is run
       in the foreground, then command-line option '-m' may be helpful by showing
       the current file name under investigation on a status-line below the
       script call.

     - Examine the QA results in directory QA_RESULTS/check_logs/Summary.

     - Manual termination of a session: if an immediate break is required,
       please inquire the process-id (pid), e.g. by ps -ef, and execute the
       command 'kill -TERM pid'. This will close the current session neatly
       leaving no remnants.

