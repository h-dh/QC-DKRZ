# -*- coding: utf-8 -*-

from setuptools import find_packages
from setuptools import setup

version = '0.5.3'
description = 'The QA tool is a quality assurance checker for metadata in climate data (NetCDF files).'
long_description = (
    open('README.rst').read() # + '\n' +
    #open('AUTHORS.rst').read() + '\n' +
    #open('CHANGES.rst').read()
)

requires = [line.strip() for line in open('requirements.txt')]

classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Science/Research',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX',
        'Programming Language :: Python',
        'Topic :: Scientific/Engineering :: Atmospheric Science',
        ]

setup(name='qatool',
      version=version,
      description=description,
      long_description=long_description,
      classifiers=classifiers,
      keywords='quality assurance climate forecast conventions netcdf esgf cordex cmip5 cf conda',
      author='DKRZ',
      url='https://github.com/IS-ENES-Data/QA-DKRZ',
      license = "Apache License v2.0",
      packages=find_packages(),
      include_package_data=True,
      zip_safe=False,
      test_suite='nose.collector',
      install_requires=requires,
      entry_points = {
          'console_scripts': [
              'cfchecker=qatool.cfchecker:main',
              ]}     
      ,
      )
