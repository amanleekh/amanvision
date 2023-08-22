#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <string>
#include <QMatrix4x4>
#include <vector>
#include <QFile>
#include <QSharedPointer>
#include <QDir>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

struct MaterialInfo
{
    QString Name;
    QVector3D Ambient;      // ambient lighting vert
    QVector3D Diffuse;      // diffuse lighting
    QVector3D Specular;     // speculat lighting
    float Shininess;

    bool isTexture;
    QString textureName;
	//unsigned int imageIdx;	// devIl image idx
	unsigned int textureIdx;	// opengl texture idx
};

struct LightInfo
{
    QVector4D Position;
    QVector3D Intensity;
};

struct Mesh
{
    QString name;
    unsigned int indexCount;
    unsigned int indexOffset;
    QSharedPointer<MaterialInfo> material;

    unsigned int numUVChannels;
    bool hasTangentsAndBitangents;
    bool hasNormals;
    bool hasBones;
};

struct Node
{
    QString name;

    // original transform matrix belong to model. 
    // if trans to unit coord, this matrix will multi a trans matrix
    QMatrix4x4 transformation;  
    QVector<QSharedPointer<Mesh> > meshes;
    QVector<Node> nodes;
};

class ModelLoader
{
public:
    enum PathType {
        RelativePath,
        AbsolutePath
    };

    ModelLoader(bool transformToUnitCoordinates = true);
    bool Load(QString filePath, PathType pathType);
    int postLoadGLTexture();
    void getBufferData( QVector<float> **vertices, QVector<float> **normals,
                        QVector<unsigned int> **indices);

    void getTextureData( QVector<QVector<float> > **textureUV,                   // For texture mapping
                         QVector<float> **tangents, QVector<float> **bitangents);// For normal mapping

    QSharedPointer<Node> getNodeData() { return m_rootNode; }
    aiScene *getScene() {return m_scene;}

    // Texture information
    // a vert could have multi texture. chnnel means how many vert tex, component means a center chn have how many tex
    int numUVChannels() { return m_textureUV.size(); }
    int numUVComponents(int channel) { return m_numUVComponents.at(channel); }

    void transformToCoordinates(float scale_val, float dst_w, float dst_h, float dst_z);
private:
    QSharedPointer<MaterialInfo> processMaterial(aiMaterial *mater);
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode, int &nodeIndex);

    // change vert coord in [-1, 1]
    void transformToUnitCoordinates();
    void findObjectDimensions(Node *node, QMatrix4x4 transformation, QVector3D &minDimension, QVector3D &maxDimension);

	QString m_filePath;
	
    aiScene* m_scene;
    QMap<QString, unsigned int> m_textureIdMap;
    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<unsigned int> m_indices;

    QVector<QVector<float/*texture mapping coords*/> > m_textureUV; // m_textureUV[uvChannelIndex] is vector of texture mapping coords
                                                                    // m_textureUV[uvChannelIndex][ii+n] == if(n==0&&numCmpnts>0) U; if(n==0&&numCmpnts>0) V; if(n==0&&numCmpnts>0) W.
    QVector<float> m_tangents;
    QVector<float> m_bitangents;
    QVector<unsigned int /*num components*/> m_numUVComponents; // m_numUVComponents[uvChannelIndex]

    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;
    bool m_transformToUnitCoordinates;

    QVector3D m_min_coord;
    QVector3D m_max_coord;
};

#endif // MODELLOADER_H
