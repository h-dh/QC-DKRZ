import os
import argparse
import argcomplete
import subprocess

from qatool.cf import CFCheck
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
    parser.add_argument('-R', dest='strict', action='store_true',
                        help='Apply also recommendations given by CF conventions.')
    parser.add_argument('-F', dest='path', action='store',
                        help='Finds recursively all NetCDF files from starting point PATH.')
    parser.add_argument('-f', '--format', default='text', choices=['text', 'html', 'json'], action='store',
                        help="Output format.")
    parser.add_argument('-o', '--output', default='-', action='store',
                        help="Output filename. Default = '-' (standard output)")
    parser.add_argument('-c', '--cfg', action='store',
                        help="Optional configuration file.")
    parser.add_argument('-e', '--env', nargs='*', default=[],
                        help="Set environment variable (key=value).")
    parser.add_argument('ncfile', nargs='*',
                        help= "Defines the location of the dataset to be checked.")

    return parser


def main():
    parser = create_parser()
    argcomplete.autocomplete(parser)

    args = parser.parse_args()

    verbose = False
    criteria = 'normal'
    
    if args.version:
        print("DKRZ cfchecker version %s" % __version__)
        return 0
    if args.verbose == 1:
        logging.root.setLevel(logging.INFO)
    elif args.verbose == 2:
        logging.root.setLevel(logging.DEBUG)
        verbose = True
    if args.strict:
        criteria = 'strict'
    if args.cfg:
        pass
    if args.env:
        user_environ = {k:v for k,v in (x.split('=') for x in args.env) }
        print user_environ

    checker = CFCheck(verbose=verbose, criteria=criteria, output_filename=args.output, output_format=args.format)
    if args.path:
        for path,dir,files in os.walk(args.path):
            for filename in files:
                if filename.endswith(('.nc',)):
                    ds_loc = os.path.abspath(os.path.join(path, filename))
                    checker.run(ds_loc)
    else:
        for ds_loc in args.ncfile:
            checker.run(ds_loc)
