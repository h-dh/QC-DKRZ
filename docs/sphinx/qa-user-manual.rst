DKRZ Quality Assurance Tool
===========================

The Quality Assurance (QA) software developed at the DKRZ tests the conformance
of meta-data of climate simulations given in NetCDF format to conventions and
rules of projects. Additionally, the QA checks time sequences of data
and the technical state of data (i.e. occurrences of Inf or NaN, gaps,
replicated sub-sets, etc.)
for atomic data set entities, i.e. variable and frequency, like 'tas' and 'mon'
for monthly means of 'Near-Surface Air Temperature'. When the amount of
data is subdivided into several files, then changes between these files in terms
of (auxiliary) coordinate variables will be detected as well as gaps or
overlapping time ranges. This may also apply to consecutive experiments.

At present, the QA checks data of the projects CMIP5 and CORDEX by consulting
tables based on a controlled vocabularies and requirements provided by the
projects. When such prior information is given about directory structures,
format of file names, variables and attributes, geographical domain, etc.,
then any deviation from compliance will be annotated in the QA results.

Please, direct questions or comments to hollweg@dkrz.de


Requirements
============

The tool requires the BASH shell and a C/C++ compiler.


Versions
========

The tool is distributed in two versions:

- QC-0.4 (stable)

  Purpose is to operate checks of CMIP5 and CORDEX data.
  This QA software package is maintained with the version control tool
  ``subversion``. The installation is done by script. Please, click
  `install <http://svn-mad.zmaw.de/svn/mad/Model/QualCheck/QC/branches/QC-0.4/install>`_
  to download; the script is also located in the QC-0.4 package. Note that
  depending on the browser, the script could be saved as ``install.txt`` and
  perhaps without execute permission. If so, run ``bash install.txt`` (or
  ``chmod u+x install``).
  After installation and configuration, QA checks are started on the command-line
  by:
  ``/package-path/scripts/qcManager [options]``

- QA-DKRZ (v.0.5-beta)

  Development stage: the software resides on GitHub and is accessible by:
  ``git clone https://github.com/h-dh/QA-DKRZ``.
  QA runs are started on the command-line by:
  ``/package-path/scripts/qa_DKRZ [options]``.

  While former versions took for granted that the NetCDF Climate and
  Forecast (CF) Meta Data Conventions were held when specified, the
  conformance is now tested. The CF check is both embedded in the QA tool
  itself and provided by a stand-alone tool including a test suite of files.

  - CF Checker

    The CF Conformance checker applies to conventions 1.4 - 1.7draft. Note
    that it takes a few seconds for the installation when it runs for the
    first time.

    Command-line for the stand-alone tool:

    ``/package-path/scripts/dkrz-cf-checker [options] [files]``

    -C str        CF conventions string; taken from global attributes by default.
    -F path       Find all NetCDF files in the sub-dir tree starting at path.
    -p str        Path to one or more NetCDF file; this is applied only to files.                without any path component.
    -R            Apply also recommendations given in the CF conventions.
    -x str        Path to QA-DKRZ/bin; required if the script was copied to the
                  outside of the package (note that a symbolic link works without
                  this option).
    --ts          Run the CF Test Suite located within the package (file names
                  may be given additionally).

  [path/]files  files

  - CF Test Suite

    A collection of NetCDF files designed to cover all rules of the CF Conventions
    derived from the examples of the PDF documents. There are two branches:

    PASS
      Proper meta-data and data sets, which should never output a failure. No. of files: 81

    FAIL
      Each file contains a single break of a rule and should never pass the test. No. of files: 187

Note that the name of the tool has changed over the years as well as the name of the starting script. At first, the acronym QC (Quality Control/Check) was used, but was changed to the more commonly used term QA (Quality Assurance). The names qcManager, qc_DKRZ, and qa_DKRZ start an identical script (also, the underscore may be replaced by a hyphen). Option names containing QC or qc are still usable for backward compatibility.


Installation
============

A script install is provided to manage different installation/update modes.
install runs on linux utilising Bash and C/C++ (AIX works, too).
Environmental variables CC, CXX, CFLAGS, and CXXFLAGS are accepted.
install establishes access to libraries, which may be linked or built, as well
as updating sources and executables.
A script snippet install_configure is installed during the first run of install.
If option ``--build`` is not applied, then the user is notified to edit
``install_configure``.
After the installation, compiler settings and paths to libraries are always
read from ``install_configure``.
Proceedings of installation/update are logged in file ``install.log``.
The full set of options is described by ``./install --help``.
Compilation of executables: adding key-word CMIP5 to the call of install
(CORDEX is the default) compiles for CMIP5.
A test-run is started automatically generating some results in the directory
``/package-path/example/test_I``.

Building libraries
------------------

Command-line: ``./install --build [options]``

Download and install libraries zlib-1.2.8 from www.zlib.net,
hdf5-1.8.9 from www.hdfgroup.org and netcdf-4.3.0 from www.unidata.ucar.edu
(shared, no FORTRAN, non-parallel).
QA-DKRZ (and the CF checker) requires also the udunits package from
http://www.unidata.ucar.edu/packages/udunits.
The libraries are built in sub-directory local/source.
If libraries had been built previously, then the sources are updated and
the libraries are rebuilt.


Update
======

Updating QC/QA from the repository and re-compilation of executables is done
easiest by using the install script. There are two modes: automatic and manual
updates. Please note that the execution of package-path/install [project] does
not call for any updates by default; this will only recompile locally changed
C/C++ programs.

Manual Update
-------------

Command-line: ``/package-path/install --up[date] [options]``

This applies any changes in the repository. If C/C++ programs are affected,
then executables are remade. Please note that libraries are not updated.
If you want to do so, then you have to set option ``--build``.

Automatic Update
----------------

Command-line: ``/package-path/install --auto-up [options]``

- Once ``--auto-up`` was set, the package will always be synchronised to the
  repository at the beginning of each QC/QA session started by
  ``/package-path/scripts/qa-DKRZ``.

- This mode may be disabled by option ``--auto-up=disable``.

- Enabling/disabling the auto-mode works also during operational runs of the
  qa-DKRZ script.

- For QA-DKRZ, there is a daily search for updates of the required tables
  (``area-type-table.xml``, ``cf-standard-name-table.xml``,
  ``standardized-region-names.html``) from http://www.cfconventions.org.


QA Configuration
================

The QA would run basically on the command-line only with the specification of
the target(s) to be checked. However, using options facilitates a check in
particular for a given project.
Configuration options could be supplied on the command-line and by one or more
conf-files. Configuration options follow a specific syntax with case-insensitive
option names (e.g. ``KEY-WORD`` and ``kEy-WorD``).

==========    ====  =================    =====================================================
KEY-WORD      \     \                    enable key-word (equivalent to key-word=t)
KEY-WORD      \=    value[,value ...]    assign key-word a (comma-separated) value (overwrite)
KEY-WORD      +\=   value[,value ...]    same, but append
==========    ====  =================    =====================================================


Partitioning of data
--------------------

The specification of a path to a directory tree by option ``PROJECT_DATA`` results
in a check of every NetCDF files found within the entire tree. This can be further
customised by the key-words ``SELECT`` and ``LOCK``, which follow special rules.

============  ====  ===============   =====   ==========================  ===========================================
KEY-WORD      \     \                 \       variable[, variable, ...]   specified variables for every path;
                                                                          +\= mode for sub-paths
KEY-WORD      \     path[,path,...]   [\=]    \                           all variables within the specified path;
                                                                          +\= mode for sub-paths
KEY-WORD      \     path[,path,...]   \=      [variable[, variable, ...]  specified variables within the given paths;
                                                                          the \= char in-between
                                                                          is mandatory; +\= mode for sub-paths
KEY-WORD      \=    path[,path,...]   \=      [variable[, variable, ...]  same, but overwrite mode
KEY-WORD      +\=   path[,path,...]   \=      [variable[, variable, ...]  append to a previous assignment
============  ====  ===============   =====   ==========================  ===========================================

- Highest precedence is for options on the command-line.

- If path has no leading '/', then the search is relative to the path specified
  by option PROJECT_DATA.

- Special for options ``-S arg`` or an appended plain string on the command-line:
  cancellation of previous SELECTions in any configuration file.

- If SELECTions are specified on the command-line (options -S) with an absolute
  path, i.e. beginning with '/', then PROJECT_DATA specified in any other
  config-files is cancelled..

- All selections refer to the atomic data set of a given variable, i.e. all
  sub-temporal files; even if a file name is appended to a path.

- Locking gets the higher precedence over selection.

- Path and variable indicators are separated by the '=' character, which may be
  omitted when there is no variable (except the case that each of the paths
  contains no '/' character).

- Regular expressions may be applied for both path(s) and variable(s).

- If an expanded path points to a sub-dir tree, then this is searched for the
  variables.

- A variable is selected if the expanded variable part fits the beginning of the#
  name, e.g. specifying 'tas' would select all tas, tasmax, and tasmin files.
  Note that every file name begins with ``variable_...`` for CMIP5/CORDEX, thus,
  use ``tas_`` for this alone.


Configuration files
===================

A description of the configuration options is given in the repository.

QC-0.4
      ``/package-path/tables/SVN_defaults/"project-name"_qc.conf``
QA-DKRZ
      ``/package-path/tables/projects/"project-name"/"project-name"_qa.conf``

Configuration files and options may be specified multiply following a given
precedence. This facilitates to have a file with short-term options (in a
file attached to the -f option on the command-line), another one with settings
to site-specific demands, which are robust against changes in the repository,
and long-term default settings from the repository. All options may be specified
on the command-line plus some more (
``/package-path/QA-DKRZ/scripts/qa_DKRZ --help``
and ``/package-path/QC_0.4/scripts/qcManager --help``, respectively).
A sequence of configuration files is accomplished by ``QA_CONF=conf-file``
assignments embedded in the configuration files (nesting depth is unrestricted).
The precedence of configuration files/options is given below from highest to
lowest.

-  directly on the command-line
-  in the task-file (``-f file``) specified on the command-line.
-  QA_CONF assignments embedded (descending starts from the ``-f file``).
-  site-specific files provided by files located straight in ``/package-path/tables``.
-  defaults for the entire project:

   QC-0.4
     ``/package-path/tables/SVN_defaults``.

   QA-DKRZ
     ``/package-path/tables/projects/ project-name``.


Annotation
==========

Any failed QA test raises an annotation. In fact, testing and issuing annotations
are different processes. The way of testing is done always by the program;
the user has no influence. However, issuing annotations is controlled by the user
by means of configuration options and the check-list table. Location:

  QC-0.4
     ``/package-path/tables/SVN_defaults/``*project-name*``_check-list.conf``

  QA-DKRZ
     ``/package-path/tables/projects/``*project-name*``_check-list.conf``
     and ``CF_check-list.conf``

A check may-be discarded entirely or for specified variables. Also, specific data
values of given variables may be excluded from annotating. Details are explained
in the check-list files.


QA Results
==========

Log-files and annotations are written to the location specified by the
configuration option QA_RESULTS. Check results of data files are grouped in
directories or files based on the options beginning with the token EXP.
The purpose of such an experiment-like name is to tag a larger set of various
checks with a name that corresponds to a certain volume of netCDF files. There
are several ways to define a name with the precedence of the following options:
explicit EXP_NAME=string, a pattern common to file names (EXP_FNAME_PATTERN),
components of paths common to the ensemble (EXP_PATH_INDEX), and the default
name 'all-scope'.

In general, five sub-directories are created:

-  ``check_logs`` (directory)

   - ``exp-like-name.log`` (file)

       The files are written in the human-readable YAML format (optional for QC-0.4
       by YAML in the configuration or ``-yaml`` on the command-line; there are
       tools, which transform YAML coding into XML). Each entry of a log-file
       represents the check of a (sub-temporal) data file regardless of whether
       annotations were issued or not.

   - ``Annotations.yaml``

       Only log-file entries having annotations are extracted.

       QC-0.4
         the name of the directory is _Notes; not created when there are no
         annotations.

  - ``_Periods`` (directory)

        CORDEX for instance, does not prescribe the periods of atomic data sets,
        i.e. the time interval from the beginning of the first sub-temporal file
        to the end of the last one. The period of each variable, grouped by
        frequencies, is tabled in files with experiment-like names.

  - ``Summary`` (directory)

        A directory with sub-directories of exp-like names containing
        human-readable files. All annotations are listed in the file
        ``annotations.txt`` as well as differing periods. Also, files are created
        for each annotation type.

-  ``cs_table`` (directory; optional)

        This directory is created if the option CHECKSUM is enabled. Files with
        experiment-like names contain entries for each checked file consisting of
        a checksum (md5 by default, but any other system may be bound),
        ``creation_date``, and ``tracking_id``; the latter two only if
        corresponding global attributes exist. This information is used to raise
        an annotation, if later versions of the same
        file name apply identical creation_date or tracking_id attributes.

-  ``data`` (directory tree)

        Mostly for internal use. The directory structure of the data file ensemble is
        reproduced containing lock-files or atomic NetCDF files with the checksum
        of the original data for each time value.

-  ``session_logs`` (directory)

        Internal use

-  ``tables`` (directory)
        All tables and configuration files used for the given check.


Best Practise
=============

**Installation**

   -  Get sources

      QC-0.4
        download the script ``install``

      QA-DKRZ
        ``git clone https://github.com/h-dh/QA-DKRZ``

   -  Create ``install_configure``
        if the NetCDF-4 library with HDF5 support is available: ``./install``

        if supported libs should be protected against deletion (by hard-links):
          ``./install --link=path``

        if NetCDF libs have to be built: ``./install --build``

   -  Edit file ``install_configure``

   -  Optional: Verify installation success: ``./install --show-inst``

   -  Make executables: ``./install [PROJECT]``.
      Note that this performs a test run with results in
      ``package-path/example/test_I``

**Configuration**

    Most of the configuration option have a useful default and some options only
    for very specific conditions occurring at different sites.

    - Use configuration options on the command-line only for testing or
      re-checking a small set of data.

    - Use a task-file for frequently modified directives, which names a
      QA/QC_conf file.

    - PROJECT_DATA should only contain a site specific path to the data set,
      e.g. PROJECT_DATA=/path/data/CORDEX .

    - Use the SELECT option for partitioning the amount of data to be checked,
      e.g. SELECT AFR-44 would check the data of every model available for the
      specified domain of CORDEX data.

    - On the other hand, SELECT '.*/historical=orog' would find any orography
      file in all historical in the given PROJECT_DATA=sub-dir-tree.

    - Option EXP_PATH_INDEX is for partitioning the results in
      QC_RESULTS/check_logs. Please choose unique names among the total
      path components to data,
      e.g. let the path be ``/path 10 /AFR-44 9 /SMHI 8 /CCCma-CanESM2 7``
      ``/historical 6 /r1i1p1 5 /SMHI-RCA4 4 /v1 3 /day 2 /clh 1``,
      index given by subscripts is
      only for illustration, and have EXP_PATH_INDEX=9,8,7,6,5,3 , then the
      resulting name would be ``AFR-44_SMHI_CCCma-CanESM2_historical_r1i1p1_v1``.

    - Similarly, PT_PATH_INDEX defines the name of a project table, which is
      created and utilised to check consistency across sub-temporal files of
      a given variable; PT_PATH_INDEX=9,8,7,4 by default for CORDEX, e.g.
      ``AFR-44_SMHI_CCCma-CanESM2_SMHI-RCA4``, which is consulted for all similar
      experiments and versions.

    - The CORDEX_check-list.conf file provides directives, how issue annotations.
      If a level of severity higher than L2 was rated, then a QA/QC session may
      stop. The option NOTE_LEVEL_LIMIT=1 would prevent any interruption.

**Operational Mode**

     Before starting to check data, please make sure that everything was set
     properly:

     - Command-line: ``/package-path/scripts/qa_DKRZ -f file -e_show_conf``

       Inspect the configuration options displayed on the screen.

     - Command-line: ``/package-path/scripts/qa_DKRZ -f file -e_show_exp``

       Path and and filename of every SELECTed item will be displayed below
       the executed command-line call. Searching the data base may take a
       somewhat long time, depending on the amount of data.

     - Command-line: ``/package-path/scripts/qa_DKRZ -f file -E_next``

       Only the first path-variable resulting from the SELECT evaluation will
       be checked. If everything appears fine in folder QA_RESULTS/check_logs,
       then restart the call without e_next. This will resume the session.

     - Use nohup for long-term execution in the background. If the script is run
       in the foreground, then command-line option -m may be helpful by showing
       the current file name under investigation on a status-line below the
       script call.

     - Examine the QA results in directory QA_RESULTS/check_logs.
       Human-readable annotations are given in sub-directory ``Summary``.

     - Manual termination of a session: if an immediate break is required,
       please inquire the process-id (pid), e.g. by ps -ef, and execute the
       command kill -TERM pid. This will close the current session neatly
       leaving no remnants.

