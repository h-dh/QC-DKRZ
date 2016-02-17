import subprocess

from qatool.base import BaseCheck

import logging
logger = logging.getLogger(__name__)

class CFCheck(BaseCheck):

    def __init__(self, verbose=False, criteria='normal', output_filename='-', output_format='text'):
        self.verbose = verbose
        self.criteria = criteria
        self.output_filename = output_filename
        self.output_format = output_format
    

    def write_output(self, output=None):
        if self.output_filename == '-':
            print output
        else:
            with open(self.output_filename, 'w') as fp:
                fp.write(output)


    def run(self, ds_loc):
        cmd = ['dkrz-cf-checker']
        if self.verbose:
            cmd.append('--debug')
        if self.criteria == 'strict':
            cmd.append('-R')
        cmd.append(ds_loc)
       
        try:
            logger.info('running %s', cmd)
            output = subprocess.check_output(cmd)
            self.write_output(output)
        except subprocess.CalledProcessError as err:
            self.write_output(err.output)

