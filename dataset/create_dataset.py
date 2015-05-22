from __future__ import print_function
import argparse, os
import numpy as np

from PIL import Image
from associate import associate


def img_to_array(img, dtype):
    x = np.asarray(img, dtype=dtype)
    if len(x.shape)==3:
        x = x.transpose(2, 0, 1)
    else:
        x = x.reshape((1, x.shape[0], x.shape[1]))
    return x


def main():
    defaultgroundtruth = 'rgbd_dataset_freiburg3_long_office_household/groundtruth.txt'
    default_rgb = 'rgbd_dataset_freiburg3_long_office_household/rgb.txt'
    default_depth = 'rgbd_dataset_freiburg3_long_office_household/depth.txt'
    default_output = 'rgbd_dataset_freiburg3_long_office_household'
    # Parse command line argument
    p = argparse.ArgumentParser()
    p.add_argument('--groundtruth_file', default=defaultgroundtruth)
    p.add_argument('--rgb_file', default=default_rgb)
    p.add_argument('--depth_file', default=default_depth)
    p.add_argument('--output_path', default=default_output)
    args = p.parse_args()

    # Associate 3 files
    assc_data = associate(args.groundtruth_file, [args.rgb_file, args.depth_file])

    # Parse data
    n_samples = len(assc_data)
    base_path_rgb = os.path.dirname(args.rgb_file)
    base_path_depth = os.path.dirname(args.depth_file)
    timestamps = np.zeros(n_samples, dtype='float32')
    gtruth = np.zeros((n_samples, 7), dtype='float32')
    rgb = np.zeros((n_samples, 3, 480, 640), dtype='uint8')
    depth  = np.zeros((n_samples, 1, 480, 640), dtype='uint16')
    for i, d in enumerate(assc_data):
        print('processing {}/{}'.format(i+1, n_samples)) 
        timestamps[i] = d[0][0]
        gtruth[i] = map(float, d[0][1:])
        rgb_img = Image.open(open(os.path.join(base_path_rgb, d[1][1])))
        rgb[i] = img_to_array(rgb_img, 'uint8')
        depth_img = Image.open(open(os.path.join(base_path_depth, d[2][1])))
        depth[i] = img_to_array(depth_img, 'uint16')
    print(np.max(depth))
    # Save
    np.savez(args.output_path, 
             timestamps=timestamps, groundtruth=gtruth, rgb=rgb, depth=depth)

if __name__ == '__main__':
    main()
