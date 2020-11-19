#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "hdf5.h"


using namespace std;

/**
 * @brief Convert the csv file to hdf5 file.
 * The csv file must fit the format as:  { x1, x2, ..., xn, y }. And the resulting hdf5 file will contain two dataset "X" and "Y".
 */
void CSV2HDF5(string csv_file_address, string hdf5_file_address) {
  ifstream fin;
  fin.open(csv_file_address);
  if (fin.is_open()) {
    string field, line;
    std::getline(fin, line); // ignore the header information
    bool extend_flag = false;
    hid_t file_id;
    file_id = H5Fcreate(hdf5_file_address.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    std::getline(fin, line);
    istringstream sin(line);
    vector<double> sample;
    vector<double> label;
    while(std::getline(sin, field, ',')) {
      sample.push_back(std::stod(field));
    }
    label.push_back(sample.back());
    sample.pop_back();
    int features_size = sample.size();
    hsize_t x_dims[2] = {1, static_cast<hsize_t>(features_size)}, max_dims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
    hsize_t chunk[2] = {1, static_cast<hsize_t>(features_size)};
    hid_t X_space, X_id, Y_space, Y_id, X_dcpl, Y_dcpl, X_mem_space, Y_mem_space;
    int rank = 2;
    X_space = H5Screate_simple(rank, x_dims, max_dims);
    X_dcpl = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(X_dcpl, 2, chunk);
    X_id = H5Dcreate2(file_id, "/X", H5T_NATIVE_DOUBLE, X_space, H5P_DEFAULT, X_dcpl, H5P_DEFAULT);
    H5Pclose(X_dcpl);

    hsize_t y_dims[1] = {1}, y_max_dims[1] = {H5S_UNLIMITED};
    hsize_t y_chunk[1] = {1};
    Y_space = H5Screate_simple(1, y_dims, y_max_dims);
    Y_dcpl = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(Y_dcpl, 1, y_chunk);
    Y_id = H5Dcreate2(file_id, "/Y", H5T_NATIVE_DOUBLE, Y_space, H5P_DEFAULT, Y_dcpl, H5P_DEFAULT);
    H5Pclose(Y_dcpl);

    H5Dwrite(X_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, sample.data());
    H5Dwrite(Y_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, label.data());
    while (std::getline(fin, line)) {
      istringstream sin2(line);
      sample.clear();
      label.clear();
      while(std::getline(sin2, field, ',')) {
        sample.push_back(std::stod(field));
      }
      label.push_back(sample.back());
      sample.pop_back();
      hsize_t x_count[2], x_start[2];
      x_dims[0]++;
      H5Dset_extent(X_id, x_dims);
      X_space = H5Dget_space(X_id);
      x_start[0] = x_dims[0] - 1;
      x_start[1] = 0;
      x_count[0] = 1;
      x_count[1] = features_size;
      X_mem_space = H5Screate_simple(rank, x_count, NULL);
      H5Sselect_hyperslab(X_space, H5S_SELECT_SET, x_start, NULL, x_count, NULL);
      H5Dwrite(X_id, H5T_NATIVE_DOUBLE, X_mem_space, X_space, H5P_DEFAULT, sample.data());

      hsize_t y_count[1], y_start[1];
      y_dims[0]++;
      H5Dset_extent(Y_id, y_dims);
      Y_space = H5Dget_space(Y_id);
      y_start[0] = y_dims[0] - 1;
      y_count[0] = 1;
      Y_mem_space = H5Screate_simple(1, y_count, NULL);
      H5Sselect_hyperslab(Y_space, H5S_SELECT_SET, y_start, NULL, y_count, NULL);
      H5Dwrite(Y_id, H5T_NATIVE_DOUBLE, Y_mem_space, Y_space, H5P_DEFAULT, label.data());
    }

    H5Sclose(X_space);
    H5Sclose(X_mem_space);
    H5Dclose(X_id);
    H5Sclose(Y_space);
    H5Sclose(Y_mem_space);
    H5Dclose(Y_id);
    H5Fclose(file_id);
  } else {
    cout << "open file failed!" << endl;
  }

}

int main() {
  string csv_file_address, hdf5_file_address;
  // csv_file_address = "iris_fa.csv";
  // hdf5_file_address = "iris_fa.h5";
  csv_file_address = "housing_scale.csv";
  hdf5_file_address = "housing_scale.h5";
  CSV2HDF5(csv_file_address, hdf5_file_address);
  return 0;
}
