#pragma once

std::vector<glm::vec3> CalculateNormal(const Poly_Data* pData)
{
	std::vector<glm::vec3> vNormal;
	vNormal.resize(pData->vertex_list.size() / 3);		
	glm::vec3 vA, vB, vC;
	glm::vec3 distBA, distCA;	
	glm::vec3 normal;
	
	// Loop through each triangle.
	int nTriangle = pData->tri_list.size()/3;
	for (int i = 0; i < nTriangle; i++)
	{
		int i1, i2, i3;
		i1 = pData->tri_list[i*3];
		i2 = pData->tri_list[i*3 + 1];
		i3 = pData->tri_list[i*3 + 2];

		// Retrieves 3 vertices from the array of vertices.
		vA = glm::vec3(pData->vertex_list[i1 * 3], pData->vertex_list[i1 * 3 + 1], pData->vertex_list[i1 * 3 + 2]);
		vB = glm::vec3(pData->vertex_list[i2 * 3], pData->vertex_list[i2 * 3 + 1], pData->vertex_list[i2 * 3 + 2]);
		vC = glm::vec3(pData->vertex_list[i3 * 3], pData->vertex_list[i3 * 3 + 1], pData->vertex_list[i3 * 3 + 2]);

		// Calculates the vector of the edges, the distance between the vertices.
		distBA = vB - vA;
		distCA = vC - vA;		
		
		// Calculates the face normal to the current triangle.
		normal = glm::cross(distBA, distCA);

		// Averages the new normal vector with the oldest buffered.
		vNormal[i1] = normal + vNormal[i1];
		vNormal[i2] = normal + vNormal[i2];
		vNormal[i3] = normal + vNormal[i3];
	}

	for (unsigned int i = 0; i < vNormal.size();i++){
		vNormal[i] = glm::normalize(vNormal[i]);
	}
	return vNormal;
}