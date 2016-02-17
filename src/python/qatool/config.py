import os

import qatool
from qatool.configparser import ExtendedConfigParser

import logging
logger = logging.getLogger(__name__)

config = None

def write_options():
    if not config:
        load_configuration()
    for section in config.sections():
        print 'Section:', section
        for name, value in config.items(section):
            print '  %s = %s' % (name, value)

def get_config_value(section, option):
    """Get desired value from  configuration files
    :param section: section in configuration files
    :type section: string
    :param option: option in the section
    :type option: string
    :returns: value found in the configuration file
    """

    if not config:
        load_configuration()

    value = ''

    if config.has_section(section):
        if config.has_option(section, option):
            value = config.get(section, option)

            # Convert Boolean string to real Boolean values
            if value:
                if value.lower() == "false":
                    value = False
                elif value.lower() == "true":
                    value = True

    return value

def load_configuration(cfgfiles=None):
    """Load QA configuration from configuration files.
    The later configuration file in the array overwrites configuration
    from the first.
    """
    global config
    
    config = ExtendedConfigParser(allow_no_value=True)

    if not cfgfiles:
        cfgfiles = _get_default_config_files_location()
        
    loaded_files = config.read(cfgfiles)
    if loaded_files:
        logger.info('Configuration file(s) {} loaded'.format(loaded_files))
    else:
        logger.info('No configuration files loaded. Using default values.')

def _get_default_config_files_location():
    """Get the locations of the standard configuration files. These are:

        1. `/etc/qa.cfg`
        2. `$HOME/.qa.cfg`
        3. `$QA_CFG environment variable`
        
    :returns: configuration files
    :rtype: list of strings
    """

    cfgfiles = [ os.path.join(qatool.__path__[0], "etc", "default.cfg") ]
    
    # configuration file as environment variable
    if os.getenv("QA_CFG"):
        cfgfiles = cfgfiles.append(os.getenv("QA_CFG"))

    # try to guess the default location
    else:
        cfgfiles.append("/etc/qa.cfg")
        if os.getenv("HOME"):
            cfgfiles.append(os.path.join(os.getenv("HOME"), ".qa.cfg"))

    return cfgfiles
