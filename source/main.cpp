#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <ostream>
#include <fstream>

#define VERSNUM 1
void print_syntax() {
    std::cout <<"KNUXFAN'S MODELING TOOL\n"
    "K N U X F A N\n"
"U \\         U \\\n"
"X   K N U X F A N\n"
"F   U       F   U\n"
"A   X       A   X\n"
"K N F X F A N   F\n"
"  \\ A         \\ A\n"
"    K N U X F A N\n"
"Version ID Number" << VERSNUM << " Built for SDLTetris 2 ALPHA! DO NOT DISTRIBUTE! (unless i distribute it, then its cool)\n"
"REMINDER! This version only produces it's own model format! If you need to produce legacy model formats, download the older version of the tool from git and compile it yourself, lazy!\n"
"Also, this needs to be said: This tool should NOT be used by the general public! If you are a general public, I'm giving you no warranty!\n"
"usage KMFModeler modeltoimport.[fbx,dae,obj...etc] [out]\n";
}
std::string dir = "";
struct vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
};
struct texture {
    std::string path;
    std::string type;
};
struct Material {
    float shininess;
    glm::vec3 diffuseColor;
    glm::vec3 emissionColor;
};
struct mesh {
    std::vector<vertex> vertices;
    std::vector<int> indices;
    std::vector<texture> textures;
    Material mat;

};
std::vector<mesh> meshes;
std::vector<texture> loadtexture(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures.size(); j++)
        {
            if(std::strcmp(textures[j].path.c_str(), std::string(dir+"/"+str.C_Str()).c_str()) == 0)
            {
                std::cout << "pass " << textures[j].path << "\n";
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            textures.push_back({dir+"/"+str.C_Str(),typeName});
        }
    }
    return textures;
}

//this is the chaotic code from the old knuxfans tetriminos...
//i hate it so much... Will be replaced in knuxfans 2 with the model format!
mesh handlemesh(aiMesh* aimesh, const aiScene* scene) {
    std::vector<vertex> vertices; //smells like mesh.h
    std::vector<int> indices;
    std::vector<texture> textures;
    Material mat;
    for(uint i = 0; i < aimesh->mNumVertices; i++) {
        vertex v;
        v.pos.x = aimesh->mVertices[i].x;
        v.pos.y = aimesh->mVertices[i].y;
        v.pos.z = aimesh->mVertices[i].z;
        v.normal.x = aimesh->mNormals[i].x;
        v.normal.y = aimesh->mNormals[i].y;
        v.normal.z = aimesh->mNormals[i].z;
        if(aimesh->mTextureCoords[0]) {
            v.texcoord.x = aimesh->mTextureCoords[0][i].x;
            v.texcoord.y = aimesh->mTextureCoords[0][i].y;
        }
        vertices.push_back(v);
    }
    for(uint i = 0; i < aimesh->mNumFaces; i++) {
        aiFace face = aimesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if(aimesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];
        std::vector<texture> diffuseMaps = loadtexture(material,
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<texture> specularMaps = loadtexture(material,
                                            aiTextureType_SPECULAR, "texture_specular");        
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<texture> emissionMaps = loadtexture(material,
                                            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

        aiColor3D clr (0.f,0.f,0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE,clr);
        mat.diffuseColor = glm::vec3(clr.r,clr.g,clr.b);
        material->Get(AI_MATKEY_COLOR_EMISSIVE,clr);
        mat.emissionColor = glm::vec3(clr.r,clr.g,clr.b);
        material->Get(AI_MATKEY_SHININESS,mat.shininess);


    }

    return {vertices,indices,textures, mat};
}
void handlenode(aiNode* node, const aiScene* scene) {
    for(uint i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(handlemesh(mesh,scene));
    }
    for(uint i = 0; i < node->mNumChildren; i++) {
        handlenode(node->mChildren[i],scene);
    }
}
void loadModel(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate|aiProcess_OptimizeMeshes);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << "\n";
        return;
    }
    dir = path.substr(0,path.find_last_of('/'));

    handlenode(scene->mRootNode, scene);

}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        print_syntax();
        return -1;
    }
    std::string file = argv[1];
    std::string outname = argv[2];

    loadModel(file);
    std::ofstream ofstrem(outname+".kmf", std::ios::out | std::ios::binary);
    int versionNum = VERSNUM;
    ofstrem.write((char *) &versionNum, sizeof(int));

    size_t meshsize = meshes.size();
    ofstrem.write((char *) &meshsize, sizeof(size_t));    
    for(long unsigned i = 0; i < meshes.size(); i++) {
        size_t size = meshes.at(i).vertices.size();
        ofstrem.write((char *) &size, sizeof(size_t));    
        for(vertex v : meshes.at(i).vertices) {
            ofstrem.write((char *) &v.pos.x, sizeof(float));    
            ofstrem.write((char *) &v.pos.y, sizeof(float));    
            ofstrem.write((char *) &v.pos.z, sizeof(float));    
            ofstrem.write((char *) &v.normal.x, sizeof(float));    
            ofstrem.write((char *) &v.normal.y, sizeof(float));    
            ofstrem.write((char *) &v.normal.z, sizeof(float));    
            ofstrem.write((char *) &v.texcoord.x, sizeof(float));    
            ofstrem.write((char *) &v.texcoord.y, sizeof(float));    
        }
        size_t indicesSize = meshes.at(i).indices.size();
        ofstrem.write((char *) &indicesSize, sizeof(size_t));    
        for(int v : meshes.at(i).indices) {
            ofstrem.write((char *) &v, sizeof(int));    
        }

        ofstrem.write((char *) &meshes.at(i).mat.shininess, sizeof(float));    
        ofstrem.write((char *) &meshes.at(i).mat.emissionColor, sizeof(glm::vec3));    
        ofstrem.write((char *) &meshes.at(i).mat.diffuseColor, sizeof(glm::vec3)); 
        size_t texsize = meshes.at(i).textures.size();

        ofstrem.write((char *) &texsize, sizeof(size_t)); 

        for(texture t : meshes.at(i).textures) {
            unsigned long sizeofPath = t.path.length();
            unsigned long sizeOfType = t.type.length();
            std::cout << t.path << " " << t.type << "\n";
            ofstrem.write((char *) &sizeofPath, sizeof(unsigned long));    
            for(char c : t.path) {
                ofstrem.write(&c, sizeof(char));  
            }
            ofstrem.write((char *) &sizeOfType, sizeof(unsigned long));    
            for(char c : t.type) {
                ofstrem.write(&c, sizeof(char));  
            }

            // ofstrem.write((char *) &sizeOfType, sizeof(unsigned long));  
            // ofstrem.write((char *) &t.type, sizeof(t.type));    

        }
    }
    ofstrem.close();

    // for (int i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }
    return 0;
}
