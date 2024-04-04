#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

#define VERSNUM 1.0
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
"Version " << VERSNUM << " Built for SDLTetris 2 ALPHA! DO NOT DISTRIBUTE! (unless i distribute it, then its cool)\n"
"usage KMFModeler modeltoimport.[fbx,dae,obj...etc] [-A atlas(true/false)] [out]";
}
std::string dir = "";
struct vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
};
struct Material {
    float shininess;
    glm::vec3 diffuseColor;
    glm::vec3 emissionColor;
};
struct mesh {
    std::vector<vertex> vertices;
    std::vector<int> indices;
    std::vector<std::string> textures;
    Material mat;

};
std::vector<mesh> meshes;
std::vector<std::string> loadtexture(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<std::string> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures.size(); j++)
        {
            if(std::strcmp(textures[j].c_str(), std::string(dir+"/"+str.C_Str()).c_str()) == 0)
            {
                std::cout << "pass " << textures[j] << "\n";
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            textures.push_back(dir+"/"+str.C_Str());
        }
    }
    return textures;
}

//this is the chaotic code from the old knuxfans tetriminos...
//i hate it so much... Will be replaced in knuxfans 2 with the model format!
mesh handlemesh(aiMesh* aimesh, const aiScene* scene) {
    std::vector<vertex> vertices; //smells like mesh.h
    std::vector<int> indices;
    std::vector<std::string> textures;
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
        std::vector<std::string> diffuseMaps = loadtexture(material,
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<std::string> specularMaps = loadtexture(material,
                                            aiTextureType_SPECULAR, "texture_specular");        
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<std::string> emissionMaps = loadtexture(material,
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
    loadModel(file);
    std::cout << meshes.at(0).textures.at(0) << "\n";
    FILE *ff = fopen("file.txt","a+"); 
    if(!ff) {
            fprintf(stderr,"error opening file...exiting\n");
            exit(1);
    }
    fprintf(ff,"float vec[] = {\n"); // write

    for(vertex v : meshes.at(0).vertices) {
        std::cout << v.pos.x << " " << v.pos.y << " " << v.pos.z << "\n";
        fprintf(ff,"%f,%f,%f,%f,%f,\n", v.pos.x,v.pos.y,v.pos.z,v.texcoord.x,v.texcoord.y); // write
    }
    fclose(ff); // close file

    // for (int i = 0; i < argc; i++) {
    //     printf("%s\n", argv[i]);
    // }
    return 0;
}