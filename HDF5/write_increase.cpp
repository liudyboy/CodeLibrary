#include <iostream>
#include "hdf5.h"

using namespace std;

void fill_array(int len, int val, int *array) {
  for (int i = 0; i < len; i++)
    array[i] = val + i;
}

int main(int argc, char **argv) {

  hid_t file_id;
  file_id = H5Fcreate("parallel2.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  hsize_t dims[2] = {2, 12}, max_dims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
  hsize_t chunk[2] = {2, 12};
  hid_t dataspace, dataset_id, dcpl;

  int rank = 2;
  dataspace = H5Screate_simple(rank, dims, max_dims); // create dataspace
  dcpl = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(dcpl, 2, chunk);
  dataset_id = H5Dcreate2(file_id, "/X", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, dcpl, H5P_DEFAULT);
  H5Sclose(dataspace);
  H5Pclose(dcpl);

  int times = 4;
  hsize_t count[2], offset[2];
  hid_t dataset_dataspace = H5Dget_space(dataset_id), mem_dataspace;
  int data[24];
  for (int i = 0; i < times; i++) {
    fill_array(24, i + 3, data);
    if (i == 0) {
      H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    } else {
      dims[0] += 2;
      H5Dset_extent(dataset_id, dims);
      dataset_dataspace = H5Dget_space(dataset_id);
      count[0] = 2;
      count[1] = 12;
      offset[0] = dims[0] - 2;
      offset[1] = 0;
      mem_dataspace = H5Screate_simple(2, count, NULL);
      H5Sselect_hyperslab(dataset_dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
      H5Dwrite(dataset_id, H5T_NATIVE_INT, mem_dataspace, dataset_dataspace, H5P_DEFAULT, data);
    }
  }


  H5Dclose(dataset_id);
  H5Sclose(dataset_dataspace);
  H5Sclose(mem_dataspace);
  H5Fclose(file_id);

  return 0;
}
