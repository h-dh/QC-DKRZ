I)
Creation of file cf_2.1.cn

 cp cf_2.1.ztxt cf_2.1.txt
 /hdh/hdh/QC-DKRZ/CF-TestSuite/mkNc cf_2.1.txt
 mv cf_2.1.nc QC-DKRZ/CF-TestSuite/Nc/Fail/chap2/cf_2.1.cn 

II)
Creation of file cf_2.5.1b.nc

Please note that ncgen corrects _FillValue in cf_2.5.1.txt.
In order to make it reappear, please, execute

ncatted -h -a _FillValue,ipsl,m,f,10.5 cf_2.5.1b.nc

