
An example for QCing CORDEX results.

Please notice that all flaws of the data are arbitrary
just to demonstrate some features of the QC.

This example would only work after having run
sucessfully the 'install' command.

If the two statements in the file 'qc-test.task' concening 

Starting the example within a console with comman within a console with command:
================================================================================

0) Show the currrent configuration:

../scripts/qc-DKRZ -f qc-test.task -E_SHOW_CONF

1) Show the currrent selection of experiments and paths:

../scripts/qcManager -f qc-test.task -E_SHOW_EXP

--------------------------------------------------

I) Checking meta-data, time values and data. 
   Directives as given by QC-0.4/tables/CORDEX_check-list.conf

../scripts/qcManager -f qc-test.task \
   -E_EMAIL_SUMMARY=name@site.dom \
   -E_EMAIL_TO=name@site.dom 

Note: any configuration option (either in qc-test.task or CORDEX_qc.conf
can be given on the command-line with the prefix '-E_'.

Note: The configuration assignments EMAIL_SUMMARY and EMAIL_TO must
be given real adresses in order to get active.

Note: a gap in th etime values causes the generation of a qc_lock<name>.txt 
file in directory:
   path/example/test_X/data/AFR-44/CLMcom/MPI-ESM-LR/historical/r1i1p1/CCLM4-8-17/v1/mon/evspsbl

ToDo: an explanation of the results is missing in this file. 
But, a short explanation would be given by email.

--------------------------------------------------

II) Checking only meta-data. Results are in sub-directory test_II

../scripts/qc-DKRZ -f qc-test.task \
-E_CHECK_MODE=meta \
-E_QC_RESULTS=/scratch/local1/k204145/hdh/QC-0.4/example/test_II

Note: checks of time values and data are discarded.

--------------------------------------------------

III) Checking meta-data, time values and data and forcing all
     Directories of QC-0.4/tables/CORDEX_check-list.conf
     a ruled out by command-line setting.

../scripts/qc-DKRZ -f qc-test.task \
   -E_EMAIL_SUMMARY=name@site.dom \
   -E_EMAIL_TO=name@site.dom  \
   -E_QC_RESULTS=/scratch/local1/k204145/hdh/QC-0.4/example/test_III \
   -E_USE_ALWAYS=L1 \

Note: all notification levels of higher severeness are converted to L1.
      a side effect is that no EMAIL_TO delivery took place ( this
      would have been different, if USE_ALWAYS=L1,EM had been set ).

