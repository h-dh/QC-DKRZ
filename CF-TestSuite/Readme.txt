CF Convention NetCDF meta-data snippets, i.e. the examples of the 
chapters and appendices of
http://cf-pcmdi.llnl.gov/documents/cf-conventions/1.4/cf-conventions.pdf
http://cf-pcmdi.llnl.gov/documents/cf-conventions/1.6/cf-conventions.pdf

Files are provided in two directory structures 'Pass/chap*' and 'Fail/chap*',
where '*' indicates the corresponding chapter in the document. 

Filenames are prefixed by 'cf_ followed by chapter.subsection[.subsubsect] number 
or by capital lettres indicating an appendix.
  1) Pass: filenames with the numbering of the original example of the doc.
  2) Fail: The sectioning of the document is denoted (by '_') indicating 
           where to look for explanations of non-conformances.

The netCDF files have been generated basically by ncgen applied
in a script, e.g.:
  cd /path/QC-DKRZ/CF-TestSuite/Txt/chap2
  for f in *.txt ; do echo $f ; /path/QC-DKRZ/CF-TestSuite/scripts/mkNc $f ; done
  mv *.nc /path/QC-DKRZ/CF-TestSuite/Nc/chap2

The generation of files
Fail/chap0/cf_0c.nc, Fail/chap2/cf_2.5.1b.nc, and Fail/chap4/cf_4.1h.nc
needed additional effort, thus these files are protected against overwriting.
(please, look at corresponding Readme.txt in the corresponding chapN directories).

The total set of NetCDF files may be generated on the command-line by:
  QC_DKRZ/CF-TestSuite/scripts/mkAllNc

The test-suite or parts of it may be checked by:
  /path/QC-DKRZ/scripts/cf-checker -R -F /path/QC-DKRZ/path/QC-DKRZ/CF-TestSuite/Nc/Fail/chap9 

Please, execute with option --help for more information about options.

