from nose.tools import ok_

from qatool import config

def test_load_config():
    config.load_configuration()
    config.write_options()
    assert config.get_config_value('settings', 'project_data') == '/tmp/data'
    assert config.get_config_value('settings', 'use_strict') == False
    assert config.get_config_value('checks', 'cf') == '1.4'
    assert config.get_config_value('processing', 'nice') == '15'
    
