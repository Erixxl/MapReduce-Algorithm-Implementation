#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>


using namespace std;


// Struct used for sending all relevant data as one variable
struct thread_params {
	int thread_id; // Thread ID - used for separation between Reducers and Mappers
	int reducers; // No. of Reducer threads
	int mappers; // No. of Mapper threads
	
	ifstream *source_file; // Source file; holds paths to input files
	int *files_left; // No. of files left
	int *current_file; // Current file number
	
	vector<pair<string, int>> *word_list; // Common word list; it's shared among all threads

	vector<bool> *sorted; // Keeps evidence of which maps are not yet sorted
	int *left_unsorted; // No. of maps left to sort

    // For each letter, a list of words with a list of files they are found in (ordered)
	vector<pair<string, vector<int>>> *word_list_a;
	vector<pair<string, vector<int>>> *word_list_b;
	vector<pair<string, vector<int>>> *word_list_c;
	vector<pair<string, vector<int>>> *word_list_d;
	vector<pair<string, vector<int>>> *word_list_e;
	vector<pair<string, vector<int>>> *word_list_f;
	vector<pair<string, vector<int>>> *word_list_g;
	vector<pair<string, vector<int>>> *word_list_h;
	vector<pair<string, vector<int>>> *word_list_i;
	vector<pair<string, vector<int>>> *word_list_j;
	vector<pair<string, vector<int>>> *word_list_k;
	vector<pair<string, vector<int>>> *word_list_l;
	vector<pair<string, vector<int>>> *word_list_m;
	vector<pair<string, vector<int>>> *word_list_n;
	vector<pair<string, vector<int>>> *word_list_o;
	vector<pair<string, vector<int>>> *word_list_p;
	vector<pair<string, vector<int>>> *word_list_q;
	vector<pair<string, vector<int>>> *word_list_r;
	vector<pair<string, vector<int>>> *word_list_s;
	vector<pair<string, vector<int>>> *word_list_t;
	vector<pair<string, vector<int>>> *word_list_u;
	vector<pair<string, vector<int>>> *word_list_v;
	vector<pair<string, vector<int>>> *word_list_w;
	vector<pair<string, vector<int>>> *word_list_x;
	vector<pair<string, vector<int>>> *word_list_y;
	vector<pair<string, vector<int>>> *word_list_z;

    // For each letter, a map of words and list of file numbers (maps are better for lookup)
	map<string, vector<int>> *list_a;
	map<string, vector<int>> *list_b;
	map<string, vector<int>> *list_c;
	map<string, vector<int>> *list_d;
	map<string, vector<int>> *list_e;
	map<string, vector<int>> *list_f;
	map<string, vector<int>> *list_g;
	map<string, vector<int>> *list_h;
	map<string, vector<int>> *list_i;
	map<string, vector<int>> *list_j;
	map<string, vector<int>> *list_k;
	map<string, vector<int>> *list_l;
	map<string, vector<int>> *list_m;
	map<string, vector<int>> *list_n;
	map<string, vector<int>> *list_o;
	map<string, vector<int>> *list_p;
	map<string, vector<int>> *list_q;
	map<string, vector<int>> *list_r;
	map<string, vector<int>> *list_s;
	map<string, vector<int>> *list_t;
	map<string, vector<int>> *list_u;
	map<string, vector<int>> *list_v;
	map<string, vector<int>> *list_w;
	map<string, vector<int>> *list_x;
	map<string, vector<int>> *list_y;
	map<string, vector<int>> *list_z;

    // For each letter, a corresponding mutex used for map insertion
	pthread_mutex_t *a_mutex;
	pthread_mutex_t *b_mutex;
	pthread_mutex_t *c_mutex;
	pthread_mutex_t *d_mutex;
	pthread_mutex_t *e_mutex;
	pthread_mutex_t *f_mutex;
	pthread_mutex_t *g_mutex;
	pthread_mutex_t *h_mutex;
	pthread_mutex_t *i_mutex;
	pthread_mutex_t *j_mutex;
	pthread_mutex_t *k_mutex;
	pthread_mutex_t *l_mutex;
	pthread_mutex_t *m_mutex;
	pthread_mutex_t *n_mutex;
	pthread_mutex_t *o_mutex;
	pthread_mutex_t *p_mutex;
	pthread_mutex_t *q_mutex;
	pthread_mutex_t *r_mutex;
	pthread_mutex_t *s_mutex;
	pthread_mutex_t *t_mutex;
	pthread_mutex_t *u_mutex;
	pthread_mutex_t *v_mutex;
	pthread_mutex_t *w_mutex;
	pthread_mutex_t *x_mutex;
	pthread_mutex_t *y_mutex;
	pthread_mutex_t *z_mutex;

	pthread_mutex_t *read_source_mutex; // Mutex used by Mappers to read from the source file
	pthread_mutex_t *build_list_mutex; // Mutex used when threads need to access & update word lists
	pthread_barrier_t *reduce_wait_barrier; // Barrier used to force all Reducers to wait for Mappers to finish
	pthread_barrier_t *sort_barrier; // Barrier used to force Reducers to wait until all words are parsed
};


// Helper function used for inserting/updating map elements
void insertHelper(map<string, vector<int>> *map, pair<string, int>& elem, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);

	// Make sure each new word has a valid empty vector associated at insertion
	if ((*map).find(elem.first) == (*map).end()) {
		(*map).insert({ elem.first, vector<int>() });
	}

	// Add new value
	(*map)[elem.first].push_back(elem.second);

	pthread_mutex_unlock(mutex);
}


// Parallel sorting of word lists
void sortHelper(map<string, vector<int>>& map, vector<pair<string, vector<int>>>& sorted)
{
    // Clear list
	if (!sorted.empty()) {
		sorted.clear();
	}

    // Sort the file number vectors
	for (auto x : map) {
		sort(x.second.begin(), x.second.end());
		sorted.push_back(x);		
	}

    // Sort the list based on: file number vector length first, word lexicographical position second
	sort(sorted.begin(), sorted.end(), [](pair<string, vector<int>> a, pair<string, vector<int>> b)
		{
			return (a.second.size() > b.second.size())
				|| (a.second.size() == b.second.size() && a.first.compare(b.first) < 0);
		}
	);
}


// Helper function that removes non-letter symbols and converts uppercase to lowercase
string parseWord(string str)
{
	string result = "";

    // Build word letter by letter - ignore non-letter chars
	for (char c : str) {
		if (c >= 'a' && c <= 'z') {
			result = result + c;
		} else if (c >= 'A' && c <= 'Z') {
			result = result + (char)(c - 'A' + 'a');
		}
	}

	return result;
}


// Parse all words from an input file and converts them into (word, file_no) pairs
void parseInputFile(ifstream& in, map<string, int>& output, int file_no)
{
	string word;

    // Read all strings from the input file
	while (in >> word) {
        // Format to lowercase and letters-only
		word = parseWord(word);

		if (word.length() == 0) {
			continue;
		}

		output.insert({ word, file_no });
	}
}


// Multithreading funcion; based on the id of the thread, it will act as either a map function or a reduce function
void *threadFunc(void *arg)
{
	thread_params *params = (thread_params *)arg;

	if (params->thread_id < params->reducers) {
		// Reduce thread

        // Wait for Mappers to finish
		pthread_barrier_wait(params->reduce_wait_barrier);

        // Divide the common word vector into equal parts to avoid interference
		int start, end, pos, size;
		size = params->word_list->size();
		pos = params->thread_id;
		start = pos * (size / params->reducers);
		end = (pos + 1 == params->reducers) ? (size - 1) : ((pos + 1) * ((size / params->reducers)) - 1);

        // Parse all elements from start to end
		for (int i = start; i <= end; ++i) {
			pair<string, int> elem = (*params->word_list)[i];
			int index = elem.first[0] - 'a';

            // Insert element in the corresponding map; use letter-specific mutex for better parallelism
			switch (index) {
				case 0:
					insertHelper(params->list_a, elem, params->a_mutex);
					break;

				case 1:
					insertHelper(params->list_b, elem, params->b_mutex);
					break;
				
				case 2:
					insertHelper(params->list_c, elem, params->c_mutex);
					break;

				case 3:
					insertHelper(params->list_d, elem, params->d_mutex);
					break;

				case 4:
					insertHelper(params->list_e, elem, params->e_mutex);
					break;
				
				case 5:
					insertHelper(params->list_f, elem, params->f_mutex);
					break;
				
				case 6:
					insertHelper(params->list_g, elem, params->g_mutex);
					break;

				case 7:
					insertHelper(params->list_h, elem, params->h_mutex);
					break;
				
				case 8:
					insertHelper(params->list_i, elem, params->i_mutex);
					break;

				case 9:
					insertHelper(params->list_j, elem, params->j_mutex);
					break;

				case 10:
					insertHelper(params->list_k, elem, params->k_mutex);
					break;
				
				case 11:
					insertHelper(params->list_l, elem, params->l_mutex);
					break;

				case 12:
					insertHelper(params->list_m, elem, params->m_mutex);
					break;

				case 13:
					insertHelper(params->list_n, elem, params->n_mutex);
					break;

				case 14:
					insertHelper(params->list_o, elem, params->o_mutex);
					break;
				
				case 15:
					insertHelper(params->list_p, elem, params->p_mutex);
					break;
				
				case 16:
					insertHelper(params->list_q, elem, params->q_mutex);
					break;

				case 17:
					insertHelper(params->list_r, elem, params->r_mutex);
					break;
				
				case 18:
					insertHelper(params->list_s, elem, params->s_mutex);
					break;

				case 19:
					insertHelper(params->list_t, elem, params->t_mutex);
					break;

				case 20:
					insertHelper(params->list_u, elem, params->u_mutex);
					break;
				
				case 21:
					insertHelper(params->list_v, elem, params->v_mutex);
					break;

				case 22:
					insertHelper(params->list_w, elem, params->w_mutex);
					break;

				case 23:
					insertHelper(params->list_x, elem, params->x_mutex);
					break;

				case 24:
					insertHelper(params->list_y, elem, params->y_mutex);
					break;
				
				case 25:
					insertHelper(params->list_z, elem, params->z_mutex);
					break;
			}
		}

		// Wait for all reducer threads to finish before sorting
		pthread_barrier_wait(params->sort_barrier);

		bool done = false;
		int list_pos = 0;

        // Take all letters and sort their corresponding maps
		while (true) {
            // Critical section
			pthread_mutex_lock(params->build_list_mutex);

            // Find the next unordered map
			if (*params->left_unsorted == 0) {
				done = true;
			} else {
				for (list_pos = 0; list_pos < 26; ++list_pos) {
					if ((*params->sorted)[list_pos] == false) {
						break;
					}
				}

				if (list_pos == 26) {
					done = true;
				} else {
					*params->left_unsorted = *params->left_unsorted - 1;
					(*params->sorted)[list_pos] = true;
				}
			}

			pthread_mutex_unlock(params->build_list_mutex);

			if (done) {
				break;
			}

            // Sort based on letter
			switch (list_pos) {
				case 0:
					sortHelper(*params->list_a, *params->word_list_a);
					break;

				case 1:
					sortHelper(*params->list_b, *params->word_list_b);
					break;
				
				case 2:
					sortHelper(*params->list_c, *params->word_list_c);
					break;

				case 3:
					sortHelper(*params->list_d, *params->word_list_d);
					break;

				case 4:
					sortHelper(*params->list_e, *params->word_list_e);
					break;
				
				case 5:
					sortHelper(*params->list_f, *params->word_list_f);
					break;
				
				case 6:
					sortHelper(*params->list_g, *params->word_list_g);
					break;

				case 7:
					sortHelper(*params->list_h, *params->word_list_h);
					break;
				
				case 8:
					sortHelper(*params->list_i, *params->word_list_i);
					break;

				case 9:
					sortHelper(*params->list_j, *params->word_list_j);
					break;

				case 10:
					sortHelper(*params->list_k, *params->word_list_k);
					break;
				
				case 11:
					sortHelper(*params->list_l, *params->word_list_l);
					break;

				case 12:
					sortHelper(*params->list_m, *params->word_list_m);
					break;

				case 13:
					sortHelper(*params->list_n, *params->word_list_n);
					break;

				case 14:
					sortHelper(*params->list_o, *params->word_list_o);
					break;
				
				case 15:
					sortHelper(*params->list_p, *params->word_list_p);
					break;
				
				case 16:
					sortHelper(*params->list_q, *params->word_list_q);
					break;

				case 17:
					sortHelper(*params->list_r, *params->word_list_r);
					break;
				
				case 18:
					sortHelper(*params->list_s, *params->word_list_s);
					break;

				case 19:
					sortHelper(*params->list_t, *params->word_list_t);
					break;

				case 20:
					sortHelper(*params->list_u, *params->word_list_u);
					break;
				
				case 21:
					sortHelper(*params->list_v, *params->word_list_v);
					break;

				case 22:
					sortHelper(*params->list_w, *params->word_list_w);
					break;

				case 23:
					sortHelper(*params->list_x, *params->word_list_x);
					break;

				case 24:
					sortHelper(*params->list_y, *params->word_list_y);
					break;
				
				case 25:
					sortHelper(*params->list_z, *params->word_list_z);
					break;
			}		
		}
	} else {
		// Map thread
		ifstream input;
		string name;
		int file_number;
		bool stop = false;
		bool skip = false;

        // Use a temporary aggregate list to store all (word, file no.) pairs found
		vector<pair<string, int>> aggregate_list;

		while (true) {
            // Critical section
			pthread_mutex_lock(params->read_source_mutex);

			// Safely read from file - find next input file
			if (*params->files_left == 0 || params->source_file->eof()) {
				stop = true;
			} else {
				*params->files_left = *params->files_left - 1;
				*params->current_file = *params->current_file + 1;
				file_number = *params->current_file;

				*params->source_file >> name;
				input.open(name);

				if (input.fail()) {
					cout << "Error: thread " << params->thread_id << " could not open file no. " << *params->current_file << "\n";
					skip = true;
				}
			}

			pthread_mutex_unlock(params->read_source_mutex);

			if (stop) {
				break;
			}

			if (skip) {
				skip = false;
				input.close();
				continue;
			}

            // Use a map to ignore duplicates
			map<string, int> word_map;

            // Parse the text from the input file
			parseInputFile(input, word_map, file_number);

            // Add all found pairs to the aggregate list
			for (pair<string, int> elem : word_map) {
				aggregate_list.push_back(elem);
			}

			input.close();
		}

        // Critical section
		pthread_mutex_lock(params->build_list_mutex);

        // If you're done, add your work to the common word list
		(*params->word_list).insert((*params->word_list).end(), aggregate_list.begin(), aggregate_list.end());

		pthread_mutex_unlock(params->build_list_mutex);

        // Signal to the Reducer threads that you're done
		pthread_barrier_wait(params->reduce_wait_barrier);
	}

	return NULL;
}


// Attempts to convert str into a in integer and save its value in val
// Return false if str contains anything but an integer
bool stringToInt(string str, int &val)
{
	try {
		size_t pos;
		val = stoi(str, &pos);

		// stoi attempts to convert to integer until it encounters a non-digit character
		return (pos == str.length());
	} catch (...) {
		// Return false if conversion fails for any reason
		return false;
	}
}


// Writes data in output file
void writeOutput(vector<pair<string, vector<int>>> list, string out_file)
{
	ofstream out(out_file);

	for (auto x : list) {
		out << x.first << ":[";

		bool space = false;
		for (auto y : x.second) {
			if (space) {
				out << ' ';
			} else {
				space = true;
			}

			out << y;
		}

		out << "]\n";
	}

	out.close();
}


// Main function
int main(int argc, char **argv)
{
	// Check for correct number of arguments
	if (argc != 4) {
		cout << "Error: incorrect number of parameters.\n";
		cout << "Correct format: ./tema1 <no_mappers> <no_reducers> <input_file>\n";
		return 0;
	}

	string arg1, arg2;
	arg1 = argv[1];
	arg2 = argv[2];

	int mappers, reducers;

	// Check if the first two arguments are integers
	if (!stringToInt(arg1, mappers) || !stringToInt(arg2, reducers)) {
		cout << "Error: <no_mappers> and <no_reducers> must be positive integers.\n";
		return 0;
	}

	// Check for invalid number of mappers/reducers
	if (mappers < 1 || reducers < 1) {
		cout << "Error: <no_mappers> and <no_reducers> must be positive integers.\n";
		return 0;
	}

	ifstream input_file(argv[3]);

	if (input_file.fail()) {
		cout << "Error: failed to open <input_file>.\n";
		return 0;
	}

    // Building the struct used as input
	int file_count = 0;
	int file_index = 0;
	input_file >> file_count;

	vector<pair<string, int>> word_list;

	vector<pair<string, vector<int>>> word_list_a;
	vector<pair<string, vector<int>>> word_list_b;
	vector<pair<string, vector<int>>> word_list_c;
	vector<pair<string, vector<int>>> word_list_d;
	vector<pair<string, vector<int>>> word_list_e;
	vector<pair<string, vector<int>>> word_list_f;
	vector<pair<string, vector<int>>> word_list_g;
	vector<pair<string, vector<int>>> word_list_h;
	vector<pair<string, vector<int>>> word_list_i;
	vector<pair<string, vector<int>>> word_list_j;
	vector<pair<string, vector<int>>> word_list_k;
	vector<pair<string, vector<int>>> word_list_l;
	vector<pair<string, vector<int>>> word_list_m;
	vector<pair<string, vector<int>>> word_list_n;
	vector<pair<string, vector<int>>> word_list_o;
	vector<pair<string, vector<int>>> word_list_p;
	vector<pair<string, vector<int>>> word_list_q;
	vector<pair<string, vector<int>>> word_list_r;
	vector<pair<string, vector<int>>> word_list_s;
	vector<pair<string, vector<int>>> word_list_t;
	vector<pair<string, vector<int>>> word_list_u;
	vector<pair<string, vector<int>>> word_list_v;
	vector<pair<string, vector<int>>> word_list_w;
	vector<pair<string, vector<int>>> word_list_x;
	vector<pair<string, vector<int>>> word_list_y;
	vector<pair<string, vector<int>>> word_list_z;

	map<string, vector<int>> *list_a = new map<string, vector<int>>;
	map<string, vector<int>> *list_b = new map<string, vector<int>>;
	map<string, vector<int>> *list_c = new map<string, vector<int>>;
	map<string, vector<int>> *list_d = new map<string, vector<int>>;
	map<string, vector<int>> *list_e = new map<string, vector<int>>;
	map<string, vector<int>> *list_f = new map<string, vector<int>>;
	map<string, vector<int>> *list_g = new map<string, vector<int>>;
	map<string, vector<int>> *list_h = new map<string, vector<int>>;
	map<string, vector<int>> *list_i = new map<string, vector<int>>;
	map<string, vector<int>> *list_j = new map<string, vector<int>>;
	map<string, vector<int>> *list_k = new map<string, vector<int>>;
	map<string, vector<int>> *list_l = new map<string, vector<int>>;
	map<string, vector<int>> *list_m = new map<string, vector<int>>;
	map<string, vector<int>> *list_n = new map<string, vector<int>>;
	map<string, vector<int>> *list_o = new map<string, vector<int>>;
	map<string, vector<int>> *list_p = new map<string, vector<int>>;
	map<string, vector<int>> *list_q = new map<string, vector<int>>;
	map<string, vector<int>> *list_r = new map<string, vector<int>>;
	map<string, vector<int>> *list_s = new map<string, vector<int>>;
	map<string, vector<int>> *list_t = new map<string, vector<int>>;
	map<string, vector<int>> *list_u = new map<string, vector<int>>;
	map<string, vector<int>> *list_v = new map<string, vector<int>>;
	map<string, vector<int>> *list_w = new map<string, vector<int>>;
	map<string, vector<int>> *list_x = new map<string, vector<int>>;
	map<string, vector<int>> *list_y = new map<string, vector<int>>;
	map<string, vector<int>> *list_z = new map<string, vector<int>>;

	pthread_mutex_t read_source_mutex, build_list_mutex;
	pthread_barrier_t reduce_wait_barrier, sort_barrier;

	pthread_mutex_t a_mutex, b_mutex, c_mutex, d_mutex, e_mutex, f_mutex,
		g_mutex, h_mutex, i_mutex, j_mutex, k_mutex, l_mutex, m_mutex,
		n_mutex, o_mutex, p_mutex, q_mutex, r_mutex, s_mutex, t_mutex,
		u_mutex, v_mutex, w_mutex, x_mutex, y_mutex, z_mutex;

	pthread_mutex_init(&a_mutex, NULL);
	pthread_mutex_init(&b_mutex, NULL);
	pthread_mutex_init(&c_mutex, NULL);
	pthread_mutex_init(&d_mutex, NULL);
	pthread_mutex_init(&e_mutex, NULL);
	pthread_mutex_init(&f_mutex, NULL);
	pthread_mutex_init(&g_mutex, NULL);
	pthread_mutex_init(&h_mutex, NULL);
	pthread_mutex_init(&i_mutex, NULL);
	pthread_mutex_init(&j_mutex, NULL);
	pthread_mutex_init(&k_mutex, NULL);
	pthread_mutex_init(&l_mutex, NULL);
	pthread_mutex_init(&m_mutex, NULL);
	pthread_mutex_init(&n_mutex, NULL);
	pthread_mutex_init(&o_mutex, NULL);
	pthread_mutex_init(&p_mutex, NULL);
	pthread_mutex_init(&q_mutex, NULL);
	pthread_mutex_init(&r_mutex, NULL);
	pthread_mutex_init(&s_mutex, NULL);
	pthread_mutex_init(&t_mutex, NULL);
	pthread_mutex_init(&u_mutex, NULL);
	pthread_mutex_init(&v_mutex, NULL);
	pthread_mutex_init(&w_mutex, NULL);
	pthread_mutex_init(&x_mutex, NULL);
	pthread_mutex_init(&y_mutex, NULL);
	pthread_mutex_init(&z_mutex, NULL);

	pthread_mutex_init(&read_source_mutex, NULL);
	pthread_mutex_init(&build_list_mutex, NULL);
	pthread_barrier_init(&reduce_wait_barrier, NULL, reducers + mappers);
	pthread_barrier_init(&sort_barrier, NULL, reducers);

	int left_unsorted = 26;
	vector<bool> sorted(26, false);
    
    // Default parameter - all its pointers will point to the common resources the threads require
	thread_params default_param;

	default_param.thread_id = 0;
	default_param.reducers = reducers;
	default_param.mappers = mappers;
	
	default_param.source_file = &input_file;
	default_param.files_left = &file_count;
	default_param.current_file = &file_index;
	
	default_param.word_list = &word_list;

	default_param.sorted = &sorted;
	default_param.left_unsorted = &left_unsorted;

	default_param.word_list_a = &word_list_a;
	default_param.word_list_b = &word_list_b;
	default_param.word_list_c = &word_list_c;
	default_param.word_list_d = &word_list_d;
	default_param.word_list_e = &word_list_e;
	default_param.word_list_f = &word_list_f;
	default_param.word_list_g = &word_list_g;
	default_param.word_list_h = &word_list_h;
	default_param.word_list_i = &word_list_i;
	default_param.word_list_j = &word_list_j;
	default_param.word_list_k = &word_list_k;
	default_param.word_list_l = &word_list_l;
	default_param.word_list_m = &word_list_m;
	default_param.word_list_n = &word_list_n;
	default_param.word_list_o = &word_list_o;
	default_param.word_list_p = &word_list_p;
	default_param.word_list_q = &word_list_q;
	default_param.word_list_r = &word_list_r;
	default_param.word_list_s = &word_list_s;
	default_param.word_list_t = &word_list_t;
	default_param.word_list_u = &word_list_u;
	default_param.word_list_v = &word_list_v;
	default_param.word_list_w = &word_list_w;
	default_param.word_list_x = &word_list_x;
	default_param.word_list_y = &word_list_y;
	default_param.word_list_z = &word_list_z;
	
	default_param.list_a = list_a;
	default_param.list_b = list_b;
	default_param.list_c = list_c;
	default_param.list_d = list_d;
	default_param.list_e = list_e;
	default_param.list_f = list_f;
	default_param.list_g = list_g;
	default_param.list_h = list_h;
	default_param.list_i = list_i;
	default_param.list_j = list_j;
	default_param.list_k = list_k;
	default_param.list_l = list_l;
	default_param.list_m = list_m;
	default_param.list_n = list_n;
	default_param.list_o = list_o;
	default_param.list_p = list_p;
	default_param.list_q = list_q;
	default_param.list_r = list_r;
	default_param.list_s = list_s;
	default_param.list_t = list_t;
	default_param.list_u = list_u;
	default_param.list_v = list_v;
	default_param.list_w = list_w;
	default_param.list_x = list_x;
	default_param.list_y = list_y;
	default_param.list_z = list_z;

	default_param.a_mutex = &a_mutex;
	default_param.b_mutex = &b_mutex;
	default_param.c_mutex = &c_mutex;
	default_param.d_mutex = &d_mutex;
	default_param.e_mutex = &e_mutex;
	default_param.f_mutex = &f_mutex;
	default_param.g_mutex = &g_mutex;
	default_param.h_mutex = &h_mutex;
	default_param.i_mutex = &i_mutex;
	default_param.j_mutex = &j_mutex;
	default_param.k_mutex = &k_mutex;
	default_param.l_mutex = &l_mutex;
	default_param.m_mutex = &m_mutex;
	default_param.n_mutex = &n_mutex;
	default_param.o_mutex = &o_mutex;
	default_param.p_mutex = &p_mutex;
	default_param.q_mutex = &q_mutex;
	default_param.r_mutex = &r_mutex;
	default_param.s_mutex = &s_mutex;
	default_param.t_mutex = &t_mutex;
	default_param.u_mutex = &u_mutex;
	default_param.v_mutex = &v_mutex;
	default_param.w_mutex = &w_mutex;
	default_param.x_mutex = &x_mutex;
	default_param.y_mutex = &y_mutex;
	default_param.z_mutex = &z_mutex;

	default_param.read_source_mutex = &read_source_mutex;
	default_param.build_list_mutex = &build_list_mutex;
	default_param.reduce_wait_barrier = &reduce_wait_barrier;
	default_param.sort_barrier = &sort_barrier;

    // Thread array
	pthread_t threads[reducers + mappers];
	vector<thread_params> vals(reducers + mappers, default_param);

    // Thread creation
	for (int i = 0; i < reducers + mappers; ++i) {
		vals[i].thread_id = i;
		pthread_create(&threads[i], NULL, threadFunc, &vals[i]);
	}

    // Thread join
	for (int i = 0; i < reducers + mappers; ++i) {
		pthread_join(threads[i], NULL);
	}

    // Finishing the work: memory freeing and output writing
	pthread_mutex_destroy(&read_source_mutex);
	pthread_mutex_destroy(&build_list_mutex);
	pthread_barrier_destroy(&reduce_wait_barrier);
	pthread_barrier_destroy(&sort_barrier);
	pthread_mutex_destroy(&a_mutex);
	pthread_mutex_destroy(&b_mutex);
	pthread_mutex_destroy(&c_mutex);
	pthread_mutex_destroy(&d_mutex);
	pthread_mutex_destroy(&e_mutex);
	pthread_mutex_destroy(&f_mutex);
	pthread_mutex_destroy(&g_mutex);
	pthread_mutex_destroy(&h_mutex);
	pthread_mutex_destroy(&i_mutex);
	pthread_mutex_destroy(&j_mutex);
	pthread_mutex_destroy(&k_mutex);
	pthread_mutex_destroy(&l_mutex);
	pthread_mutex_destroy(&m_mutex);
	pthread_mutex_destroy(&n_mutex);
	pthread_mutex_destroy(&o_mutex);
	pthread_mutex_destroy(&p_mutex);
	pthread_mutex_destroy(&q_mutex);
	pthread_mutex_destroy(&r_mutex);
	pthread_mutex_destroy(&s_mutex);
	pthread_mutex_destroy(&t_mutex);
	pthread_mutex_destroy(&u_mutex);
	pthread_mutex_destroy(&v_mutex);
	pthread_mutex_destroy(&w_mutex);
	pthread_mutex_destroy(&x_mutex);
	pthread_mutex_destroy(&y_mutex);
	pthread_mutex_destroy(&z_mutex);

	writeOutput(word_list_a, "a.txt");
	writeOutput(word_list_b, "b.txt");
	writeOutput(word_list_c, "c.txt");
	writeOutput(word_list_d, "d.txt");
	writeOutput(word_list_e, "e.txt");
	writeOutput(word_list_f, "f.txt");
	writeOutput(word_list_g, "g.txt");
	writeOutput(word_list_h, "h.txt");
	writeOutput(word_list_i, "i.txt");
	writeOutput(word_list_j, "j.txt");
	writeOutput(word_list_k, "k.txt");
	writeOutput(word_list_l, "l.txt");
	writeOutput(word_list_m, "m.txt");
	writeOutput(word_list_n, "n.txt");
	writeOutput(word_list_o, "o.txt");
	writeOutput(word_list_p, "p.txt");
	writeOutput(word_list_q, "q.txt");
	writeOutput(word_list_r, "r.txt");
	writeOutput(word_list_s, "s.txt");
	writeOutput(word_list_t, "t.txt");
	writeOutput(word_list_u, "u.txt");
	writeOutput(word_list_v, "v.txt");
	writeOutput(word_list_w, "w.txt");
	writeOutput(word_list_x, "x.txt");
	writeOutput(word_list_y, "y.txt");
	writeOutput(word_list_z, "z.txt");

	delete list_a;
	delete list_b;
	delete list_c;
	delete list_d;
	delete list_e;
	delete list_f;
	delete list_g;
	delete list_h;
	delete list_i;
	delete list_j;
	delete list_k;
	delete list_l;
	delete list_m;
	delete list_n;
	delete list_o;
	delete list_p;
	delete list_q;
	delete list_r;
	delete list_s;
	delete list_t;
	delete list_u;
	delete list_v;
	delete list_w;
	delete list_x;
	delete list_y;
	delete list_z;

	input_file.close();
	return 0;
}

