import argparse
import argcomplete

import logging
logging.basicConfig(format='%(message)s', level=logging.WARN)
logger = logging.getLogger(__name__)


def create_parser():
    parser = argparse.ArgumentParser(
        prog="cfchecker",
        usage='''cfchecker [<options>] [<args>]''',
        description="Checks NetCDF files to the Climate and Forecast Conventions.",
        )
    parser.add_argument("-v",
                        dest="verbose",
                        help="enable verbose mode",
                        action="store_true")
    parser.add_argument("--service",
                        dest='service',
                        required=False,
                        type=type(''),
                        default='http://localhost:8983/solr/birdhouse',
                        help="Solr URL. Default: http://localhost:8983/solr/birdhouse",
                        action="store")

    return parser


def execute(args):
    if args.verbose:
        logger.setLevel(logging.DEBUG)

    logger.info('Done.')

        
def main():
    logger.setLevel(logging.INFO)

    parser = create_parser()
    argcomplete.autocomplete(parser)

    args = parser.parse_args()
    execute(args)
