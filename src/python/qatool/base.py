import os

class BaseCheck(object):
    def __init__(self, verbose=False, criteria='normal', output_filename='-', output_format='text'):
        self.verbose = verbose
        self.criteria = criteria
        self.output_filename = output_filename
        self.output_format = output_format
        self.setup_runner()

    def setup_runner(self):
        try:
            os.remove(self.output_filename)
        except OSError:
            pass

    def run(self, ds):
        """
        Run all checks on dataset.

        Overwrite this method in your checker.
        """
        pass


    def write_output(self, output=None):
        if self.output_filename == '-':
            print output
        else:
            with open(self.output_filename, 'a') as fp:
                fp.write(output)
