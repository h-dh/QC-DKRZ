
An example for the QA to generate of some results.

Please notice that all flaws of the data are arbitrary
just to demonstrate some features of the QA.

This example would only work after having run
sucessfully the 'install' command.

Starting the example within a console with comman within a console with command:
================================================================================

0) Show the currrent configuration:

../scripts/qa-DKRZ -f qa-test.task -e_show_conf

1) Show the currrent selection of experiments and paths:

../scripts/qa-DKRZ -f qa-test.task -e_show_exp

--------------------------------------------------

I) Checking meta-data, time values and data. 
   Directives as given by QA-DKRz/tables/project/CORDEX/CORDEX_check-list.conf

../scripts/qcManager -f qa-test.task \
   -E_EMAIL_SUMMARY=name@site.dom \
   -E_EMAIL_TO=name@site.dom 

Note: any configuration option (either in qc-test.task or CORDEX_qc.conf
can be given on the command-line with the prefix '-E_'.

Note: The configuration assignments EMAIL_SUMMARY and EMAIL_TO must
be given real adresses in order to get active.

Note: a gap in the time values causes the generation of a qa_lock<name>.txt 
file in directory:
   path/example/test_X/data/AFR-44/CLMcom/MPI-ESM-LR/historical/r1i1p1/CCLM4-8-17/v1/mon/evspsbl

--------------------------------------------------

II) Checking only meta-data. Results are in sub-directory test_II

../scripts/qa-DKRZ -f qa-test.task \
-E_CHECK_MODE=meta \
-E_QC_RESULTS=path/QA-DKRZ/example/test_II

Note: checks of time values and data are discarded.

--------------------------------------------------

III) Checking meta-data, time values and data and forcing all
     Directories of QA-DKRZ/tables/projects/CORDEX/CORDEX_check-list.conf
     a ruled out by command-line setting.

../scripts/qa-DKRZ -f qA-test.task \
   -E_EMAIL_SUMMARY=name@site.dom \
   -E_EMAIL_TO=name@site.dom  \
   -E_QC_RESULTS=path/QC-0.4/example/test_III \
   -E_USE_ALWAYS=L1 \

Note: all notification levels of higher severeness are converted to L1.
      a side effect is that no EMAIL_TO delivery took place ( this
      would have been different, if USE_ALWAYS=L1,EM had been set ).

