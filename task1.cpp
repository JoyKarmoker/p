#include <iostream>
#include <vector>
#include "mpi.h"
using namespace std;

int main(int argc, char *argv[]) {
    int m, n, p, k;
    int number_of_matrix_in_each_processor;
    int total_number_of_processors, rank;
    double cal_start_time, cal_end_time;

    int *mat_a;
    int *mat_b;
    int *mat_c = nullptr;  // Initialize mat_c to nullptr

    int *l_a;
    int *l_b;
    int *l_c;

    int max_matrix_element_value = 50;
    int min_matrix_element_value = 1;
    int matrix_element_value = min_matrix_element_value;

    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Status status;
    MPI_Comm_size(comm, &total_number_of_processors);
    MPI_Comm_rank(comm, &rank);

    if (rank == 0) {
        std::cout << "Give K: ";
        std::cin >> k;
        std::cout << "Give M: ";
        std::cin >> m;
        std::cout << "Give N: ";
        std::cin >> n;
        std::cout << "Give P: ";
        std::cin >> p;

        mat_a = (int *) malloc(k * m * n * sizeof(int));
        mat_b = (int *) malloc(k * n * p * sizeof(int));
        mat_c = (int *) malloc(k * m * p * sizeof(int));

        for (int i = 0; i < m * n * k; i++) {
            if (matrix_element_value > max_matrix_element_value)
                matrix_element_value = min_matrix_element_value;
            mat_a[i] = matrix_element_value++;
        }

        for (int i = 0; i < n * p * k; i++) {
            if (matrix_element_value > max_matrix_element_value)
                matrix_element_value = min_matrix_element_value;
            mat_b[i] = matrix_element_value++;
        }
    }
    cal_start_time = MPI_Wtime();
    MPI_Bcast(&k, 1, MPI_INT, 0, comm);
    MPI_Bcast(&m, 1, MPI_INT, 0, comm);
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);
    MPI_Bcast(&p, 1, MPI_INT, 0, comm);
    
    number_of_matrix_in_each_processor = k / total_number_of_processors;
    if(rank == total_number_of_processors-1)
    {
        number_of_matrix_in_each_processor += k%total_number_of_processors;
    }
    l_a = (int *) malloc(number_of_matrix_in_each_processor * m * n * sizeof(int));
    l_b = (int *) malloc(number_of_matrix_in_each_processor * n * p * sizeof(int));
    l_c = (int *) malloc(number_of_matrix_in_each_processor * m * p * sizeof(int));
    MPI_Scatter(mat_a, number_of_matrix_in_each_processor * m * n, MPI_INT,
                l_a, number_of_matrix_in_each_processor * m * n, MPI_INT, 0, comm);
    MPI_Scatter(mat_b, number_of_matrix_in_each_processor * n * p, MPI_INT,
                l_b, number_of_matrix_in_each_processor * n * p, MPI_INT, 0, comm);
    // MPI_Barrier(comm);

    int m_a[number_of_matrix_in_each_processor][m][n];
    int m_b[number_of_matrix_in_each_processor][n][p];
    int m_c[number_of_matrix_in_each_processor][m][p];

    int idx = 0;
    for (int c = 0; c < number_of_matrix_in_each_processor; c++) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                m_a[c][i][j] = l_a[idx++];
            }
        }
    }

    idx = 0;
    for (int c = 0; c < number_of_matrix_in_each_processor; c++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < p; j++) {
                m_b[c][i][j] = l_b[idx++];
            }
        }
    }

    
    for (int c = 0; c < number_of_matrix_in_each_processor; c++) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < p; j++) {
                m_c[c][i][j] = 0;

                for (int z = 0; z < n; z++) {
                    m_c[c][i][j] += m_a[c][i][z] * m_b[c][z][j];
                }
            }
        }
    }

    idx = 0;
    for (int c = 0; c < number_of_matrix_in_each_processor; c++) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                l_c[idx++] = m_c[c][i][j];
            }
        }
    }

    MPI_Gather(l_c, number_of_matrix_in_each_processor * m * p, MPI_INT, mat_c, number_of_matrix_in_each_processor * m * p, MPI_INT, 0, comm);

    idx = 0;
    if (rank == 0) {
        // Display 1 matrix result if needed
        
        for (int c = 0; c <1; c++) 
        {
            for (int i = 0; i < m; i++) 
            {
                for (int j = 0; j < n; j++) 
                {
                    cout << l_c[idx++] <<" ";
                }
                cout << endl;
            }
            cout << endl;
        }
        
    }

    cal_end_time = MPI_Wtime();
    printf("PID %d: Total time: %lf\n", rank, cal_end_time - cal_start_time);

    MPI_Finalize();
    free(l_a);
    free(l_b);
    free(l_c);
    if (rank == 0) {
        free(mat_a);
        free(mat_b);
        free(mat_c);
    }
    return 0;
}
