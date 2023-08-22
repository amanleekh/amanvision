
#include "common_def.h"
#include "modelloader.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <IL/il.h>
#include <QDebug>
#include <limits>

#include "GLES2/gl2.h"


#define DEBUGOUTPUT_NORMALS(nodeIndex) (false)//( QList<int>{1}.contains(nodeIndex) )//(false)

ModelLoader::ModelLoader(bool transformToUnitCoordinates) :
    m_transformToUnitCoordinates(transformToUnitCoordinates)
{
    double amin = std::numeric_limits<double>::max();
    double amax = std::numeric_limits<double>::min();
    m_min_coord = QVector3D(amin,amin,amin);
    m_max_coord = QVector3D(amax,amax,amax);

    m_scene = NULL;
}

// look for file using relative path
QString findFile(QString relativeFilePath, int scanDepth)
{
    QString str = relativeFilePath;
    for (int ii = -1; ii < scanDepth; ++ii)
    {
        if (QFile::exists(str))
        {
            return str;
        }
        str.prepend("../");
    }
    
    return "";
}

bool ModelLoader::Load(QString filePath, PathType pathType)
{
    QString l_filePath;
    if (pathType == RelativePath)
    {
        l_filePath = findFile(filePath, 5);
    }
    else
    {
        l_filePath = filePath;
    }
	m_filePath = l_filePath;

    Assimp::Importer importer;

    //const aiScene* scene = importer.ReadFile(
    //    l_filePath.toStdString(),
    //    aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
    //    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
    //    aiProcess_SortByPType);

    const aiScene* scene = importer.ReadFile(
        l_filePath.toStdString(),
        aiProcess_GenSmoothNormals | aiProcess_Triangulate);
    if (!scene)
    {
        AVM_ERR("ModelLoader::Load faild. msg %s\n", importer.GetErrorString());
        return false;
    }
    m_scene = (aiScene *)scene;

    if (scene->HasMaterials())
    {
        for (unsigned int ii = 0; ii < scene->mNumMaterials; ++ii)
        {
            QSharedPointer<MaterialInfo> mater 
                = processMaterial(scene->mMaterials[ii]);
            m_materials.push_back(mater);
        }
    }

    if (scene->HasMeshes())
    {
        for (unsigned int ii = 0; ii < scene->mNumMeshes; ++ii)
        {
            m_meshes.push_back(processMesh(scene->mMeshes[ii]));
        }
    }
    else
    {
        AVM_ERR("ModelLoader::Load. no meshes found\n");
        return false;
    }

    if (scene->HasLights())
    {
        AVM_LOG("ModelLoader::Load. has lights\n");
    }

    if (scene->mRootNode != NULL)
    {
        int nodeIndex = 0;
        Node *rootNode = new Node;
        processNode(scene, scene->mRootNode, 0, *rootNode, nodeIndex);
        m_rootNode.reset(rootNode);
    }
    else
    {
        AVM_ERR("ModelLoader::Load. error loading model\n");
        return false;
    }

    findObjectDimensions(m_rootNode.data(), QMatrix4x4(), m_min_coord, m_max_coord);
    AVM_LOG("ModelLoader::Load '%s'. min_x=%.2f, min_y=%.2f, min_z=%.2f\n",
        filePath.toStdString().c_str(), m_min_coord.x(), m_min_coord.y(), m_min_coord.z());
    AVM_LOG("ModelLoader::Load '%s'. max_x=%.2f, max_y=%.2f, max_z=%.2f\n",
        filePath.toStdString().c_str(), m_max_coord.x(), m_max_coord.y(), m_max_coord.z());
    
    // This will transform the model to unit coordinates, so a model of any size or shape will fit on screen
    if (m_transformToUnitCoordinates)
    {
        transformToUnitCoordinates();
    }

    return true;
}

void ModelLoader::getBufferData( QVector<float> **vertices, QVector<float> **normals, QVector<unsigned int> **indices)
{
    if(vertices != 0)
    {
        *vertices = &m_vertices;
    }

    if(normals != 0)
    {
        *normals = &m_normals;
    }
    
    if(indices != 0)
    {
        *indices = &m_indices;
    }
}


void ModelLoader::getTextureData(QVector<QVector<float> > **textureUV, QVector<float> **tangents, QVector<float> **bitangents)
{
    if(textureUV != 0)
    {
        *textureUV = &m_textureUV;
    }

    if(tangents != 0)
    {
        *tangents = &m_tangents;
    }

    if(bitangents != 0)
    {
        *bitangents = &m_bitangents;
    }
}


int ModelLoader::postLoadGLTexture()
{
    QString path_pre = m_filePath.section('/', 0, -2) + "/";

#if 0
    for (int i = 0; i < m_scene->mNumMaterials; i++)
    {
        int texIx = 0;
        aiString path;	// filename

        aiReturn texFound = m_scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIx, &path);
        while (texFound == AI_SUCCESS) 
		{
            //fill map with textures, OpenGL image ids set to 0
            m_textureIdMap[path.data] = 0;
            // more textures?
            texIx++;
            texFound = m_scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIx, &path);
        }
    }
#endif	
	int numTextures = m_textureIdMap.size();
	if (numTextures <= 0)
	{
		return 0;
	}

	// init il, multi init is ok
	ilInit();
	
	/* create and fill array with DevIL texture ids */
	ILuint* imageIds = new ILuint[numTextures];
	ilGenImages(numTextures, imageIds); 

	/* create and fill array with GL texture ids */
	GLuint *textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds); /* Texture name generation */

	/* get iterator */
    QMap<QString, unsigned int>::iterator itr = m_textureIdMap.begin();
    for (unsigned int i = 0; itr != m_textureIdMap.end(); ++i, ++itr)
	{
		//save IL image ID
        QString filename = itr.key();
        filename.replace(QString("\\"), QString("/"));
        filename = path_pre + filename;
        itr.value() = textureIds[i];	  // save texture id for filename in map
		
		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 
        ILboolean success = ilLoadImage((ILstring)filename.toStdString().c_str());
		if (success) 
		{
			/* Convert image to RGBA */
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 

			/* Create and load textures to OpenGL */
			glBindTexture(GL_TEXTURE_2D, textureIds[i]); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
				ilGetData());
		}
		else 
		{
            AVM_ERR("ModelLoader::postLoadGLTexture. Couldn't load Image: %s\n", filename.toStdString().c_str());
        }
	}

	// set materialinfo
	QVector<QSharedPointer<MaterialInfo> >::iterator materinfo_iter = m_materials.begin();
	for (unsigned int i = 0; materinfo_iter != m_materials.end(); ++i, ++materinfo_iter)
	{
        if (materinfo_iter->data()->isTexture)
		{
            materinfo_iter->data()->textureIdx = m_textureIdMap[materinfo_iter->data()->textureName];
		}
	}

	/* Because we have already copied image data into texture data
	we can release memory used by image. */
	ilDeleteImages(numTextures, imageIds); 

    delete [] imageIds;
	delete [] textureIds;
	
    return 0;
}


QSharedPointer<MaterialInfo> ModelLoader::processMaterial(aiMaterial *material)
{
    QSharedPointer<MaterialInfo> mater(new MaterialInfo);

    aiString mname;
    material->Get(AI_MATKEY_NAME, mname);
    if (mname.length > 0)
    {
        mater->Name = mname.C_Str();
    }
    mater->isTexture = false;

    int shadingModel;
    material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

    if ((shadingModel != aiShadingMode_Phong) 
        && (shadingModel != aiShadingMode_Gouraud))
    {
        AVM_LOG("ModelLoader::processMaterial. this mesh's shading model is not implemented in this loader, setting to default material\n");
        mater->Name = "DefaultMaterial";
    }
    else
    {
        aiColor3D dif (0.f,0.f,0.f);
        aiColor3D amb (0.f,0.f,0.f);
        aiColor3D spec (0.f,0.f,0.f);
        float shine = 0.0;

        material->Get(AI_MATKEY_COLOR_AMBIENT, amb);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, dif); //->Get(<material-key>,<where-to-store>))
        material->Get(AI_MATKEY_COLOR_SPECULAR, spec);
        material->Get(AI_MATKEY_SHININESS, shine);

        mater->Ambient = QVector3D(amb.r, amb.g, amb.b);
        mater->Diffuse = QVector3D(dif.r, dif.g, dif.b);
        mater->Specular = QVector3D(spec.r, spec.g, spec.b);
        mater->Shininess = shine;

        mater->Ambient *= .2f;
        if (mater->Shininess == 0.0) 
        {
            mater->Shininess = 30;
        }

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
        {
            //AVM_LOG("ModelLoader::processMaterial. Diffuse Texture(s) Found %d for for Material %s\n",
            //    material->GetTextureCount(aiTextureType_DIFFUSE), mater->Name.toStdString().c_str());
            aiString texPath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, 
                &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
            {
                QString texturePath = texPath.data;
                mater->isTexture = true;
                mater->textureName = texturePath;

				m_textureIdMap[texPath.data] = 0;
                //AVM_LOG("ModelLoader::processMaterial. Texture path %s\n", texturePath.toStdString().c_str());
            }
            else
            {
                AVM_LOG("ModelLoader::processMaterial. Failed to get texture for material\n");
            }
        }
    }

    return mater;
}

QSharedPointer<Mesh> ModelLoader::processMesh(aiMesh *mesh)
{
    QSharedPointer<Mesh> newMesh(new Mesh);
    newMesh->name = mesh->mName.length != 0 ? mesh->mName.C_Str() : "";
    newMesh->indexOffset = m_indices.size();
    unsigned int indexCountBefore = m_indices.size();
    int vertindexoffset = m_vertices.size()/3;

    newMesh->numUVChannels = mesh->GetNumUVChannels();
    newMesh->hasTangentsAndBitangents = mesh->HasTangentsAndBitangents();
    newMesh->hasNormals = mesh->HasNormals();
    newMesh->hasBones = mesh->HasBones();

    // Get Vertices
    if (mesh->mNumVertices > 0)
    {
        for (uint ii=0; ii<mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mVertices[ii];

            m_vertices.push_back(vec.x);
            m_vertices.push_back(vec.y);
            m_vertices.push_back(vec.z);
        }
    }

    // Get Normals
    if (mesh->HasNormals())
    {
        m_normals.resize(m_vertices.size());

        int nind = vertindexoffset * 3;

        for (uint ii = 0; ii < mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mNormals[ii];
            m_normals[nind] = vec.x;
            m_normals[nind+1] = vec.y;
            m_normals[nind+2] = vec.z;
            nind += 3;
        };
    }

    // Get Texture coordinates
    if (mesh->GetNumUVChannels() > 0)
    {
        if ((unsigned int)m_textureUV.size() < mesh->GetNumUVChannels()) // Caution, assumes all meshes in this model have same number of uv channels
        {
            m_textureUV.resize(mesh->GetNumUVChannels());
            m_numUVComponents.resize(mesh->GetNumUVChannels());
        }

        for (unsigned int mchanInd = 0; mchanInd < mesh->GetNumUVChannels(); ++mchanInd)
        {
            Q_ASSERT(mesh->mNumUVComponents[mchanInd] == 2 && "Error: Texture Mapping Component Count must be 2. Others not supported");

            m_numUVComponents[mchanInd] = mesh->mNumUVComponents[mchanInd];
            m_textureUV[mchanInd].resize((m_vertices.size()/3)*2);

            int uvind = vertindexoffset * m_numUVComponents[mchanInd];

            for (uint iind = 0; iind < mesh->mNumVertices; ++iind)
            {
                // U
                m_textureUV[mchanInd][uvind] = mesh->mTextureCoords[mchanInd][iind].x;
                if (mesh->mNumUVComponents[mchanInd] > 1)
                {
                    // V
                    m_textureUV[mchanInd][uvind+1] = mesh->mTextureCoords[mchanInd][iind].y;
                    if (mesh->mNumUVComponents[mchanInd] > 2)
                    {
                        // W
                        m_textureUV[mchanInd][uvind+2] = mesh->mTextureCoords[mchanInd][iind].z;
                    }
                }
                uvind += m_numUVComponents[mchanInd];
            }
        }
    }

    // Get Tangents and bitangents
    if (mesh->HasTangentsAndBitangents())
    {
        m_tangents.resize(m_vertices.size());
        m_bitangents.resize(m_vertices.size());

        int tind = vertindexoffset * 3;

        for (uint ii = 0; ii < mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mTangents[ii];
            m_tangents[tind] = vec.x;
            m_tangents[tind+1] = vec.y;
            m_tangents[tind+2] = vec.z;

            aiVector3D &vec2 = mesh->mBitangents[ii];
            m_bitangents[tind] = vec2.x;
            m_bitangents[tind+1] = vec2.y;
            m_bitangents[tind+2] = vec2.z;

            tind += 3;
        };
    }

    // Get mesh indexes
    for (uint t = 0; t < mesh->mNumFaces; ++t)
    {
        aiFace* face = &mesh->mFaces[t];
        if (face->mNumIndices != 3)
        {
            AVM_ERR("ModelLoader::processMaterial. Mesh face %d/%d with %d not exactly 3 indices, ignoring this primitive\n",
                t+1, mesh->mNumFaces, face->mNumIndices);
            continue;
        }

        m_indices.push_back(face->mIndices[0]+vertindexoffset);
        m_indices.push_back(face->mIndices[1]+vertindexoffset);
        m_indices.push_back(face->mIndices[2]+vertindexoffset);
    }

    newMesh->indexCount = m_indices.size() - indexCountBefore;
    newMesh->material = m_materials.at(mesh->mMaterialIndex);

    return newMesh;
}

void ModelLoader::processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode, int &nodeIndex)
{
    //static int nodeIndex = 0;

    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : "";
    //printf("--name=%s\n", newNode.name.toStdString().c_str());

    newNode.transformation = QMatrix4x4(node->mTransformation[0]);

    newNode.meshes.resize(node->mNumMeshes);
    for (uint imesh = 0; imesh < node->mNumMeshes; ++imesh)
    {
        QSharedPointer<Mesh> mesh = m_meshes[node->mMeshes[imesh]];
        newNode.meshes[imesh] = mesh;

        if (DEBUGOUTPUT_NORMALS(nodeIndex)) 
        {
            AVM_LOG("ModelLoader::processNode. Start Print Normals for nodeIndex:%d, MeshIndex:%d, NodeName=%s\n",
                nodeIndex, imesh, newNode.name.toStdString().c_str());
            for (quint32 ii=newNode.meshes[imesh]->indexOffset;
                 ii<newNode.meshes[imesh]->indexCount+newNode.meshes[imesh]->indexOffset;
                 ++ii) 
            {
                int ind = m_indices[ii] * 3;
                AVM_LOG("ModelLoader::processNode. %d %d %d\n", m_normals[ind], m_normals[ind]+1, m_normals[ind]+2);
            }
            AVM_LOG("ModelLoader::processNode. End Print Normals for nodeIndex:%d, , MeshIndex:%d, NodeName=%s\n",
                nodeIndex, imesh, newNode.name.toStdString().c_str());
        }
    }

#if 0
    qDebug() << "NodeName" << newNode.name;
    qDebug() << "  NodeIndex" << nodeIndex;
    qDebug() << "  NumChildren" << node->mNumChildren;
    qDebug() << "  NumMeshes" << newNode.meshes.size();
    for (int ii = 0; ii < newNode.meshes.size(); ++ii) 
    {
        qDebug() << "    MeshName" << newNode.meshes[ii]->name;
        qDebug() << "    MaterialName" << newNode.meshes[ii]->material->Name;
        qDebug() << "    MeshVertices" << newNode.meshes[ii]->indexCount;
        qDebug() << "    numUVChannels" << newNode.meshes[ii]->numUVChannels;
        qDebug() << "    hasTangAndBit" << newNode.meshes[ii]->hasTangentsAndBitangents;
        qDebug() << "    hasNormals" << newNode.meshes[ii]->hasNormals;
        qDebug() << "    hasBones" << newNode.meshes[ii]->hasBones;
    }
#endif

    ++nodeIndex;

    for (uint ich = 0; ich < node->mNumChildren; ++ich)
    {
        newNode.nodes.push_back(Node());
        processNode(scene, node->mChildren[ich], parentNode, newNode.nodes[ich], nodeIndex);
    }
}

void ModelLoader::transformToUnitCoordinates()
{
    QVector3D minDimension = m_min_coord;
    QVector3D maxDimension = m_max_coord;

    // Calculate scale and translation needed to center and fit on screen
    float dist = qMax(maxDimension.x() - minDimension.x(), qMax(maxDimension.y()-minDimension.y(), maxDimension.z() - minDimension.z()));
    float sc = 1.0 / dist;
    QVector3D center = (maxDimension - minDimension)/2;
    QVector3D trans = -(maxDimension - center);

    // Apply the scale and translation to a matrix
    QMatrix4x4 transformation;
    transformation.setToIdentity();
    transformation.scale(sc);
    transformation.translate(trans);

    // Multiply the transformation to the root node transformation matrix
    m_rootNode.data()->transformation = transformation * m_rootNode.data()->transformation;
}


void ModelLoader::transformToCoordinates(float scale_val, float dst_w, float dst_h, float dst_z)
{
    if ((dst_w <= 0.0f) && (dst_h <= 0.0f) && (dst_z <= 0.0f) && (scale_val <= 0.0f))
    {
        return;
    }

    float sc = 1.0f;
    QVector3D minDimension = m_min_coord;
    QVector3D maxDimension = m_max_coord;
    if (scale_val > 0.0f)
    {
        sc = scale_val;
        AVM_LOG("ModelLoader::transformToCoordinates. sc=%f\n", sc);
    }
    else
    {
        float scx = dst_w / (maxDimension.x() - minDimension.x());
        float scy = dst_h / (maxDimension.y() - minDimension.y());
        float scz = dst_z / (maxDimension.z() - minDimension.z());
        sc = qMax(scx, qMax(scy, scz));
        AVM_LOG("ModelLoader::transformToCoordinates. scx=%f, scy=%f, scz=%f, sc=%f\n", scx, scy, scz, sc);
    }
    QVector3D center = (maxDimension - minDimension) / 2;
    QVector3D trans = -(maxDimension - center);

    // Apply the scale and translation to a matrix
    QMatrix4x4 transformation;
    transformation.setToIdentity();
    transformation.scale(sc);
    transformation.translate(trans);

    // Multiply the transformation to the root node transformation matrix
    m_rootNode.data()->transformation = transformation * m_rootNode.data()->transformation;
}

void ModelLoader::findObjectDimensions(Node *node, QMatrix4x4 transformation, QVector3D &minDimension, QVector3D &maxDimension)
{
    transformation *= node->transformation;

    for (int ii = 0; ii < node->meshes.size(); ++ii) 
    {
        int start = node->meshes[ii]->indexOffset;
        int end = start + node->meshes[ii]->indexCount;
        for (int ii = start; ii < end; ++ii)
        {
            int ind = m_indices[ii] * 3;
            QVector4D vec(m_vertices[ind], m_vertices[ind+1], m_vertices[ind+2], 1.0);
            vec = transformation * vec;

            if (vec.x() < minDimension.x())
            {
                minDimension.setX(vec.x());
            }
            if (vec.y() < minDimension.y())
            {
                minDimension.setY(vec.y());
            }
            if (vec.z() < minDimension.z())
            {
                minDimension.setZ(vec.z());
            }
            if (vec.x() > maxDimension.x())
            {
                maxDimension.setX(vec.x());
            }
            if (vec.y() > maxDimension.y())
            {
                maxDimension.setY(vec.y());
            }
            if (vec.z() > maxDimension.z())
            {
                maxDimension.setZ(vec.z());
            }
        }
    }

    for (int ii = 0; ii < node->nodes.size(); ++ii) 
    {
        findObjectDimensions(&(node->nodes[ii]), transformation, minDimension, maxDimension);
    }
}
