.. _running:

==============
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


