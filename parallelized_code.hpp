#include <fstream> // For input and output file stream operations
#include <vector>


using namespace std;


void insert_remove_sgl_stack(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);
void insert_remove_sgl_queue(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);
void insert_remove_treiber(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);

void insert_remove_mns(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);

void insert_remove_treiber_elim(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);

void insert_remove_sgl_elim(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);


void insert_remove_stack_flat(vector<int> &input_data, vector<int> &output_data, int thread_id, int buffer_type);
