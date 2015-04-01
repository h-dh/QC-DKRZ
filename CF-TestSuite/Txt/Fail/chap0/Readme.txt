Please note that giving the netCDF file a specific FAIL-feature
was accomplished by a program which is not part of the repository.
Please, contact hollweg@dkrz.de

cd QC-DKRZ/src

ln -sf Inquiry_fnct/inquiry_true.cpp inquiry_fnct.cpp
ln -sf ModifyNc_fnct/writeTime_InfNaN.cpp modify_fnct.cpp

cd ..
install MODIFY

cd to_netCDF_file

QC-DKRZ/bin/modifyNc.x  -m cf_0c.nc
