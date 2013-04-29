
#include "../sc_cpu/Model.h"
#include "FittingCylinder.h"


void FittingPrimitive::DualGraphNode::expandByFaces(Model* mesh, vector<int>& faceindexlist){
			for(int i=0;i<faceindexlist.size();i++){
				int index = faceindexlist[i];
				double index_area = mesh->ComputeFaceArea(index);// todo
				// faces and tot area
				this->faces.push_back(index);//
				this->totArea += index_area;//

				// vertices and edges
				//int b = index * 3;
				int c1 = mesh->mesh_->faces[index][0];
				int c2 = mesh->mesh_->faces[index][1];
				int c3 = mesh->mesh_->faces[index][2];
				this->vertices.push_back(c1);//
				this->vertices.push_back(c2);//
				this->vertices.push_back(c3);//

				edges.emplace(EdgeRecord(c1, c2));
				edges.emplace(EdgeRecord(c2, c3));
				edges.emplace(EdgeRecord(c3, c1)); // need operator = for EdgeRecords ???

				// init center and cov_v
				Vector3d v1 = Vector3d(mesh->mesh_->vertices,c1);
				Vector3d v2 = Vector3d(mesh->mesh_->vertices,c2);
				Vector3d v3 = Vector3d(mesh->mesh_->vertices,c3);
				this->center += ((v1 + v2 + v3) / 3) * index_area;
				this->cov_v = this->cov_v + (SymMatrix3d(v1) + SymMatrix3d(v2) + SymMatrix3d(v3)) * (index_area / 3.0);

				// init cov_c
				Vector3d normal1 = Vector3d(mesh->facenormals_, index); // size of face normal/ faces ???
				for (int i=0; i<3;i++)
				{
					int adj = mesh->mesh_->across_edge[index][i]; // the index of three neighbor face, may need check?? number of neighbor

					Vector3d normal2 = Vector3d(mesh->facenormals_, adj );
					Vector3d commonEdge;
					std::unordered_set<int> vSet;
					vSet.insert(mesh->mesh_->faces[adj][0]); //vertex number of neighbor face
					vSet.insert(mesh->mesh_->faces[adj][1]);
					vSet.insert(mesh->mesh_->faces[adj][2]);
					if (((vSet.find(c1)) != vSet.end()) && (vSet.find(c2)) != vSet.end())// look for common edge, may be optimized ? by cross_edge[]
					{
						commonEdge = v2 - v1;
					}
					if (((vSet.find(c2)) != vSet.end()) && (vSet.find(c3)) != vSet.end())
					{
						commonEdge = v3 - v2;
					}
					if (((vSet.find(c3)) != vSet.end()) && (vSet.find(c1)) != vSet.end())
					{
						commonEdge = v1 - v3;
					}
					double len = commonEdge.L2Norm();
					Vector3d en1= commonEdge/len;
					double w = -(normal1.Cross(normal2)).Dot(en1);
					this->cov_c = this->cov_c + SymMatrix3d(en1) * (len * w); // update cov_c normal variation cov matrix
				}
				/*
				for (int i = 0; i < 3; i++)
				{
				int adj = mesh.AdjFF[index][i];
				int d1 = mesh.FaceIndex[b + i];
				int d2 = mesh.FaceIndex[b + (i + 1) % 3];
				Vector3d u1 = new Vector3d(mesh.VertexPos, d1 * 3);
				Vector3d u2 = new Vector3d(mesh.VertexPos, d2 * 3);
				Vector3d e = u1 - u2;
				Vector3d en = e.Normalize();
				double len = e.Length();
				Vector3d normal2 = new Vector3d(mesh.FaceNormal, adj * 3);
				double w = -(normal1.Cross(normal2)).Dot(en);
				this.cov_c += new SymMatrix3d(en) * (len * w);
				}
				*/

				// init ATA and ATb
				/*Vector4d w1 = Vector4d(  v1 *2, 1.0);
				Vector4d w2 = Vector4d(v2 * 2, 1.0);
				Vector4d w3 = Vector4d(v3 * 2, 1.0);
				double sqArea = totArea * totArea;
				this->ATA = ( SymMatrix4d(w1) +  SymMatrix4d(w2) +  SymMatrix4d(w3)) * sqArea;
				this->ATb = (w1 * v1.Dot(v1) + w2 * v2.Dot(v2) + w3 * v3.Dot(v3)) * sqArea;*/
			}		
			return;
		}

FittingPrimitive::FittingPrimitive(Model* mesh, vector<int> * faceindexlist){
		init();
		m=mesh;
		node.expandByFaces(mesh, *faceindexlist);
		
}

FittingCylinder::FittingCylinder (Model* mesh, vector<int> * faceindexlist):FittingPrimitive(mesh, faceindexlist){
	}

FittingSphere::FittingSphere(Model* mesh, vector<int> * faceindexlist):FittingPrimitive(mesh, faceindexlist){
	}