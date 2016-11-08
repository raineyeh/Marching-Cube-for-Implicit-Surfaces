#include "marching_lookup.h"
void find_ambiguous_cases(){
	/* This code fills up the ambiguity look-up table, and is run once to generate the ambiguity table in marching_lookup.h
	It's not used in the actual program. 
	Ambiguity occurs when there is a grid cell face with vertices whose neighboring vertices are of different signs.
	This program works by producing all possible code combinations for those ambiguous cases, and list their 
	  corresponding invert index, as well as listing the 4 vertices that makes up the ambiguous face. */

	// The look-up table corresponding to each cube code. There is ambuguity, 
	//   the entry list the inverted cube code and the cube face with the ambiguity.
	int alt_map_table[256][5];
	for (int i = 0; i < 256; i++)
	for (int j = 0; j < 5; j++)
		alt_map_table[i][j] = -1;

	// For each face of the cube, put the 2 possible ambiguous codes, and find 
	// all other possible configuration of the 4 other vertices.
	for (int fi = 0; fi < 6; fi++){
		bool v_fixed[8] = { false }; // array of 8 indicating whether the vertex code is part of the ambiguous face
		int* f_list = cube_face_vertex_table[fi]; // the 4 vertices that forms this face

		v_fixed[f_list[0]] = true;
		v_fixed[f_list[1]] = true;
		v_fixed[f_list[2]] = true;
		v_fixed[f_list[3]] = true;

		// the 2 possibilities of ambiguous faces
		bool fixed_code_2[][4] = { { 0, 1, 0, 1 }, { 1, 0, 1, 0 } };
		bool occ[8] = { false };

		//for both variations of fixed_code
		for (int fci = 0; fci < 2; fci++){
			//cout << "FCI" << fci << endl;
			bool *fixed_code = fixed_code_2[fci];
			bool var_code_5[5] = { false };
			bool* var_code = var_code_5 + 1;

			//for each variation of var_code... this will iterate until the binary code reaches 10000
			while (!var_code_5[0]){ 
				int j = 0, k = 0;
				//assign the fixed face's codes
				for (int i = 0; i < 4; i++)
					occ[f_list[i]] = fixed_code[i];
				//assign the variable vertices' codes
				for (int i = 0; i < 8; i++){
					if (!v_fixed[i])
						occ[i] = var_code[k++];
				}
				
				int o = 0, n = 0; // calculate the code and the inverse code
				for (int i = 0; i < 8; i++){
					if (occ[i])
						o += two_to_the[i];
					else
						n += two_to_the[i];
				}
				for (int i = 0; i < 8; i++)
					cout << occ[i] << ",";
				cout << " case " << o << "-> " << n << "";
				
				//assign the look-up table for ambiguous cases. 
				//first one is the look-up index, next 4 are the vertices of the ambiguous faces. 
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
	//Print everything out
	for (int i = 0; i < 256; i++){
		cout << "{ " << alt_map_table[i][0];
		for (int j = 1; j < 5; j++){
			cout << ", " << alt_map_table[i][j];
		}
		cout << "}, //" << i << endl;
	}
}

