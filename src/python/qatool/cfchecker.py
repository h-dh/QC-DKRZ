import argparse
import argcomplete
import subprocess

from qatool import __version__

import logging
logging.basicConfig(format='%(message)s', level=logging.WARN)
logger = logging.getLogger(__name__)


def create_parser():
    parser = argparse.ArgumentParser(
        prog="cfchecker",
        description="Checks compliance of climate data (NetCDF files) for Climate and Forecast conventions (http://cfconventions.org).",
        )
    parser.add_argument('-V', '--version', action='store_true',
                        help='Display the cfhecker version information.')
    parser.add_argument('--verbose' , '-v', action="count",
                        help="Increase output. May be specified up to three times.")
    parser.add_argument('-R', dest='extra_checks', action='store_true',
                        help='Apply also recommendations given by CF conventions.')
    parser.add_argument('-F', dest='path', action='store',
                        help='Finds recursively all NetCDF files from starting point PATH.')
    parser.add_argument('ncfile', nargs='*', help= "Defines the location of the dataset to be checked.")

    return parser


def execute(args):
    if args.version:
        print("DKRZ cfchecker version %s" % __version__)
        return 0
    if args.verbose == 1:
        logger.setLevel(logging.INFO)
    elif args.verbose == 2:
        logger.setLevel(logging.DEBUG)
    cmd = ['dkrz-cf-checker']
    if args.ncfile:
        cmd.extend(args.ncfile)
    try:
        subprocess.check_output(cmd)
    except subprocess.CalledProcessError as err:
        print err.output
    return 0

        
def main():
    logger.setLevel(logging.INFO)

    parser = create_parser()
    argcomplete.autocomplete(parser)

    args = parser.parse_args()
    execute(args)
