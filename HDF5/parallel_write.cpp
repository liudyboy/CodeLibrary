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

  hid_t plist_id, file_id;
  plist_id = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(plist_id, comm, info); // Stores MPI IO communicator information to the file access property list.

  file_id = H5Fcreate("parallel.h5", H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
  H5Pclose(plist_id);

  hsize_t dims[2];
  hid_t dataspace, dataset_id;
  dims[0] = 10;
  dims[1] = 12;
  int rank = 2;
  dataspace = H5Screate_simple(rank, dims, NULL); // create dataspace
  dataset_id = H5Dcreate(file_id, "X", H5T_NATIVE_INT, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Sclose(dataspace);

  hsize_t count[2], offset[2];
  hid_t mem_space, file_space;
  count[0] = dims[0] / mpi_size;
  count[1] = dims[1];
  offset[0] = mpi_rank * count[0];
  offset[1] = 0;
  mem_space = H5Screate_simple(rank, count, NULL);

  file_space = H5Dget_space(dataset_id);
  H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, NULL, count, NULL);

  int* data = new int [static_cast<int>(count[0] * count[1])];
  for (int i = 0; i < static_cast<int>(count[0] * count[1]); i++)
    data[i] = mpi_rank;

  plist_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_COLLECTIVE);
  H5Dwrite(dataset_id, H5T_NATIVE_INT, mem_space, file_space, plist_id, data);

  H5Dclose(dataset_id);
  H5Sclose(mem_space);
  H5Sclose(file_space);
  H5Pclose(plist_id);
  H5Fclose(file_id);

  MPI_Finalize();
  return 0;
}
