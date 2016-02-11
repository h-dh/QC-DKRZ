.. _best-pratices:


================
 Best Practises
================

Installation
============

.. code-block:: text

    - Get sources
      QC-0.4: download the script install
      QA-DKRZ: git clone https://github.com/h-dh/QA-DKRZ

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

Configuration
=============

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

Operational Mode
================

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

