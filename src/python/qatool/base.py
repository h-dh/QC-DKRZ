class BaseCheck(object):
    def __init__(self, verbose=False, criteria='normal', output_filename='-', output_format='text'):
        self.verbose = verbose
        self.criteria = criteria
        self.output_filename = output_filename
        self.output_format = output_format

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
            with open(self.output_filename, 'w') as fp:
                fp.write(output)
