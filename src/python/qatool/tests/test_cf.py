import unittest

from pkg_resources import resource_filename

from qatool.cf import CFCheck

static_files = {
    'cf_pass_2_1'                    : resource_filename('qatool', 'tests/data/Pass/chap2/cf_2.1.nc'),
    'cf_fail_2_2'                    : resource_filename('qatool', 'tests/data/Fail/chap2/cf_2.2.nc'),
    }

class TestCF(unittest.TestCase):
    def setUp(self):
        self.cf = CFCheck()

    def test_cf_pass_2_1(self):
        result = self.cf.run(static_files['cf_pass_2_1'])
        self.assertTrue(result)

    def test_cf_fail_2_1(self):
        result = self.cf.run(static_files['cf_fail_2_2'])
        self.assertFalse(result)
    
