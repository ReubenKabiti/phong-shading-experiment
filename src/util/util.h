#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>
#include <map>
#define MAX_OBJ_LEN 1024 * 1000

namespace Util
{

	std::pair<std::vector<float>, std::vector<uint32_t>> LoadObj(const char* filename)
	{
		std::pair<std::vector<float>, std::vector<uint32_t>> out;
		std::vector<float> vertices;
		std::vector<uint32_t> indices;

		// read the file
		FILE* fp = fopen(filename, "r");
		if (!fp)
			return out;

		char* buff = new char[MAX_OBJ_LEN];

		int numRead = fread(buff, 1, MAX_OBJ_LEN, fp);
		// assuming numRead << MAX_OBJ_LEN
		buff[numRead] = '\n';

		fclose(fp);

		char vBuff[20]; // the string for the vertices
		char iBuff[10]; // the string for the indices

		// read each line
		int lineStart = 0;
		int lineEnd = 0;
		for (int i = 0; i <= numRead; i++)
		{
			// if we have encountered a new line
			// process the line, split it into components then convert those into verts or inds
			if (buff[i] == '\n')
			{
				lineEnd = i;
				// if the line is a vertex
				if (buff[lineStart] == 'v')
				{
					int vStart = lineStart+1;
					int vEnd = vStart;
					for (; vEnd <= lineEnd; ++vEnd)
					{
						if (vEnd > vStart && buff[vEnd] == ' ' || buff[vEnd] == '\n')
						{
							// copy the float string and convert it to a float using atof
							memcpy(vBuff, buff+vStart+1, vEnd - vStart - 1);
							vBuff[vEnd-vStart] = '\0';
							vertices.push_back(atof(vBuff));
							vStart = vEnd;
							memset(vBuff, 0, sizeof(vBuff));
						}
					}

				}
				// if the line is an index
				else if (buff[lineStart] == 'f')
				{
					int iStart = lineStart+1;
					int iEnd = iStart;
					for (; iEnd <= lineEnd; ++iEnd) 
					{
						if (iEnd > iStart && buff[iEnd] == ' ' || buff[iEnd] == '\n')
						{
							// copy the int string and convert it to a int using atoi
							memcpy(iBuff, buff+iStart+1, iEnd - iStart - 1);
							iBuff[iEnd-iStart] = '\0';
							indices.push_back(atoi(iBuff) - 1);
							iStart = iEnd;
							memset(iBuff, 0, sizeof(iBuff));
						}
					}
				}

				lineStart = lineEnd = (i+1);
			}
		}
		out.first = vertices;
		out.second = indices;
		delete[] buff;
		return out;
	}


	std::vector<float> GenerateNormals(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
	{
		std::map<uint32_t, std::vector<glm::vec3>> normals;

		// variables to store vectors temporarily
		glm::vec3 v1;
		glm::vec3 v2;
		glm::vec3 v3;
		// go through all the indices
		for (size_t i = 0; i < indices.size() - 2; i+=3)
		{
			uint32_t ind1 = 3*indices[i];
			uint32_t ind2 = 3*indices[i + 1];
			uint32_t ind3 = 3*indices[i + 2];
			v1.x = vertices[ind1];
			v1.y = vertices[ind1 + 1];
			v1.z = vertices[ind1 + 2];

			v2.x = vertices[ind2];
			v2.y = vertices[ind2 + 1];
			v2.z = vertices[ind2 + 2];

			v3.x = vertices[ind3];
			v3.y = vertices[ind3 + 1];
			v3.z = vertices[ind3 + 2];

			glm::vec3 v12 = v2 - v1;
			glm::vec3 v13 = v3 - v1;

			glm::vec3 normal = glm::cross(v12, v13);
			if (normals.count(ind1/3))
			{
				normals[ind1/3].push_back(normal);
			}
			else
			{
				normals[ind1/3] = std::vector<glm::vec3>();
				normals[ind1/3].push_back(normal);
			}
			if (normals.count(ind2/3))
			{
				normals[ind2/3].push_back(normal);
			}
			else
			{
				normals[ind2/3] = std::vector<glm::vec3>();
				normals[ind2/3].push_back(normal);
			}
			if (normals.count(ind3/3))
			{
				normals[ind3/3].push_back(normal);
			}
			else
			{
				normals[ind3/3] = std::vector<glm::vec3>();
				normals[ind3/3].push_back(normal);
			}
		}


		// go through all the verts and normals

		std::vector<float> out;
		uint32_t ind = 0;
		for (size_t i = 0; i < vertices.size() - 2; i+=3, ind++)
		{
			out.push_back(vertices[i]);
			out.push_back(vertices[i + 1]);
			out.push_back(vertices[i + 2]);
			glm::vec3 normal(0);
			std::vector<glm::vec3> ns = normals[ind];
			for (auto n : ns)
			{
				normal += n;
			}
			out.push_back(normal.x);
			out.push_back(normal.y);
			out.push_back(normal.z);
		}
		return out;
	}
}
