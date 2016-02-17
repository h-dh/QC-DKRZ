import subprocess

from qatool.base import BaseCheck

import logging
logger = logging.getLogger(__name__)

class CFCheck(BaseCheck):

    def run(self, ds):
        cmd = ['dkrz-cf-checker']
        if self.verbose:
            cmd.append('--debug')
        if self.criteria == 'strict':
            cmd.append('-R')
        cmd.append(ds)

        success = False
       
        try:
            logger.info('running %s', cmd)
            output = subprocess.check_output(cmd)
            if "PASS" in output:
                success = True
            self.write_output(output)
        except subprocess.CalledProcessError as err:
            if "PASS" in err.output:
                success = True
            self.write_output(err.output)
        return success

