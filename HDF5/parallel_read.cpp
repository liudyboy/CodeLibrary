#include <iostream>
#include "hdf5.h"
#include "mpi.h"

using namespace std;

int main(int argc, char **argv) {
  int mpi_size, mpi_rank;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Info info = MPI_INFO_NULL;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &mpi_rank);
  MPI_Comm_size(comm, &mpi_size);

  hid_t file_access, file_id;
  file_access = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(file_access, comm, info);
  char filename[] = "parallel.h5";
  file_id = H5Fopen(filename, H5F_ACC_RDWR, file_access);
  H5Pclose(file_access);

  hid_t dataset, dataset_space;
  dataset = H5Dopen2(file_id, "X", H5P_DEFAULT);
  dataset_space = H5Dget_space(dataset);
  int rank;
  hsize_t *current_dims;
  hsize_t *max_dims;
  rank = H5Sget_simple_extent_ndims(dataset_space);   // get the dataset information
  current_dims= (hsize_t*)malloc(rank*sizeof(hsize_t));
  max_dims=(hsize_t*)malloc(rank*sizeof(hsize_t));
  H5Sget_simple_extent_dims(dataset_space, current_dims, max_dims);

  hsize_t start[2], count[2], stride[2];
  start[0] = mpi_rank * current_dims[0] / mpi_size;
  start[1] = 0;
  count[0] = current_dims[0] / mpi_size;
  count[1] = current_dims[1];
  stride[0] = 1;
  stride[1] =1;
  H5Sselect_hyperslab(dataset_space, H5S_SELECT_SET, start, stride, count, NULL);

  hid_t mem_dataspace;
  mem_dataspace = H5Screate_simple(2, count, NULL);

  // int** data = new int*[2];
  // int *array = new int [2 * 12];
  // for(int i = 0; i < 2; i++)
  //   data[i] = &array[i * 12];
  // H5Dread(dataset, H5T_NATIVE_INT, mem_dataspace, dataset_space, H5P_DEFAULT, array);

  int data[2][12];
  H5Dread(dataset, H5T_NATIVE_INT, mem_dataspace, dataset_space, H5P_DEFAULT, data);

  for (int i = 0; i < mpi_size; i++) {
    if (mpi_rank == i) {
      cout << "rank " << mpi_rank << " :" << endl;
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 12; j++) {
          cout << data[i][j] << " ";
        }
        cout << endl;
      }
    }
    MPI_Barrier(comm);
  }

  H5Sclose(mem_dataspace);
  H5Sclose(dataset_space);
  H5Dclose(dataset);
  H5Fclose(file_id);
}
