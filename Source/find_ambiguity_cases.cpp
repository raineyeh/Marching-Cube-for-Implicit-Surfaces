#include "marching_lookup.h"
void find_ambiguous_cases(){
	/* This code fills up the ambiguity look-up table, and is run once.
	It's not used in the actual program. But add to the code base just for documentation's sake. */

	int alt_map_table[256][5];
	for (int i = 0; i < 256; i++)
	for (int j = 0; j < 5; j++)
		alt_map_table[i][j] = -1;

	for (int fi = 0; fi < 6; fi++){
		bool v_fixed[8] = { false };
		int* f_list = cube_face_vertex_table[fi];

		v_fixed[f_list[0]] = 1;
		v_fixed[f_list[1]] = 1;
		v_fixed[f_list[2]] = 1;
		v_fixed[f_list[3]] = 1;

		//1010
		bool fixed_code_2[][4] = { { 0, 1, 0, 1 }, { 1, 0, 1, 0 } };
		bool occ[8] = { false };

		//for both variations of fixed_code
		for (int fci = 0; fci < 2; fci++){
			//cout << "FCI" << fci << endl;
			bool *fixed_code = fixed_code_2[fci];
			bool var_code_5[5] = { false };
			bool* var_code = var_code_5 + 1;

			//for each variation of var_code
			while (!var_code_5[0]){
				int j = 0, k = 0;
				for (int i = 0; i < 8; i++){
					if (!v_fixed[i])
						occ[i] = var_code[k++];
				}
				for (int i = 0; i < 4; i++)
					occ[f_list[i]] = fixed_code[i];

				int o = 0, n = 0;
				for (int i = 0; i < 8; i++){
					if (occ[i])
						o += two_to_the[i];
					else
						n += two_to_the[i];
				}
				for (int i = 0; i < 8; i++)
					cout << occ[i] << ",";
				cout << " case " << o << "-> " << n << "";

				alt_map_table[o][0] = n;
				for (int i = 0; i < 4; i++)
					alt_map_table[o][i + 1] = f_list[i];

				//increase varcode by 1
				for (int i = 4; i >= 0; i--){
					if (var_code_5[i]){
						var_code_5[i] = false;
					}
					else{
						var_code_5[i] = true;
						break;
					}
				}
				/*for (int i = 0; i < 5; i++)
				cout << var_code_5[i] << " ";
				*/
				cout << endl;
			}
		}
	}
	cout << endl;
	for (int i = 0; i < 256; i++){
		cout << "{ " << alt_map_table[i][0];
		for (int j = 1; j < 5; j++){
			cout << ", " << alt_map_table[i][j];
		}
		cout << "}, //" << i << endl;
	}
}

