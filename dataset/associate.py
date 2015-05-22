#!/usr/bin/python

from __future__ import print_function
import argparse, sys, os, numpy

default_max_diff = 0.02
def read_from_file(filename):
    """
    Reads a trajectory from a text file. 

    Args: 
      filename -- File name
    
    Returns:
      dict -- dictionary of (stamp,data) tuples

    File format:
      The file format is "stamp d1 d2 d3 ...", where stamp denotes the time 
      stamp to be matched, and "d1 d2 d3.." is arbitary data (e.g., a 3D 
      position and 3D orientation) associated to this timestamp. 
    """
    with open(filename, 'r') as f:
        lines = f.read().replace(","," ").replace("\t"," ").split("\n")
        data = []
        for line in lines:
            line = line.strip()
            if len(line)==0 or line.startswith('#'):
                continue
            vals = line.split()
            if vals:
                vals[0] = float(vals[0])
                data.append(vals)
        data.sort(key=lambda x: x[0])
        return data

def find_matching(data1, data2, max_diff):
    """
    Associate two dictionaries of (stamp,data). As the time stamps never match 
    exactly, we aim to find the closest match for every input tuple.

    Args:
      data1 -- base data
      data2 -- matching data
      max_diff -- search radius for candidate generation

    Output:
      matches -- list of matched tuples ((stamp1,data1),(stamp2,data2))
    """
    i1, i2 = 0, 0
    matches = {}
    while i1 < len(data1) and i2 < len(data2):
        diff = data1[i1][0] - data2[i2][0]
        if abs(diff) < max_diff:
            matches[i1] = [i2]
            i1, i2 = i1 + 1, i2 + 1
        else:
            if diff < 0.0:
                i1 += 1
            else:
                i2 += 1
    return matches


def associate(base_file, associate_files, max_diff=default_max_diff):
    # Read data from file
    base_data = read_from_file(base_file)
    associate_data = [read_from_file(path) for path in associate_files]
    # Find matching bewteen each associate data and base data
    matches = {}
    for assc_data in associate_data:
        new_matches = find_matching(base_data, assc_data, max_diff)
        if not matches:
            matches = new_matches
        else:
            common_indices = set(matches.keys()) & set(new_matches.keys())
            for index in matches.keys():
                if index in common_indices:
                    matches[index].extend(new_matches[index])
                else:
                    del matches[index]
    # Associate data and timestamp
    output_data = []
    for base_ind, assc_inds in matches.items():
        d = [base_data[base_ind]]
        for ind, assc_data in zip(assc_inds, associate_data):
            d.append(assc_data[ind])
        output_data.append(d)
    output_data.sort(key=lambda x: x[0][0])
    return output_data

def main():
    # parse command line
    parser = argparse.ArgumentParser(
        description=('This script takes two data files with '
                     'timestamps and associates them.'))
    parser.add_argument('base_file', type=str,
                        help='Text file with base timestamp')
    parser.add_argument('associate_files', type=str, nargs='+',
                        help='Text files with timestamps to be associated.')
    parser.add_argument('--max_diff', type=float, default=default_max_diff, 
                        help=('maximally allowed time difference '
                              'for matching entries (default: 0.02)'))
    args = parser.parse_args()

    # Associate data
    output_data = associate(args.base_file, args.associate_files, args.max_diff)

    # Print out
    for d in output_data:
        print(d)

if __name__ == '__main__':
    main()
