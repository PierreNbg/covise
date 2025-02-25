/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAppearance.cpp
//

#include "VrmlNodeAppearance.h"
#include "VrmlNodeType.h"

#include "Viewer.h"
#include "VrmlNodeMaterial.h"
#include "VrmlNodeTexture.h"
#include "VrmlScene.h"
#include "Doc.h"

using namespace vrml;

// Define the built in VrmlNodeType:: "Appearance" fields
void VrmlNodeAppearance::initFields(VrmlNodeAppearance *node, vrml::VrmlNodeType *t)
{
    initFieldsHelper(node, t,
                     exposedField("material", node->d_material));
    for (size_t i = 0; i < node->MAX_TEXTURES; i++)
    {
        std::string suffix = i == 0 ? std::string() : std::to_string(i + 1);
        initFieldsHelper(node, t,
                         exposedField("texture" + suffix, node->d_textures[i],
                                      [node, i](const VrmlSFNode *)
                                      {
                                          if (i >= node->d_numTextures)
                                              node->d_numTextures = i + 1;
                                      }),
                         exposedField("textureTransform" + suffix, node->d_textureTransforms[i]));
    }
    VrmlNodeChild::initFields(node, t);
}

VrmlNodeAppearance::VrmlNodeAppearance(VrmlScene *scene)
    : VrmlNodeChild(scene, typeName())
{
}

void VrmlNodeAppearance::cloneChildren(VrmlNamespace *ns)
{
    // Replace references with clones
    if (d_material.get())
    {
        d_material.set(d_material.get()->clone(ns));
        d_material.get()->parentList.push_back(this);
    }
    for (size_t i = 0; i < d_textures.size(); i++)
    {
        if (d_textures[i].get())
        {
            d_textures[i].set(d_textures[i].get()->clone(ns));
            d_textures[i].get()->parentList.push_back(this);
        }
        if (d_textureTransforms[i].get())
        {
            d_textureTransforms[i].set(d_textureTransforms[i].get()->clone(ns));
            d_textureTransforms[i].get()->parentList.push_back(this);
        }
    }
}

inline bool isTextureModified(int numTextures, const std::array<VrmlSFNode, 10> &textures,
                              const std::array<VrmlSFNode, 10> &textureTransforms)
{
    for (int i = 0; i < numTextures; i++)
    {
        if (!textures[i].get())
            continue;
        if (textures[i].get()->isModified() || (textureTransforms[i].get() && textureTransforms[i].get()->isModified()))
        {
            return true;
        }
    }
    return false;
}

bool VrmlNodeAppearance::isModified() const
{
    return d_modified || (d_material.get() && d_material.get()->isModified() ||
                          isTextureModified(d_numTextures, d_textures, d_textureTransforms));
}

void VrmlNodeAppearance::clearFlags()
{
    VrmlNode::clearFlags();
    if (d_material.get())
        d_material.get()->clearFlags();

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (d_textures[i].get())
            d_textures[i].get()->clearFlags();
        if (d_textureTransforms[i].get())
            d_textureTransforms[i].get()->clearFlags();
    }
}

void VrmlNodeAppearance::addToScene(VrmlScene *s, const char *rel)
{
    nodeStack.push_front(this);
    if (d_relativeUrl.get() == NULL)
        d_relativeUrl.set(rel);
    d_scene = s;
    if (d_material.get())
        d_material.get()->addToScene(s, rel);

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (d_textures[i].get())
            d_textures[i].get()->addToScene(s, rel);
        if (d_textureTransforms[i].get())
            d_textureTransforms[i].get()->addToScene(s, rel);
    }
    nodeStack.pop_front();
}

// Copy the routes to nodes in the given namespace.

void VrmlNodeAppearance::copyRoutes(VrmlNamespace *ns)
{
    nodeStack.push_front(this);
    VrmlNode::copyRoutes(ns); // Copy my routes

    // Copy subnode routes
    if (d_material.get())
        d_material.get()->copyRoutes(ns);

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (d_textures[i].get())
            d_textures[i].get()->copyRoutes(ns);
        if (d_textureTransforms[i].get())
            d_textureTransforms[i].get()->copyRoutes(ns);
    }
    nodeStack.pop_front();
}

void VrmlNodeAppearance::render(Viewer *viewer)
{
    VrmlNodeMaterial *m = d_material.get() ? d_material.get()->as<VrmlNodeMaterial>() : 0;
    if (m)
    {
        float trans = m->transparency();
        float *diff = m->diffuseColor();
        float diffuse[3] = {diff[0], diff[1], diff[2]};
        // int nTexComponents = t ? t->nComponents() : 0;
        // if (nTexComponents == 2 || nTexComponents == 4)
        //    trans = 0.0;
        // if (nTexComponents >= 3)
        //	diffuse[0] = diffuse[1] = diffuse[2] = 1.0;

        auto relUrl = d_relativeUrl.get() ? d_relativeUrl.get() : d_scene->urlDoc()->url();
        viewer->setNameModes(m->VrmlNode::name(), relUrl.c_str());
        viewer->setMaterial(m->ambientIntensity(),
                            diffuse,
                            m->emissiveColor(),
                            m->shininess(),
                            m->specularColor(),
                            trans);

        m->clearModified();
    }
    else
    {
        viewer->setColor(1.0, 1.0, 1.0); // default color
        viewer->enableLighting(false);   // turn lighting off for this object
    }

    int numTextures = MAX_TEXTURES;
    for (; numTextures > 0; numTextures--)
    {
        if (d_textures[numTextures - 1].get())
            break;
    }

    viewer->setNumTextures(numTextures);
    for (int i = 0; i < numTextures; i++)
    {
        viewer->textureNumber = i;
        if (d_textures[i].get())
        {
            if (d_textureTransforms[i].get())
                d_textureTransforms[i].get()->render(viewer);
            else
                viewer->setTextureTransform(0, 0, 0, 0);
        }
        auto t = d_textures[i].get()->as<VrmlNodeTexture>();
        if (i == 0)
            t->setAppearance(this);
        // MAYBE something must be done here for multi-texturing to work correctly
        t->render(viewer);
    }
    clearModified();
}

bool VrmlNodeAppearance::isOnlyGeometry() const
{
    if (!VrmlNodeChild::isOnlyGeometry())
        return false;

    for (size_t i = 0; i < MAX_TEXTURES; i++)
    {
        if (d_textures[i].get() && !d_textures[i].get()->isOnlyGeometry())
            return false;
    }

    return true;
}
